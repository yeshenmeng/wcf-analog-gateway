#include "lora_transmission.h"
#include "wireless_comm_services.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "sw_timer_rtc.h"
#include "rng_lpm.h"
#include "light.h"
#include "string.h"
#include "math.h"
#include "sx1262.h"
#include "sys_param.h"
#include "uart_svc.h"
#include "calendar.h"


typedef enum {
	LORA_OFFLINE,
	LORA_CONNECT,
}Lora_ConnStatus;

static uint8_t LoraMTU = 200;
static uint8_t LoraTxCount = 0;
static uint8_t LoraTxBuf[255] = {0};
static Lora_State LoraState;
static Lora_Info_t Lora_Info;
static Lora_ConnStatus LoraConnStatus = LORA_OFFLINE;
static Lora_ConnStatus LoraPreConnStatus = LORA_OFFLINE;

/* SPI驱动程序实例ID,ID和外设编号对应，0:SPI0  1:SPI1 2:SPI2 */
#define SPI_INSTANCE  0 
/* 定义名称为spi的SPI驱动程序实例 */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE); 
/* SPI传输完成标志 */
static volatile bool spi_xfer_done; 
static radio_drv_funcs_t wireless_drv;

__weak void LORA_TaskStopHandler(void* param);
void LORA_TxCompleteCallback(uint8_t* pData, uint16_t size);


#include "host_net_swap.h"
#define COMM_TRANSMISSION_MSB		1
#if COMM_TRANSMISSION_MSB == 1 && COMM_TRAMSMISSION_LSB == 1
#error "通信传输字节序只能定义一种"
#endif
#define COMM_TRAMSMISSION_LSB		0

static uint8_t LoraReplyBuf[255];
static uint8_t LoraReplySize;
lora_reply_data_t lora_reply_data = {
	.short_addr = {0X66,0X66},
	.long_addr = SYS_PARAM_DEV_LONG_ADDR,
	.gateway_addr = SYS_PARAM_DEV_GATEWAY_ADDR,
};

#define TRANSMIT_X_AXIS		0X01
#define TRANSMIT_Y_AXIS		0X02
#define TRANSMIT_Z_AXIS		0X04
lora_reply_data_t lora_reply_init_data = {
	.status = SAMPLE_TIME_CTR | TIME_STAMP | TIME_OFFSET | 
			  FIXED_POINT_TIME | SAMPLE_PERIOD | HEART_BEAT_PERIOD | 
			  SENS_FREQ | SUM_FRAGMENT | FRAGMENT_SIZE | TRANSMIT_FLAG,
	.sample_time_ctr = 0,
	.time_offset = 0,
//	.fixed_point_time = (1<<9)|(1<<11),
	.fixed_point_time = 0X00FFFFFF,
	.sample_period = 60 * 8,
	.heart_beat_period = 60 * 10,
	.sens_freq = SENS_FREQ_250HZ,
//	.sens_freq = SENS_FREQ_62HZ,
	.sum_fragment = 2,
	.fragment_size = 1024,
	.transmit_flag = TRANSMIT_X_AXIS | TRANSMIT_Y_AXIS | TRANSMIT_Z_AXIS,
};


static void gpiote_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin == LORA_IRQ_PIN)
	{
		if(action == NRF_GPIOTE_POLARITY_LOTOHI)
		{
			//接收LORA数据
			LORA_TxCompleteCallback(NULL, NULL);
		}
	}
}

static void LORA_ExtIntConfig(void)
{
	nrfx_gpiote_in_config_t nrfx_gpiote_in_config;
	
	if (!nrf_drv_gpiote_is_init())
	{
			uint32_t err_code;
			err_code = nrf_drv_gpiote_init();
			APP_ERROR_CHECK(err_code);
	}	
	
	nrfx_gpiote_in_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;
	nrfx_gpiote_in_config.is_watcher = false;
	nrfx_gpiote_in_config.pull = NRF_GPIO_PIN_NOPULL;
	nrfx_gpiote_in_config.hi_accuracy = true;
	nrf_drv_gpiote_in_init(LORA_IRQ_PIN,
												 &nrfx_gpiote_in_config,
												 &gpiote_in_pin_handler);
	
	nrf_drv_gpiote_in_event_enable(LORA_IRQ_PIN, true);
}

//SPI事件处理函数
static void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
  //设置SPI传输完成  
	spi_xfer_done = true;
}

