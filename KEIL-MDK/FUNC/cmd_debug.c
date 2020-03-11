#include "cmd_debug.h"
#include "string_operate.h"
#include "lora_transmission.h"
#include "string.h"
#include "uart_svc.h"
#include "calendar.h"


typedef int (*data_parse_t)(uint8_t* data, uint8_t size);
typedef int (*attr_set_t)(uint8_t* data, uint8_t size);

const char cmd_type_end_mark = ':'; 
const char cmd_dev_addr_end_mark = ' ';
const char cmd_attr_end_mark = ' '; 
const char cmd_attr_value_end_mark = ' '; 
const char* cmd_type[] = {
	"w200",
	"w201",
};

#define W200_ATTR_NUMS		12

const char* cmd_w200_attr_tb[] = {
	"long_addr",
	"short_addr",
	"sample_time_ctr",
	"fixed_point_time",
	"time_stamp",
	"time_offset",
	"x_thres",
	"y_thres",
	"z_thres",
	"gateway_addr",
	"print_ctrl",
	"dev_ctrl",
};

#define W200_PRINT_CTRL_NUMS	6
const char* cmd_w200_print_ctrl_tb[] = {
	"raw_open",
	"raw_close",
	"parse_open",
	"parse_close",
	"reply_open",
	"reply_close",
};

#define W200_DEV_CTRL_NUMS	2
const char* cmd_w200_dev_ctrl_tb[] = {
	"reply_open",
	"reply_close",
};

const char* cmd_w200_reply_tb[] = {
	"w200:param setting success",
	"w200:error code 1,param long_addr setting error:",
	"w200:error code 2,param short_addr setting error:",
	"w200:error code 3,param mode setting error:",
	"w200:error code 4,param interval setting error:",
	"w200:error code 5,param time_stamp_addr setting error:",
	"w200:error code 6,param timee_offset setting error:",
	"w200:error code 7,param x_thres setting error:",
	"w200:error code 8,param y_thres setting error:",
	"w200:error code 9,param z_thres setting error:",
	"w200:error code 10,param dev_addr setting error:",
	"w200:error code 11,invalid command:",
	"w200:error code 12,invalid attribute:",
	"w200:error code 13,no instructions exist",
	"w200:error code 14,param gateway_addr setting error:",
	"w200:error code 15,device is not exist",
	"w200:error code 16,param print_ctrl setting error:",
	"w200:error code 17,param dev_ctrl setting error:",
};

const char* cmd_w201_attr_tb[] = {
	"long_addr",
	"short_addr",
	"mode",
	"interval",
	"time_stamp",
	"time_offset",
	"x_thres",
	"y_thres",
	"z_thres",
	"gateway_addr",
	"print_ctrl",
	"dev_ctrl",
};

static int cmd_w200_data_parse(uint8_t* data, uint8_t size);
static int cmd_w201_data_parse(uint8_t* data, uint8_t size);
data_parse_t data_parse[sizeof(cmd_type) / sizeof(cmd_type[0])] = {
	cmd_w200_data_parse,
	cmd_w201_data_parse,
};

static int cmd_long_addr_attr_set(uint8_t* data, uint8_t size);
static int cmd_short_addr_attr_set(uint8_t* data, uint8_t size);
static int cmd_mode_attr_set(uint8_t* data, uint8_t size);
static int cmd_interval_attr_set(uint8_t* data, uint8_t size);
static int cmd_time_stamp_attr_set(uint8_t* data, uint8_t size);
static int cmd_time_offset_attr_set(uint8_t* data, uint8_t size);
static int cmd_x_thres_attr_set(uint8_t* data, uint8_t size);
static int cmd_y_thres_attr_set(uint8_t* data, uint8_t size);
static int cmd_z_thres_attr_set(uint8_t* data, uint8_t size);
static int cmd_gateway_addr_attr_set(uint8_t* data, uint8_t size);
static int cmd_print_ctrl_attr_set(uint8_t* data, uint8_t size);
static int cmd_dev_ctrl_attr_set(uint8_t* data, uint8_t size);
attr_set_t w200_attr_set[W200_ATTR_NUMS] = {
	cmd_long_addr_attr_set,
	cmd_short_addr_attr_set,
	cmd_mode_attr_set,
	cmd_interval_attr_set,
	cmd_time_stamp_attr_set,
	cmd_time_offset_attr_set,
	cmd_x_thres_attr_set,
	cmd_y_thres_attr_set,
	cmd_z_thres_attr_set,
	cmd_gateway_addr_attr_set,
	cmd_print_ctrl_attr_set,
	cmd_dev_ctrl_attr_set,
};

