#include "uart_svc.h"
#include "nrf_uart.h"
#include "app_uart.h"
#include "app_timer.h"
#include "cmd_debug.h"
#include "inclinometer.h"
#include "string_operate.h"
#include "lora_transmission.h"
#include "string.h"
#include "host_net_swap.h"
#include "sx1262.h"


#define UART_TX_BUF_SIZE 256       //���ڷ��ͻ����С���ֽ�����
#define UART_RX_BUF_SIZE 256       //���ڽ��ջ����С���ֽ�����

static uint16_t UartRxBufSize = 0;
static uint8_t UartRxBuf[255];
static uint8_t cmd_flag = 0;
static uint16_t LoraRxBufSize = 0;
static uint8_t LoraRxBuf[255];
static uint8_t LoraRxFlag = 0;

ctrl_class_t ctrl_class = {
	.print_ctrl = 0X02,
	.dev_ctrl = 0X01,
};
peer_data_t peer_data = {0};

void uart_send(uint8_t* data, uint16_t size)
{
	for(int i=0; i<size; i++)
	{
		while(app_uart_put(data[i]) != NRF_SUCCESS);
	}
}

void uart_receive(uint8_t* buf, uint16_t size)
{
	for(int i=0; i<size; i++)
	{
		while(app_uart_get(&buf[i]) != NRF_SUCCESS);
	}
}

APP_TIMER_DEF(uart_id);
void SWT_UartCallback(void* param)
{
	cmd_flag = 1;
	return;
}
void uart_timer_init(void)
{
	app_timer_create(&uart_id,
					APP_TIMER_MODE_SINGLE_SHOT,
					SWT_UartCallback);
}

void bytes_to_chars(uint8_t* des, char* src, uint16_t size)
{
	for(int i=0; i<size; i++)
	{
		src[i] = des[i];
	}
}

void uart_error_handle(app_uart_evt_t * p_event)
{
	//ͨѶ�����¼�
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
	{
		APP_ERROR_HANDLER(p_event->data.error_communication);
	}
	//FIFO�����¼�
	else if (p_event->evt_type == APP_UART_FIFO_ERROR)
	{
		APP_ERROR_HANDLER(p_event->data.error_code);
	}
	//���ڽ����¼�
	else if (p_event->evt_type == APP_UART_DATA_READY)
	{
		app_uart_get(&UartRxBuf[UartRxBufSize]);
		UartRxBufSize++;
		app_timer_start(uart_id, APP_TIMER_TICKS(20), NULL);
	}
	//���ڷ�������¼�
	else if (p_event->evt_type == APP_UART_TX_EMPTY)
	{
		;
	}
}

void Lora_RxHandler(uint8_t* p_data, uint8_t size)
{
	LoraRxFlag = 1;
	LoraRxBufSize = size;
	memcpy(LoraRxBuf, p_data, size);
}

void iot_param_cfg(void)
{
	if(cmd_flag == 1)
	{
		cmd_flag = 0;
		
		cmd_data_rx(UartRxBuf, UartRxBufSize);
		cmd_data_parse();
		cmd_data_process();
		cmd_data_reply();
		
		memset(UartRxBuf, 0X00, UartRxBufSize);
		UartRxBufSize = 0;
	}
}