static void LORA_SPI_Config(void)
{
	nrf_gpio_cfg_output(LORA_SPI_CS_PIN);
	LORA_SPI_CS_DISABLE();
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.miso_pin = LORA_SPI_MISO_PIN;
	spi_config.mosi_pin = LORA_SPI_MOSI_PIN;
	spi_config.sck_pin  = LORA_SPI_SCK_PIN;
	APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));
}

static void LORA_SPI_ConfigDefault(void)
{
	nrf_drv_spi_uninit(&spi);
	nrf_gpio_cfg_default(LORA_SPI_MISO_PIN);
	nrf_gpio_cfg_default(LORA_SPI_MOSI_PIN);
	nrf_gpio_cfg_default(LORA_SPI_SCK_PIN);
	nrf_gpio_cfg_default(LORA_SPI_CS_PIN);
}

void LORA_SPI_Transfer(uint8_t* tx_buffer, uint8_t tx_length, uint8_t* rx_buffer, uint8_t rx_length)
{
	spi_xfer_done = false;
	LORA_SPI_CS_ENABLE();
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, tx_buffer, tx_length, rx_buffer, rx_length));
	while(!spi_xfer_done);
}

static void LORA_RADIO_Init(void)
{
	wireless_drv = radio_sx1262_lora_init();
	wireless_drv.radio_reset();
	wireless_drv.radio_init();
}