typedef struct {
	uint8_t dev_short_addr[2];
	uint8_t long_addr[8];
	uint8_t gateway_addr[8];
	uint8_t short_addr[2];
	uint8_t sample_time_ctr;
	uint32_t fixed_point_time;
	uint32_t time_stamp;
	uint16_t time_offset;
}comm_attr_t;

typedef struct {
	comm_attr_t comm_attr;
	float x_thres;
	float y_thres;
	float z_thres;
}w200_attr_t;

static w200_attr_t w200_attr;
static w200_attr_t w200_attr_tmp;
static uint8_t cmd_data_rx_size = 0;
static uint8_t cmd_data_buf[255];
static uint8_t w200_reply_mark = 0;
static char w200_reply_msg[100];
static uint8_t w200_data_process_mark = 0;
static uint32_t w200_data_flag = 0;
	
static int cmd_long_addr_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	
	if(size != 16)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 2;
		return -1;
	}
	
	for(int i = 0; i < size; i++)
	{
		if(is_hex_or_digit(str[i]) == 0) //判断字符是否是16进制字符
		{
			strcpy(w200_reply_msg, str);
			w200_reply_mark = 2;
			return -1;
		}
	}
	
	hex_string_to_bytes((const char*)str, w200_attr_tmp.comm_attr.long_addr, strlen(str));
	return 0;
}

static int cmd_short_addr_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	
	if(size != 4)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 3;
		return -1;
	}
	
	for(int i = 0; i < size; i++)
	{
		if(is_hex_or_digit(str[i]) == 0) //判断字符是否是16进制字符
		{
			strcpy(w200_reply_msg, str);
			w200_reply_mark = 3;
			return -1;
		}
	}
	
	hex_string_to_bytes((const char*)str, w200_attr_tmp.comm_attr.short_addr, strlen(str));
	return 0;
}

static int cmd_mode_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	int value;
	if(string_to_integer((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 4;
		return -1;
	}
	w200_attr_tmp.comm_attr.sample_time_ctr = value & 0xFF;
	return 0;
}

static int cmd_interval_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	int value;
	if(string_to_integer((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 5;
		return -1;
	}
	w200_attr_tmp.comm_attr.fixed_point_time = value;
	return 0;
}

static int cmd_time_stamp_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	int value;
	if(string_to_integer((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 6;
		return -1;
	}
	w200_attr_tmp.comm_attr.time_stamp = value;
	return 0;
}

static int cmd_time_offset_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	int value;
	if(string_to_integer((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 7;
		return -1;
	}
	w200_attr_tmp.comm_attr.time_offset = value & 0xFFFF;
	return 0;
}

static int cmd_x_thres_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	float value;
	if(string_to_float((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 8;
		return -1;
	}
	w200_attr_tmp.x_thres = value;
	return 0;
}

static int cmd_y_thres_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	float value;
	if(string_to_float((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 9;
		return -1;
	}
	w200_attr_tmp.y_thres = value;
	return 0;
}

static int cmd_z_thres_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	float value;
	if(string_to_float((const char*)str, &value) < 0)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 10;
		return -1;
	}
	w200_attr_tmp.z_thres = value;
	return 0;
}

static int cmd_gateway_addr_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	
	if(size != 16)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 15;
		return -1;
	}
	
	for(int i = 0; i < size; i++)
	{
		if(is_hex_or_digit(str[i]) == 0) //判断字符是否是16进制字符
		{
			strcpy(w200_reply_msg, str);
			w200_reply_mark = 15;
			return -1;
		}
	}
	
	hex_string_to_bytes((const char*)str, w200_attr_tmp.comm_attr.gateway_addr, strlen(str));
	return 0;
}

static int cmd_print_ctrl_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';

	int i;
	for(i = 0; i < W200_PRINT_CTRL_NUMS; i++)
	{
		if(strcmp(str, cmd_w200_print_ctrl_tb[i]) == 0)
		{
			extern ctrl_class_t ctrl_class;
			if(i == 0)
			{
				ctrl_class.print_ctrl |= 0X01;
			}
			else if(i == 1)
			{
				ctrl_class.print_ctrl &= ~0X01;
			}
			else if(i == 2)
			{
				ctrl_class.print_ctrl |= 0X02;
			}
			else if(i == 3)
			{
				ctrl_class.print_ctrl &= ~0X02;
			}
			else if(i == 4)
			{
				ctrl_class.print_ctrl |= 0X04;
			}
			else if(i == 5)
			{
				ctrl_class.print_ctrl &= ~0X04;
			}
			break;
		}
	}
	
	if(i >= W200_PRINT_CTRL_NUMS)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 17;
		return -1;
	}
	
	return 0;
}