void iot_conn_process(void)
{
	extern lora_reply_data_t lora_reply_data;
	memcpy(lora_reply_data.long_addr, &LoraRxBuf[5], 8);
	memcpy(lora_reply_data.short_addr, &LoraRxBuf[11], 2);

	if(ctrl_class.print_ctrl & 0X02)
	{
		char str[17];
		bytes_to_hex_string(lora_reply_data.long_addr, str, 8, 0);
		char div_1 = ':';
		char div_2 = ' ';
		char div_3 = ' ';
//		if(lora_reply_data.long_addr[0] == 0XC8)
//			printf("��ǲ������%c",div_1);
//		else if(lora_reply_data.long_addr[0] == 0XC9)
//			printf("�����Ʋ������%c",div_1);
		printf("�������%c",div_1);
		printf("����ַ%c%s%c",div_2,str,div_3);
	}

	if(ctrl_class.print_ctrl & 0X02)
	{
		extern sx1262_drive_t* lora_obj_get(void);
		char div_1 = ':';
		printf("�����ź�ǿ��%c%d",div_1,lora_obj_get()->radio_state.rssi);
		printf("\n");
	}
		
	if(ctrl_class.dev_ctrl & 0X01)
	{
		extern void Lora_ConnReply(void);
		Lora_ConnReply();
		
		int i = 0;
		for(i = 0; i < peer_data.current_conn_nums; i++)
		{
			if(*(uint64_t*)lora_reply_data.long_addr == *(uint64_t*)peer_data.peer_attr[i].long_addr)
			{
				if(lora_reply_data.long_addr[0] == 0XCA)
				{
					peer_data.peer_attr[i].init_flag = 1;
				}
				return;
			}
		}
		
		if(i >=  peer_data.current_conn_nums)
		{
			memcpy(peer_data.peer_attr[peer_data.current_conn_nums].short_addr, lora_reply_data.short_addr, 2);
			memcpy(peer_data.peer_attr[peer_data.current_conn_nums].long_addr, lora_reply_data.long_addr, 8);
			if(lora_reply_data.long_addr[0] == 0XCA)
			{
				peer_data.peer_attr[peer_data.current_conn_nums].init_flag = 1;
			}
			peer_data.current_conn_nums++;
		}
	}
}

typedef __packed struct {
	uint8_t sum_fragment;
	uint8_t sub_fragment;
	uint8_t sum_frame;
	uint8_t sub_frame;
}data_head_t;

typedef __packed struct {
	data_head_t head;
	uint8_t data[64];
}data_ptk_t;

typedef struct {
	/*! Accel X data */
	int16_t x;
	/*! Accel Y data */
	int16_t y;
	/*! Accel Z data */
	int16_t z;
}sens_accel_t;

data_ptk_t *data_ptk;
sens_accel_t sens_accel[32];

uint8_t axis_flag = 0;
uint8_t fragment_cnt = 0;
uint8_t frame_cnt = 0;