void LORA_Config(void)
{
	nrf_gpio_cfg(LORA_POWER_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);

	nrf_gpio_cfg(LORA_TRANSMIT_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
	
	nrf_gpio_cfg(LORA_RECEIVE_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
	
	nrf_gpio_cfg(LORA_RESET_PIN,
							 NRF_GPIO_PIN_DIR_OUTPUT,
							 NRF_GPIO_PIN_INPUT_DISCONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);

	nrf_gpio_cfg(LORA_BUSY_PIN,
							 NRF_GPIO_PIN_DIR_INPUT,
							 NRF_GPIO_PIN_INPUT_CONNECT,
							 NRF_GPIO_PIN_NOPULL,
							 NRF_GPIO_PIN_S0S1,
							 NRF_GPIO_PIN_NOSENSE);
							
	LORA_PWOER_ENABLE();
	LORA_TRANSMIT_ENABLE();
	LORA_RECEIVE_DISABLE();
	LORA_RESET_DISABLE();
	LORA_ExtIntConfig();
	LORA_SPI_Config();
	LORA_RADIO_Init();
	nrf_delay_us(100);
}
	
void LORA_ConfigDefault(void)
{
	nrfx_gpiote_in_event_disable(LORA_IRQ_PIN);
	nrfx_gpiote_in_uninit(LORA_IRQ_PIN);
	LORA_SPI_ConfigDefault();
	nrf_gpio_cfg_default(LORA_POWER_PIN);
	nrf_gpio_cfg_default(LORA_TRANSMIT_PIN);
	nrf_gpio_cfg_default(LORA_RECEIVE_PIN);
	nrf_gpio_cfg_default(LORA_RESET_PIN);
	nrf_gpio_cfg_default(LORA_BUSY_PIN);
	nrf_gpio_cfg_default(LORA_IRQ_PIN);
}	

void LORA_TaskStart(void)
{
//	if(Lora_Info.State == LORA_IDLE)
	{
		Lora_Info.LPMHandle->TaskSetStatus(LORA_TASK_ID, LPM_TASK_STA_RUN);
		LoraState = LORA_ACTIVE;
		LoraPreConnStatus = LoraConnStatus;
	}
}

static void LORA_TaskStop(void)
{
	if(Lora_Info.State == LORA_ACTIVE)
	{
		Lora_Info.Param.TxFailTimes = 0;
		LoraState = LORA_STOP;
	}
}

static void LORA_SetTxData(uint8_t* pData, uint8_t size)
{
	for(int i=0; i<sizeof(LoraTxBuf); i++)
	{
		LoraTxBuf[i] = *(pData+i);
	}
}

static void LORA_IdleLowPowerManage(void)
{
	if(Lora_Info.Param.IsIdleEnterLp == 0)
	{
		return;
	}
	
	Lora_Info.LPMHandle->TaskSetStatus(LORA_TASK_ID, LPM_TASK_STA_LP);  /* 设置任务低功耗状态 */
}

void SWT_LoraTaskTimeSliceCallback(void)
{
	if(LoraState!=LORA_TIMEOUT && LORA_TIMEOUT!=LORA_STOP)
	{
		LoraState = LORA_TIMEOUT;
	}
}

void SWT_LoraTxTimeoutCallback(void)
{
	if(LoraState!=LORA_TIMEOUT && LORA_TIMEOUT!=LORA_STOP)
	{
		LoraState = LORA_TX_FAIL;
	}
}

void SWT_LoraIdleCallback(void)
{
	if(LoraState!=LORA_TIMEOUT && LORA_TIMEOUT!=LORA_STOP)
	{
		LoraState = LORA_ACTIVE;
	}
}

void LORA_TxCompleteCallback(uint8_t* pData, uint16_t size)
{
	if(Lora_Info.State == LORA_ACTIVE)
	{
		if(LoraState!=LORA_TIMEOUT && LORA_TIMEOUT!=LORA_STOP)
		{
			LoraState = LORA_TX_SUCCESS;
		}
	}
}

uint32_t delay_time = 0;
static uint32_t LORA_RandomDelay(void)
{
	static uint8_t max_flag = 0;
	static uint32_t delayTime = 0;
	RNG_LPM_t* rng_lpm = RNG_LPM_GetHandle();
	uint32_t randomNum;
	
	if(Lora_Info.Param.TxFailTimes == 1)
	{
		max_flag = 0;
	}
	
	rng_lpm->RandomUpper = Lora_Info.Param.RandomDelayUpper;
	rng_lpm->RandomLower = Lora_Info.Param.RandomDelayLower;
	rng_lpm->GenerateRandomNumber(&randomNum, 1);
	
	if(max_flag == 0)
	{
		delayTime = pow(2, Lora_Info.Param.TxFailTimes-1) * Lora_Info.Param.DelayBaseTime + randomNum;
	}
	
	if(max_flag == 0 && Lora_Info.Param.TxMaxDelayTime < delayTime)
	{
		max_flag = 1;
	}
	
	if(max_flag == 1)
	{
		delayTime = Lora_Info.Param.TxMaxDelayTime + randomNum;
	}
	
	delay_time = delayTime;
	return delayTime;
}

static void LORA_FillConnCmdCache(void)
{
	LoraTxCount = 0;
	
	/* 命令头 */
	uint32_t cmdHeader = CMD_CONNECT;
	memcpy(&LoraTxBuf[LoraTxCount], (uint8_t*)&cmdHeader, sizeof(cmdHeader));
	LoraTxCount += sizeof(cmdHeader);
	
	/* 数据段长度 */
	LoraTxBuf[LoraTxCount] = 8;
	LoraTxCount += 1;
	
	/* 数据段测点长地址 */
	wireless_comm_services_t* wirelessCommSvc = Wireless_CommSvcGetHandle();
	wirelessCommSvc->_sensor->readPropToBuf(1, &LoraTxBuf[LoraTxCount]);
	LoraTxCount += 8;
	
	/* CRC16 */
	uint16_t crc16 = wirelessCommSvc->modbusRtuCRC(LoraTxBuf, LoraTxCount);
	memcpy(&LoraTxBuf[LoraTxCount], (uint8_t*)&crc16, sizeof(crc16));
	LoraTxCount += sizeof(crc16);
}

static void LORA_FillPublishCmdCache(void)
{
	LoraTxCount = 0;
	
	/* 命令头 */
	uint32_t cmdHeader = CMD_PUBLISH;
	memcpy(&LoraTxBuf[LoraTxCount], (uint8_t*)&cmdHeader, sizeof(cmdHeader));
	LoraTxCount += sizeof(cmdHeader);
	
	/* 数据段长度 */
	LoraTxBuf[LoraTxCount] = 8;
	LoraTxCount += 1;
	
	/* 数据段测点短地址 */
	wireless_comm_services_t* wirelessCommSvc = Wireless_CommSvcGetHandle();
	wirelessCommSvc->_sensor->readPropToBuf(2, &LoraTxBuf[LoraTxCount]);
	LoraTxCount += 2;	
	
	/* 数据段属性数据 */
	wirelessCommSvc->_sensor->readPropToBuf(3, &LoraTxBuf[LoraTxCount]);
	LoraTxCount += 8;
	
	/* CRC16 */
	uint16_t crc16 = wirelessCommSvc->modbusRtuCRC(LoraTxBuf, LoraTxCount);
	memcpy(&LoraTxBuf[LoraTxCount], (uint8_t*)&crc16, sizeof(crc16));
	LoraTxCount += sizeof(crc16);
}

static FlagStatus Radio_CADDetect(void)
{
	return SET;
}

void Lora_ConnReply(void)
{
	LoraReplySize = 0;
	
	/* 命令头 */
	uint32_t cmdHeader = 0x02;
#if COMM_TRANSMISSION_MSB == 1
	cmdHeader = swap_htonl(cmdHeader);
#endif
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&cmdHeader, sizeof(cmdHeader));
	LoraReplySize += sizeof(cmdHeader);
	
	/* 数据段长度 */
	LoraReplyBuf[LoraReplySize] = 16;
	LoraReplySize += 1;
	
	/* 数据段测点长地址 */
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&lora_reply_data.long_addr, sizeof(lora_reply_data.long_addr));
	LoraReplySize += sizeof(lora_reply_data.long_addr);
	
	/* 数据段网关长地址 */
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&lora_reply_data.gateway_addr, sizeof(lora_reply_data.gateway_addr));
	LoraReplySize += sizeof(lora_reply_data.gateway_addr);
	
	if(lora_reply_data.status & GATEWAY_ADDR)
	{
		lora_reply_data.status &= ~GATEWAY_ADDR;
	}
	
