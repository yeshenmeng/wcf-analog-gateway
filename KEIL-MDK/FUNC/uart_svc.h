#ifndef __UART_SVC_H__
#define __UART_SVC_H__
#include "main.h"

#if (SYS_HW_VERSION == SYS_HW_VERSION_V0_3_0)
#define UART_RX_PIN 				20
#define UART_TX_PIN  				19
#define UART_CTS_PIN				5
#define UART_RTS_PIN				6
#elif (SYS_HW_VERSION == SYS_HW_VERSION_V0_1_1)
#define UART_RX_PIN 				16
#define UART_TX_PIN  				17
#define UART_CTS_PIN				5
#define UART_RTS_PIN				6
#endif

#define PRINT_LROA_RX_SRC_MSG		0
#define PRINT_LROA_PARSE_MSG		1
#define LROA_REPLY_ENBALE			1

#define GATEWAY_CAP_SIZE			200
typedef enum {
	disconn,
	conn,
}conn_status_t;

typedef struct {
	conn_status_t conn_status;
	uint8_t long_addr[8];
	uint8_t short_addr[2];
	uint8_t set_flag;
	uint8_t init_flag;
}peer_attr_t;

typedef struct {
	uint32_t print_ctrl;
	uint32_t dev_ctrl;
}ctrl_class_t;

typedef struct {
	uint8_t current_conn_nums;
	peer_attr_t peer_attr[GATEWAY_CAP_SIZE];
}peer_data_t;

void uart_init(void);
void uart_send(uint8_t* data, uint16_t size);
void uart_receive(uint8_t* buf, uint16_t size);
void uart_run(void);


#endif