void iot_data_push_process(void)
{
	if(ctrl_class.print_ctrl & 0X02)
	{
		uint8_t num = 5;
		printf("��㳤��ַ:%02X%02X%02X%02X%02X%02X%02X%02X\n",LoraRxBuf[5],LoraRxBuf[6],LoraRxBuf[7],LoraRxBuf[8],LoraRxBuf[9],LoraRxBuf[10],LoraRxBuf[11],LoraRxBuf[12]);
		num += 8; //���Ը���
		
		uint8_t heart_beat_flag = 0;
		float value = 0;
		if(LoraRxBuf[4] == 78)
		{
			num++; //���ٶ�ֵID
			num++; //����ֵ
		}
		else if(LoraRxBuf[4] == 85)
		{
			num++; //��ص���ID
			num++; //�¶�����ID
			num++; //���ٶ�ֵID
			axis_flag = 1;
			
			num++; //����ֵ
			printf("����:%d\n", LoraRxBuf[num]);
			
			num++;
			swap_reverse(&LoraRxBuf[num], 4);
			memcpy(&value, &LoraRxBuf[num], 4);
			printf("�¶�:%.2f\n", value);
			num += 4;
			
			fragment_cnt = 0;
			frame_cnt = 0;
		}
		else if(LoraRxBuf[4] == 14)
		{
			num++; //�¶�����ID
			num++; //����ֵ
			swap_reverse(&LoraRxBuf[num], 4);
			memcpy(&value, &LoraRxBuf[num], 4);
			printf("�¶�:%.2f\n", value);
			heart_beat_flag = 1;
		}
		
		if(heart_beat_flag == 0)
		{
			data_ptk = (data_ptk_t*)&LoraRxBuf[num];
			printf("�ܶ�:%d,�Ӷ�%d,��֡%d,��֡%d\n", data_ptk->head.sum_fragment,data_ptk->head.sub_fragment,data_ptk->head.sum_frame,data_ptk->head.sub_frame);
			num += 4;
			
			if(axis_flag == 1)
			{
				for(int i = 0; i < 32; i++)
				{
					sens_accel[i].x = ((uint16_t)LoraRxBuf[num] << 8) + LoraRxBuf[num+1];
					num += 2;
				}
				
				printf("X������:\n");
				for(int i = 0; i < 32; i++)
				{
					printf("%d,", sens_accel[i].x);
				}
				printf("\n");
				
				frame_cnt++;
				if(frame_cnt >= 32)
				{
					frame_cnt = 0;
					fragment_cnt++;
					
					if(fragment_cnt >= data_ptk->head.sum_fragment)
					{
						fragment_cnt = 0;
						axis_flag = 2;
					}
				}
			}
			else if(axis_flag == 2)
			{
				for(int i = 0; i < 32; i++)
				{
					sens_accel[i].y = ((uint16_t)LoraRxBuf[num] << 8) + LoraRxBuf[num+1];
					num += 2;
				}
				
				printf("Y������:\n");
				for(int i = 0; i < 32; i++)
				{
					printf("%d,", sens_accel[i].y);
				}
				printf("\n");
				
				frame_cnt++;
				if(frame_cnt >= 32)
				{
					frame_cnt = 0;
					fragment_cnt++;
					
					if(fragment_cnt >= data_ptk->head.sum_fragment)
					{
						fragment_cnt = 0;
						axis_flag = 3;
					}
				}
			}
			else if(axis_flag == 3)
			{
				for(int i = 0; i < 32; i++)
				{
					sens_accel[i].z = ((uint16_t)LoraRxBuf[num] << 8) + LoraRxBuf[num+1];
					num += 2;
				}
				
				printf("Z������:\n");
				for(int i = 0; i < 32; i++)
				{
					printf("%d,", sens_accel[i].z);
				}
				printf("\n");
				
				frame_cnt++;
				if(frame_cnt >= 32)
				{
					frame_cnt = 0;
					fragment_cnt++;
					
					if(fragment_cnt >= data_ptk->head.sum_fragment)
					{
						fragment_cnt = 0;
						axis_flag = 0;
					}
				}
			}
		}
	}
	
	if(ctrl_class.print_ctrl & 0X02)
	{
		extern sx1262_drive_t* lora_obj_get(void);
		char div_1 = ':';
		printf("�����ź�ǿ��%c%d",div_1,lora_obj_get()->radio_state.rssi);
		printf("\n");
	}
	
	if(ctrl_class.dev_ctrl & 0X01)
	{
		int i;
		uint64_t long_addr = *(uint64_t*)&LoraRxBuf[5];
		for(i = 0; i < peer_data.current_conn_nums; i++)
		{
			if(*(uint64_t*)peer_data.peer_attr[i].long_addr == long_addr)
			{
				break;
			}
		}
		
		if(i >= peer_data.current_conn_nums)
		{
			return;
		}
		
		uint8_t reply_flag = 0;
		extern lora_reply_data_t lora_reply_data;
		if(lora_reply_data.long_addr[0] == 0XCA)
		{
			reply_flag = 1;
		}
		
		if(!reply_flag)
			return;
		
		extern void Lora_DataReply(void);
		Lora_DataReply();
	}
}

void iot_data_lost_rate_process(void)
{
	if(ctrl_class.print_ctrl & 0X02)
	{
		extern lora_reply_data_t lora_reply_data;
		memcpy(lora_reply_data.long_addr, &LoraRxBuf[5], 8);
		extern uint8_t payload_length;
		payload_length = LoraRxBuf[13];
		
		char div_1 = ':';
		printf("LORA�����ʲ���%c",div_1);
		extern sx1262_drive_t* lora_obj_get(void);
		printf("�����ź�ǿ��%c%d",div_1,lora_obj_get()->radio_state.rssi);
		printf("\n");
	}
	
	if(ctrl_class.dev_ctrl & 0X01)
	{
		extern void Lora_TestReply(void);
		Lora_TestReply();
	}
}