//	/* 数据段测点短地址 */
//	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&lora_reply_data.short_addr, sizeof(lora_reply_data.short_addr));
//	LoraReplySize += sizeof(lora_reply_data.short_addr);	
	
	/* CRC16 */
	wireless_comm_services_t* wirelessCommSvc = Wireless_CommSvcGetHandle();
	uint16_t crc16 = wirelessCommSvc->modbusRtuCRC(LoraReplyBuf, LoraReplySize);
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&crc16, sizeof(crc16));
	LoraReplySize += sizeof(crc16);
	
	LORA_TRANSMIT_ENABLE();
	LORA_RECEIVE_DISABLE();
	
	LIGHT_2_ON();
	if(wireless_drv.radio_TXData(LoraReplyBuf, LoraReplySize))
	{
		extern ctrl_class_t ctrl_class;
		if(ctrl_class.print_ctrl & 0X04)
		{
			printf("回复失败!\n");
		}
	}
	LIGHT_2_OFF();
	
	LORA_TRANSMIT_DISABLE();
	LORA_RECEIVE_ENABLE();
	wireless_drv.radio_Rxmode();
	
	extern ctrl_class_t ctrl_class;
	if(ctrl_class.print_ctrl & 0X04)
	{
		printf("回复数据:");
		for(int i=0; i<LoraReplySize; i++)
		{
			printf("0x%02x ", LoraReplyBuf[i]);
		}
		printf("\n");
	}
}