static int cmd_dev_ctrl_attr_set(uint8_t* data, uint8_t size)
{
	char str[size+1];
	bytes_to_char(data, str, size);
	str[size] = '\0';
	
	int i;
	for(i = 0; i < W200_DEV_CTRL_NUMS; i++)
	{
		if(strcmp(str, cmd_w200_dev_ctrl_tb[i]) == 0)
		{
			extern ctrl_class_t ctrl_class;
			if(i == 0)
			{
				ctrl_class.dev_ctrl |= 0X01;
			}
			else if(i == 1)
			{
				ctrl_class.dev_ctrl &= ~0X01;
			}
			break;
		}
	}
	
	if(i >= W200_DEV_CTRL_NUMS)
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 18;
		return -1;
	}
	
	return 0;
}

static int cmd_w200_attr_value_parse(uint8_t* data, uint8_t size, uint8_t index)
{
	uint8_t i;
	for(i = 0; i < size; i++)
	{
		if(*(char*)&data[i] == cmd_attr_value_end_mark) //判断属性值结束位置
		{
			break;
		}
	}
	
	if(i > size)
	{
		w200_reply_mark = index + 2;
		return -1;
	}

	w200_data_flag |= (1 << index);
	if(w200_attr_set[index](data, i) < 0) //保存属性值
	{
		return -1;
	}
	
	if(i == size) //数据解析完成
	{
		return 0;
	}
	
	return i;
}