void uart_test(void)
{
	if(LoraRxFlag == 1)
	{
		LoraRxFlag = 0;

		if(*(uint32_t*)LoraRxBuf == 0X01000000)
		{
			iot_conn_process();
		}
		else if(*(uint32_t*)LoraRxBuf == 0X03000000)
		{
			iot_data_push_process();
		}
		else if(*(uint32_t*)LoraRxBuf == 0X05000000)
		{
			iot_data_lost_rate_process();
		}
		else
		{
			extern sx1262_drive_t* lora_obj_get(void);
			char div_1 = ':';
			printf("�����ź�ǿ��%c%d",div_1,lora_obj_get()->radio_state.rssi);
			printf("\n");
			printf("\n");
		}
		
		return;
	}
	
	iot_param_cfg();
}

void uart_run(void)
{
	uart_test();
}

//��������
void uart_init(void)
{
	uint32_t err_code;

	//���崮��ͨѶ�������ýṹ�岢��ʼ��
	const app_uart_comm_params_t comm_params =
	{
		UART_RX_PIN,//����uart��������
		UART_TX_PIN,//����uart��������
		UART_RTS_PIN,//����uart RTS���ţ����عرպ���Ȼ������RTS��CTS���ţ����������������ԣ������������������ţ����������Կ���ΪIOʹ��
		UART_CTS_PIN,//����uart CTS����
		APP_UART_FLOW_CONTROL_DISABLED,//�ر�uartӲ������
		false,//��ֹ��ż����
		NRF_UART_BAUDRATE_115200//uart����������Ϊ115200bps
	};

	//��ʼ�����ڣ�ע�ᴮ���¼��ص�����
	//����������һ�ν��գ�����������Ų��ӻᱨ��
	APP_UART_FIFO_INIT(&comm_params,
					 UART_RX_BUF_SIZE,
					 UART_TX_BUF_SIZE,
					 uart_error_handle,
					 APP_IRQ_PRIORITY_LOWEST,
					 err_code);

	APP_ERROR_CHECK(err_code);
	uart_timer_init();
}

#if 0
char* header_str = "header";
char* length_str = "length";
char* short_addr_str = "short_addr";
char* attr_nums_str = "attr_nums";
char* gas_id_str = "gas_id";
char* temp_id_str = "temp_id";
char* x_accel_id_str = "x_accel_id";
char* y_accel_id_str = "y_accel_id";
char* z_accel_id_str = "z_accel_id";
char* x_angle_id_str = "x_angle_id";
char* y_angle_id_str = "y_angle_id";
char* z_angle_id_str = "z_angle_id";
char* gas_value_str = "gas_value";
char* temp_value_str = "temp_value";
char* x_accel_value_str = "x_accel_value";
char* y_accel_value_str = "y_accel_value";
char* z_accel_value_str = "z_accel_value";
char* x_angle_value_str = "x_angle_value";
char* y_angle_value_str = "y_angle_value";
char* z_angle_value_str = "z_angle_value";