void Lora_DataReply(void)
{
	LoraReplySize = 0;
	
	/* 命令头 */
	uint32_t cmdHeader = 0x04;
#if COMM_TRANSMISSION_MSB == 1
	cmdHeader = swap_htonl(cmdHeader);
#endif
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&cmdHeader, sizeof(cmdHeader));
	LoraReplySize += sizeof(cmdHeader);
	
	uint8_t replySize = 0;
	uint8_t lenIndex;
	LoraReplyBuf[LoraReplySize] = replySize;
	lenIndex = LoraReplySize;
	LoraReplySize += 1;	
	
	/* 数据段测点短地址 */
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&lora_reply_data.long_addr, sizeof(lora_reply_data.long_addr));
	LoraReplySize += sizeof(lora_reply_data.long_addr);	
	
	uint8_t attr_num = 0;
	uint8_t attr_index = 0;
	uint8_t attr_id[255];
	uint8_t attr_value_index = 0;
	uint8_t addr_value[255];
	
	lora_reply_data_t lora_reply = {0};
	extern peer_data_t peer_data;
	for(int i = 0; i < peer_data.current_conn_nums; i++)
	{
		if(*(uint64_t*)peer_data.peer_attr[i].long_addr == *(uint64_t*)lora_reply_data.long_addr)
		{
			if(peer_data.peer_attr[i].init_flag == 1)
			{
				peer_data.peer_attr[i].init_flag = 0;
				memcpy(&lora_reply, &lora_reply_init_data, sizeof(lora_reply_init_data));
				lora_reply.time_offset = lora_reply_init_data.time_offset * (i+1);
			}
			else if(peer_data.peer_attr[i].set_flag == 1)
			{
				peer_data.peer_attr[i].set_flag = 0;
				memcpy(&lora_reply, &lora_reply_data, sizeof(lora_reply_data));
			}
			break;
		}
	}
	
	if(lora_reply.status & LONG_ADDR)
	{
//		lora_reply_data.status &= ~LONG_ADDR;
		attr_num += 1;
		attr_id[attr_index++] = 1;
		memcpy(&addr_value[attr_value_index], lora_reply.long_addr, sizeof(lora_reply.long_addr));
		attr_value_index += sizeof(lora_reply.long_addr);
	}
	
	if(lora_reply.status & SHORT_ADDR)
	{
//		lora_reply_data.status &= ~SHORT_ADDR;
		attr_num += 1;
		attr_id[attr_index++] = 2;
		memcpy(&addr_value[attr_value_index], lora_reply.short_addr, sizeof(lora_reply.short_addr));
		attr_value_index += sizeof(lora_reply.short_addr);
	}
	
	if(lora_reply.status & SAMPLE_TIME_CTR)
	{
//		lora_reply_data.status &= ~SAMPLE_TIME_CTR;
		attr_num += 1;
		attr_id[attr_index++] = 3;
		memcpy(&addr_value[attr_value_index], (uint8_t*)&lora_reply.sample_time_ctr, sizeof(lora_reply.sample_time_ctr));
		attr_value_index += sizeof(lora_reply.sample_time_ctr);
	}
	
	if(lora_reply.status & FIXED_POINT_TIME)
	{
//		lora_reply_data.status &= ~FIXED_POINT_TIME;
		attr_num += 1;
		attr_id[attr_index++] = 4;
		uint32_t fixed_point_time = lora_reply.fixed_point_time;
#if COMM_TRANSMISSION_MSB == 1
		fixed_point_time = swap_htonl(fixed_point_time);
#endif
		memcpy(&addr_value[attr_value_index], (uint8_t*)&fixed_point_time, sizeof(fixed_point_time));
		attr_value_index += sizeof(fixed_point_time);
	}
	
	if(lora_reply.status & SAMPLE_PERIOD)
	{
//		lora_reply_data.status &= ~SAMPLE_PERIOD;
		attr_num += 1;
		attr_id[attr_index++] = 5;
		uint32_t sample_period = lora_reply.sample_period;
#if COMM_TRANSMISSION_MSB == 1
		sample_period = swap_htonl(sample_period);
#endif
		memcpy(&addr_value[attr_value_index], (uint8_t*)&sample_period, sizeof(sample_period));
		attr_value_index += sizeof(sample_period);
	}
	
	if(lora_reply.status & HEART_BEAT_PERIOD)
	{
//		lora_reply_data.status &= ~HEART_BEAT_PERIOD;
		attr_num += 1;
		attr_id[attr_index++] = 6;
		uint16_t heart_beat_period = lora_reply.heart_beat_period;
#if COMM_TRANSMISSION_MSB == 1
		heart_beat_period = swap_htons(heart_beat_period);
#endif
		memcpy(&addr_value[attr_value_index], (uint8_t*)&heart_beat_period, sizeof(heart_beat_period));
		attr_value_index += sizeof(heart_beat_period);
	}
	
	if(lora_reply.status & SENS_FREQ)
	{
//		lora_reply_data.status &= ~SENS_FREQ;
		attr_num += 1;
		attr_id[attr_index++] = 7;
		memcpy(&addr_value[attr_value_index], (uint8_t*)&lora_reply.sens_freq, sizeof(lora_reply.sens_freq));
		attr_value_index += sizeof(lora_reply.sens_freq);
	}
	
	if(lora_reply.status & TIME_STAMP)
	{
//		lora_reply_data.status &= ~TIME_STAMP;
		attr_num += 1;
		attr_id[attr_index++] = 8;
		Calendar_t* calendar_mod = Calendar_GetHandle();
		uint32_t time_stamp = (uint32_t)calendar_mod->GetTimeStamp();
#if COMM_TRANSMISSION_MSB == 1
		time_stamp = swap_htonl(time_stamp);
#endif
		memcpy(&addr_value[attr_value_index], (uint8_t*)&time_stamp, sizeof(time_stamp));
		attr_value_index += sizeof(time_stamp);
	}
	
	if(lora_reply.status & TIME_OFFSET)
	{
//		lora_reply_data.status &= ~TIME_OFFSET;
		attr_num += 1;
		attr_id[attr_index++] = 9;
		uint16_t time_offset = lora_reply.time_offset;
#if COMM_TRANSMISSION_MSB == 1
		time_offset = swap_htons(time_offset);
#endif
		memcpy(&addr_value[attr_value_index], (uint8_t*)&time_offset, sizeof(time_offset));
		attr_value_index += sizeof(time_offset);
	}
	
	if(lora_reply.status & SUM_FRAGMENT)
	{
//		lora_reply_data.status &= ~SUM_FRAGMENT;
		attr_num += 1;
		attr_id[attr_index++] = 10;
		memcpy(&addr_value[attr_value_index], (uint8_t*)&lora_reply.sum_fragment, sizeof(lora_reply.sum_fragment));
		attr_value_index += sizeof(lora_reply.sum_fragment);
	}
	
	if(lora_reply.status & FRAGMENT_SIZE)
	{
//		lora_reply_data.status &= ~FRAGMENT_SIZE;
		attr_num += 1;
		attr_id[attr_index++] = 11;
		uint16_t fragment_size = lora_reply.fragment_size;
#if COMM_TRANSMISSION_MSB == 1
		fragment_size = swap_htons(fragment_size);
#endif
		memcpy(&addr_value[attr_value_index], (uint8_t*)&fragment_size, sizeof(fragment_size));
		attr_value_index += sizeof(fragment_size);
	}	

	if(lora_reply.status & TRANSMIT_FLAG)
	{
//		lora_reply_data.status &= ~TRANSMIT_FLAG;
		attr_num += 1;
		attr_id[attr_index++] = 12;
		memcpy(&addr_value[attr_value_index], (uint8_t*)&lora_reply.transmit_flag, sizeof(lora_reply.transmit_flag));
		attr_value_index += sizeof(lora_reply.transmit_flag);
	}
	
	/* 数据段长度 */
	replySize = 8;
	if(attr_num != 0)
	{
		replySize += 1 + attr_index + attr_value_index;
		LoraReplyBuf[LoraReplySize] = attr_num; //属性个数
		LoraReplySize += 1;	
		
		memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&attr_id, attr_index); //属性ID
		LoraReplySize += attr_index;
		
		memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&addr_value, attr_value_index); //属性值
		LoraReplySize += attr_value_index;
	}
	LoraReplyBuf[lenIndex] = replySize;
	
	/* CRC16 */
	wireless_comm_services_t* wirelessCommSvc = Wireless_CommSvcGetHandle();
	uint16_t crc16 = wirelessCommSvc->modbusRtuCRC(LoraReplyBuf, LoraReplySize);
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&crc16, sizeof(crc16));
	LoraReplySize += sizeof(crc16);
	
	LORA_TRANSMIT_ENABLE();
	LORA_RECEIVE_DISABLE();
	
	LIGHT_2_ON();
	if(wireless_drv.radio_TXData(LoraReplyBuf, LoraReplySize))
	{
		extern ctrl_class_t ctrl_class;
		if(ctrl_class.print_ctrl & 0X04)
		{
			printf("回复失败!\n");
		}
	}
	LIGHT_2_OFF();
	
	LORA_TRANSMIT_DISABLE();
	LORA_RECEIVE_ENABLE();
	wireless_drv.radio_Rxmode();	
	
	extern ctrl_class_t ctrl_class;
	if(ctrl_class.print_ctrl & 0X04)
	{
		printf("回复数据:");
		for(int i=0; i<LoraReplySize; i++)
		{
			printf("0x%02x ", LoraReplyBuf[i]);
		}
		printf("\n");
	}
}