static int cmd_w200_attr_parse(uint8_t* data, uint8_t size)
{
	uint8_t i;
	for(i = 0; i < size; i++)
	{
		if(*(char*)&data[i] == cmd_attr_end_mark) //判断属性结束位置
		{
			break;
		}
	}
	
	if(i >= size)
	{
		char str[size+1];
		bytes_to_char(data, str, size);
		str[i] = '\0';
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 13;
		return -1;
	}
	
	char str[i+1];
	bytes_to_char(data, str, i); //拷贝出属性类型
	str[i] = '\0';
	
	for(uint8_t j = 0; j < W200_ATTR_NUMS; j++)
	{
		if(strcmp(str, cmd_w200_attr_tb[j]) == 0) //判断属性类型是否有效
		{
			int len = cmd_w200_attr_value_parse(&data[i+1], size-i-1, j); //解析属性值
			if(len > -1)
			{
				if(len == 0)
				{
					return 0;
				}
				
				if(cmd_w200_attr_parse(&data[i+len+2], size-i-len-2) == 0)
				{
					return 0;
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return len;
			}
		}
	}
	
	strcpy(w200_reply_msg, str);
	w200_reply_mark = 13;
	return -1;
}

static int cmd_w200_data_parse(uint8_t* data, uint8_t size)
{
	uint8_t i;
	for(i = 0; i < size; i++)
	{
		if(*(char*)&data[i] == cmd_dev_addr_end_mark) //判断测点地址结束位置
		{
			break;
		}
	}
	
	if(i >= size || i != 4)
	{
		char str[i+1];
		bytes_to_char(data, str, i);
		str[i] = '\0';
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 11;
		return -1;
	}
	
	char (*p_str)[4] = (char (*)[4])data; //定义一个指向一个数组的指针，该数组有4个元素
	for(int j = 0; j < 4; j++)
	{
		if(is_hex_or_digit((*p_str)[j]) == 0) //判断字符是否是16进制字符
		{
			char str[i+1];
			bytes_to_char(data, str, i);
			str[i] = '\0';
			strcpy(w200_reply_msg, str);
			w200_reply_mark = 11;
			return -1;
		}
	}
	hex_string_to_bytes((const char*)p_str, w200_attr_tmp.comm_attr.dev_short_addr, i); //保存测点地址
	
	if(cmd_w200_attr_parse(&data[i+1], size-i-1) < 0) //解析属性
	{
		return -1;
	}
	
	return 0;
}

static int cmd_w201_data_parse(uint8_t* data, uint8_t size)
{
	return 0;
}

void cmd_data_rx(uint8_t* data, uint8_t size)
{
	if(cmd_data_rx_size != 0)
	{
		return;
	}
	
	memcpy(cmd_data_buf, data, size);
	cmd_data_rx_size = size;
}

void cmd_data_parse(void)
{
	w200_data_flag = 0;
	if(cmd_data_rx_size == 0)
	{
		return;
	}
	
	uint8_t i;
	for(i = 0; i < cmd_data_rx_size; i++)
	{
		if(*(char*)&cmd_data_buf[i] == cmd_type_end_mark) //判断命令类型结束位置
		{
			break;
		}
	}
	
	if(i >= cmd_data_rx_size)
	{
		w200_reply_mark = 14;
		cmd_data_rx_size = 0;
		return;
	}
	
	char str[i+1];
	bytes_to_char(cmd_data_buf, str, i); //拷贝出命令
	str[i] = '\0';
	
	uint8_t j;
	for(j = 0; j < sizeof(cmd_type) / sizeof(cmd_type[0]); j++)
	{
		if(strcmp(cmd_type[j], str) == 0) //检查命令是否有效
		{
			if(data_parse[j](&cmd_data_buf[i+1], cmd_data_rx_size-i-1) < 0) //数据解析
			{
				cmd_data_rx_size = 0;
				return;
			}
			break;
		}
	}
	
	if(j >= sizeof(cmd_type) / sizeof(cmd_type[0]))
	{
		strcpy(w200_reply_msg, str);
		w200_reply_mark = 12;
		cmd_data_rx_size = 0;
		return;
	}
	
	memcpy(&w200_attr, &w200_attr_tmp, sizeof(w200_attr_tmp));
	cmd_data_rx_size = 0;
	w200_reply_mark = 1;
	w200_data_process_mark = 1;
}

void cmd_data_process(void)
{
	if(w200_data_process_mark == 1)
	{
		w200_data_process_mark = 0;
		extern lora_reply_data_t lora_reply_data;
		lora_reply_data.status = w200_data_flag;
		
		extern peer_data_t peer_data;
		uint8_t is_exit_dev = 0;
		if(peer_data.current_conn_nums != 0 &&
		   (lora_reply_data.status & (~(GATEWAY_ADDR|PRINT_CTRL|DEV_CTRL))))
		{
			for(int i = 0; i < peer_data.current_conn_nums; i++)
			{
				if(*(uint16_t*)w200_attr.comm_attr.dev_short_addr == 0XFFFF || 
				   *(uint16_t*)peer_data.peer_attr[i].short_addr == *(uint16_t*)w200_attr.comm_attr.dev_short_addr)
				{
					is_exit_dev = 1;
					peer_data.peer_attr[i].set_flag = 1;
					
					if(*(uint16_t*)w200_attr.comm_attr.dev_short_addr != 0XFFFF)
					{
						break;
					}
				}
			}
		}
		else if(lora_reply_data.status & (GATEWAY_ADDR|PRINT_CTRL|DEV_CTRL))
		{
			is_exit_dev = 1;
		}
		
		if(is_exit_dev == 0)
		{
			w200_reply_mark = 16;
			return;
		}
		
		if(lora_reply_data.status & LONG_ADDR)
		{
			memcpy(lora_reply_data.long_addr, w200_attr.comm_attr.long_addr, sizeof(w200_attr.comm_attr.long_addr));
		}
		
		if(lora_reply_data.status & SHORT_ADDR)
		{
			memcpy(lora_reply_data.short_addr, w200_attr.comm_attr.short_addr, sizeof(w200_attr.comm_attr.short_addr));
		}
		
		if(lora_reply_data.status & SAMPLE_TIME_CTR)
		{
			memcpy(&lora_reply_data.sample_time_ctr, &w200_attr.comm_attr.sample_time_ctr, sizeof(w200_attr.comm_attr.sample_time_ctr));
		}
		
		if(lora_reply_data.status & FIXED_POINT_TIME)
		{
			memcpy(&lora_reply_data.fixed_point_time, &w200_attr.comm_attr.fixed_point_time, sizeof(w200_attr.comm_attr.fixed_point_time));
		}
		
		if(lora_reply_data.status & TIME_STAMP)
		{
			Calendar_t* calendar_mod = Calendar_GetHandle();
			calendar_mod->SetTimeStamp(w200_attr.comm_attr.time_stamp);
//			memcpy(&lora_reply_data.time_stamp, &w200_attr.comm_attr.time_stamp, sizeof(w200_attr.comm_attr.time_stamp));
		}
		
		if(lora_reply_data.status & TIME_OFFSET)
		{
			memcpy(&lora_reply_data.time_offset, &w200_attr.comm_attr.time_offset, sizeof(w200_attr.comm_attr.time_offset));
		}
		
		if(lora_reply_data.status & GATEWAY_ADDR)
		{
			memcpy(&lora_reply_data.gateway_addr, &w200_attr.comm_attr.gateway_addr, sizeof(w200_attr.comm_attr.gateway_addr));
		}
		
		if(lora_reply_data.status & PRINT_CTRL)
		{
			lora_reply_data.status &= ~PRINT_CTRL;
		}
		
		if(lora_reply_data.status & DEV_CTRL)
		{
			lora_reply_data.status &= ~DEV_CTRL;
		}
	}
}

void cmd_data_reply(void)
{
	switch(w200_reply_mark)
	{
		case 1:
			printf("%s\n", cmd_w200_reply_tb[0]);
			break;
		case 2:
			printf("%s", cmd_w200_reply_tb[1]);
			printf("%s\n", w200_reply_msg);
			break;
		case 3:
			printf("%s", cmd_w200_reply_tb[2]);
			printf("%s\n", w200_reply_msg);
			break;
		case 4:
			printf("%s", cmd_w200_reply_tb[3]);
			printf("%s\n", w200_reply_msg);
			break;
		case 5:
			printf("%s", cmd_w200_reply_tb[4]);
			printf("%s\n", w200_reply_msg);
			break;
		case 6:
			printf("%s", cmd_w200_reply_tb[5]);
			printf("%s\n", w200_reply_msg);
			break;
		case 7:
			printf("%s", cmd_w200_reply_tb[6]);
			printf("%s\n", w200_reply_msg);
			break;
		case 8:
			printf("%s", cmd_w200_reply_tb[7]);
			printf("%s\n", w200_reply_msg);
			break;
		case 9:
			printf("%s", cmd_w200_reply_tb[8]);
			printf("%s\n", w200_reply_msg);
			break;
		case 10:
			printf("%s", cmd_w200_reply_tb[9]);
			printf("%s\n", w200_reply_msg);
			break;
		case 11:
			printf("%s", cmd_w200_reply_tb[10]);
			printf("%s\n", w200_reply_msg);
			break;
		case 12:
			printf("%s", cmd_w200_reply_tb[11]);
			printf("%s\n", w200_reply_msg);
			break;
		case 13:
			printf("%s", cmd_w200_reply_tb[12]);
			printf("%s\n", w200_reply_msg);
			break;
		case 14:
			printf("%s\n", cmd_w200_reply_tb[13]);
			break;
		case 15:
			printf("%s", cmd_w200_reply_tb[14]);
			printf("%s\n", w200_reply_msg);
			break;
		case 16:
			printf("%s\n", cmd_w200_reply_tb[15]);
			break;
		case 17:
			printf("%s", cmd_w200_reply_tb[16]);
			printf("%s\n", w200_reply_msg);
			break;
		case 18:
			printf("%s", cmd_w200_reply_tb[17]);
			printf("%s\n", w200_reply_msg);
			break;
		case 0:
			break;
	}
	
	w200_reply_mark = 0;
}