if(*(uint32_t*)LoraRxBuf == 0X03000000)
{
	printf("%s,",header_str);
	printf("%s,",length_str);
	printf("%s,",short_addr_str);
	printf("%s,",attr_nums_str);
	printf("%s,",gas_id_str);
	printf("%s,",temp_id_str);
	printf("%s,",x_accel_id_str);
	printf("%s,",y_accel_id_str);
	printf("%s,",z_accel_id_str);
	nrf_delay_ms(10);
	if(LoraRxBuf[4] == 40)
	{
		printf("%s,",x_angle_id_str);
		printf("%s,",y_angle_id_str);
		printf("%s,",z_angle_id_str);
	}
	printf("%s,",gas_value_str);
	printf("%s,",temp_value_str);
	printf("%s,",x_accel_value_str);
	printf("%s,",y_accel_value_str);
	printf("%s,",z_accel_value_str);
	if(LoraRxBuf[4] == 40)
	{
		printf("%s,",x_angle_value_str);
		printf("%s,",y_angle_value_str);
		printf("%s,",z_angle_value_str);
	}
	nrf_delay_ms(10);
	printf("\n");
	swap_reverse(&LoraRxBuf[0], 4);
	printf("%-*d,",strlen(header_str),*(uint32_t*)&LoraRxBuf[0]); //ͷ
	printf("%-*d,",strlen(length_str),LoraRxBuf[4]); //����
	printf("0x%02x,0x%02x ,", LoraRxBuf[5],LoraRxBuf[6]); //�̵�ַ
	printf("%-*d,",strlen(attr_nums_str),LoraRxBuf[7]); //���Ը���
	printf("%-*d,",strlen(gas_id_str),LoraRxBuf[8]); //�������Ժ�
	printf("%-*d,",strlen(temp_id_str),LoraRxBuf[9]); //�¶����Ժ�
	printf("%-*d,",strlen(x_accel_id_str),LoraRxBuf[10]); //x���ٶ����Ժ�
	printf("%-*d,",strlen(y_accel_id_str),LoraRxBuf[11]); //y���ٶ����Ժ�
	printf("%-*d,",strlen(z_accel_id_str),LoraRxBuf[12]); //z���ٶ����Ժ�
	nrf_delay_ms(10);
	uint8_t index = 0;
	if(LoraRxBuf[4] == 40)
	{
		printf("%-*d,",strlen(x_angle_id_str),LoraRxBuf[13]); //x�Ƕ����Ժ�
		printf("%-*d,",strlen(y_angle_id_str),LoraRxBuf[14]); //y�Ƕ����Ժ�
		printf("%-*d,",strlen(z_angle_id_str),LoraRxBuf[15]); //z�Ƕ����Ժ�
		index += 3;
	}
	printf("%-*d,",strlen(gas_value_str),LoraRxBuf[13+index]); //����ֵ
	swap_reverse(&LoraRxBuf[14+index], 4);
	float value = 0;
	memcpy(&value, &LoraRxBuf[14+index], 4);
	printf("%-*.3f,",strlen(temp_value_str),value); //�¶�ֵ
	swap_reverse(&LoraRxBuf[18+index], 4);
	memcpy(&value, &LoraRxBuf[18+index], 4);
	printf("%-*.3f,",strlen(x_accel_value_str),value); //x���ٶ�ֵ
	swap_reverse(&LoraRxBuf[22+index], 4);
	memcpy(&value, &LoraRxBuf[22+index], 4);
	printf("%-*.3f,",strlen(y_accel_value_str),value); //y���ٶ�ֵ
	swap_reverse(&LoraRxBuf[26+index], 4);
	memcpy(&value, &LoraRxBuf[26+index], 4);
	printf("%-*.3f,",strlen(z_accel_value_str),value); //z���ٶ�ֵ
	nrf_delay_ms(10);
	if(LoraRxBuf[4] == 40)
	{
		swap_reverse(&LoraRxBuf[30+index], 4);
		memcpy(&value, &LoraRxBuf[30+index], 4);
		printf("%-*.3f,",strlen(x_angle_value_str),value); //x�Ƕ�ֵ
		swap_reverse(&LoraRxBuf[34+index], 4);
		memcpy(&value, &LoraRxBuf[34+index], 4);
		printf("%-*.3f,",strlen(y_angle_value_str),value); //y�Ƕ�ֵ
		swap_reverse(&LoraRxBuf[38+index], 4);
		memcpy(&value, &LoraRxBuf[38+index], 4);
		printf("%-*.3f,",strlen(z_angle_value_str),value); //z�Ƕ�ֵ
	}
	printf("\n");
}
#endif