uint8_t payload_length = 0;
void Lora_TestReply(void)
{
	LoraReplySize = 0;
	
	/* 命令头 */
	uint32_t cmdHeader = 0x06;
#if COMM_TRANSMISSION_MSB == 1
	cmdHeader = swap_htonl(cmdHeader);
#endif
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&cmdHeader, sizeof(cmdHeader));
	LoraReplySize += sizeof(cmdHeader);
	
	/* 数据段长度 */
	LoraReplyBuf[LoraReplySize] = 16 + payload_length;
	LoraReplySize += 1;
	
	/* 数据段测点长地址 */
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&lora_reply_data.long_addr, sizeof(lora_reply_data.long_addr));
	LoraReplySize += sizeof(lora_reply_data.long_addr);
	
	/* 数据段网关长地址 */
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&lora_reply_data.gateway_addr, sizeof(lora_reply_data.gateway_addr));
	LoraReplySize += sizeof(lora_reply_data.gateway_addr);
	
	/* 数据段载荷 */
	memset(&LoraReplyBuf[LoraReplySize], 0XA5, payload_length);
	LoraReplySize += payload_length;	
	
	/* CRC16 */
	wireless_comm_services_t* wirelessCommSvc = Wireless_CommSvcGetHandle();
	uint16_t crc16 = wirelessCommSvc->modbusRtuCRC(LoraReplyBuf, LoraReplySize);
	memcpy(&LoraReplyBuf[LoraReplySize], (uint8_t*)&crc16, sizeof(crc16));
	LoraReplySize += sizeof(crc16);
	
	LORA_TRANSMIT_ENABLE();
	LORA_RECEIVE_DISABLE();

	LIGHT_2_ON();
	if(wireless_drv.radio_TXData(LoraReplyBuf, LoraReplySize))
	{
		extern ctrl_class_t ctrl_class;
		if(ctrl_class.print_ctrl & 0X04)
		{
			printf("回复失败!\n");
		}
	}
	LIGHT_2_OFF();
	
	nrf_delay_ms(50);
	
	LORA_TRANSMIT_DISABLE();
	LORA_RECEIVE_ENABLE();
	wireless_drv.radio_Rxmode();
	
	extern ctrl_class_t ctrl_class;
	if(ctrl_class.print_ctrl & 0X04)
	{
		printf("回复数据:");
		for(int i=0; i<LoraReplySize; i++)
		{
			printf("0x%02x ", LoraReplyBuf[i]);
		}
		printf("\n");
	}
}

__weak void Lora_RxHandler(uint8_t* p_data, uint8_t size){}
uint8_t rx_size;
uint8_t rx_buf[255];
static void LORA_StatusProc(void)
{
	uint8_t LoraOutState;
	SWT_t* timer = SWT_GetHandle();
	
	switch((uint8_t)LoraState)
	{
		case LORA_ACTIVE:
			LoraState = LORA_IDLE;
			Lora_Info.LPMHandle->TaskSetStatus(LORA_TASK_ID, LPM_TASK_STA_RUN);
			
			if(Lora_Info.State == LORA_IDLE)
			{
//				if(LoraConnStatus == LORA_OFFLINE)
//				{
//					LORA_FillConnCmdCache();
//				}
//				else if(LoraConnStatus == LORA_CONNECT)
//				{
//					LORA_FillPublishCmdCache();
//				}
				
				Lora_Info.State = LORA_ACTIVE;
//				timer->LoraTaskTimeSlice->Start(Lora_Info.Param.TaskTimeSlice); /* 启动LORA时间片定时器 */
			}
				
//			if(Radio_CADDetect() == SET)
//			{
//				LORA_Config();
//			#ifdef DEBUG_TEST
//				uint8_t dummy[13] = {0X00};
//				memset(dummy, 0X55, sizeof(dummy));
//			#endif
//				if(wireless_drv.radio_TXData(dummy, sizeof(dummy)) == LORA_RET_CODE_ERR)
//				{
//					LoraState = LORA_TX_FAIL;
//				}
//				else
//				{
//					timer->LoraTxTimeout->Start(Lora_Info.Param.TxTimeout); /* 启动LORA数据发送超时定时器 */
//				}
				LORA_TRANSMIT_DISABLE();
				LORA_RECEIVE_ENABLE();
				wireless_drv.radio_Rxmode();
				LIGHT_1_ON();
//		#ifdef DEBUG_TEST
//				if(Lora_Info.Param.TxFailTimes == 0)
//				{
//					LoraState = LORA_TX_SUCCESS;
//				}
//		#endif
//			}
//			else
//			{
//				LoraState = LORA_TX_FAIL;
//			}
			break;
		
		case LORA_TX_SUCCESS:
			LIGHT_1_OFF();
			LoraState = LORA_IDLE;
			Lora_Info.Param.TxFailTimes = 0;
			timer->LoraTxTimeout->Stop();
//			LORA_ConfigDefault();
			/* 解析接收到的响应数据 */
//			Lora_Info.CommHandle->parseMasterMsg();
			
			wireless_drv.radio_dio1_irq_func(rx_buf, &rx_size);
			Lora_RxHandler(rx_buf, rx_size);
			wireless_drv.radio_Rxmode();
		
			if(LoraConnStatus == LORA_OFFLINE)
			{
				timer->LoraTaskTimeSlice->Stop();
				
				/* 连接LORA网关成功 */
				LoraConnStatus = LORA_CONNECT;
				LoraOutState = LORA_OUT_STATE_LINK;
				Lora_Info.State = LORA_IDLE;
				LORA_TaskStopHandler((void *)&LoraOutState); /* LORA任务停止处理 */
			}
			else
			{
				LoraState = LORA_TIMEOUT; /* 发送成功模拟时间片定时器超时 */
			}
			break;
			
		case LORA_TX_FAIL:
			LIGHT_1_OFF();
			LoraState = LORA_IDLE;
			Lora_Info.Param.TxFailTimes++;
//			LORA_ConfigDefault();
		
			if(Lora_Info.Param.TxFailTimes >= Lora_Info.Param.TxMaxFailTimes)
			{
				LoraState = LORA_TIMEOUT; /* 超过发送失败次数 */
			}
			else
			{
				timer->LoraIdle->Start(LORA_RandomDelay()); /* 计算随机延时时间启动LORA空闲定时器 */
				LORA_IdleLowPowerManage(); /* 进入LORA低功耗,等待定时器唤醒 */
			}
			break;
		
		/* 时间片定时器超时事件 */
		case LORA_TIMEOUT:
			if(LoraPreConnStatus == LORA_OFFLINE)
			{
				LoraOutState = LORA_OUT_STATE_OFFLINE;
			}
			else if(LoraPreConnStatus==LORA_CONNECT && Lora_Info.Param.TxFailTimes!=0)
			{
				LoraConnStatus = LORA_OFFLINE;
				LoraOutState = LORA_OUT_STATE_DISCON;
			}
			else if(LoraPreConnStatus==LORA_CONNECT && Lora_Info.Param.TxFailTimes==0)
			{
				LoraOutState = LORA_OUT_STATE_CONNECT;
			}
		/* 主动停止LORA活动事件 */
		case LORA_STOP:
			LoraState = LORA_IDLE;
			Lora_Info.Param.TxFailTimes = 0;
			timer->LoraTaskTimeSlice->Stop(); /* 停止所有LORA定时器 */
			timer->LoraTxTimeout->Stop();
			timer->LoraIdle->Stop();
//			LORA_ConfigDefault();
			Lora_Info.State = LORA_IDLE;
			Lora_Info.LPMHandle->TaskSetStatus(LORA_TASK_ID, LPM_TASK_STA_STOP);
			LORA_TaskStopHandler((void *)&LoraOutState); /* LORA任务停止处理 */
			break;
	}
}

Lora_Info_t* LORA_TaskInit(LPM_t* LPMHandle)
{
	LORA_Config();
	LoraState = LORA_IDLE;
	Lora_Info.State = LORA_IDLE;
	
	Lora_Info.Param.DelayBaseTime = LORA_DELAY_BASE_TIME;
	Lora_Info.Param.RandomDelayUpper = LORA_RANDOME_DELAY_UPPER;
	Lora_Info.Param.RandomDelayLower = LORA_RANDOME_DELAY_LOWER;
	Lora_Info.Param.TxMaxDelayTime = LORA_TX_MAX_DELAY_TIME;
	Lora_Info.Param.TaskTimeSlice = SWT_LORA_TIME_SLICE_TIME;
	Lora_Info.Param.TxTimeout = SWT_LORA_TIMEOUT_TIME;
	Lora_Info.Param.TxFailTimes = 0;
	Lora_Info.Param.TxMaxFailTimes = LORA_TX_MAX_FIAL_TIMES;
	Lora_Info.Param.IsIdleEnterLp = ENABLE;
	
	Lora_Info.LPMHandle = LPMHandle;
	
	Lora_Info.TaskStart = LORA_TaskStart;
	Lora_Info.TaskStop = LORA_TaskStop;
	Lora_Info.SetTxData = LORA_SetTxData;
	Lora_Info.StatusProc = LORA_StatusProc;
	
	Lora_Info.LPMHandle->TaskRegister(LORA_TASK_ID);
	
	return &Lora_Info;
}

/* LORA任务停止处理 */
__weak void LORA_TaskStopHandler(void* param)
{
	return;
}












