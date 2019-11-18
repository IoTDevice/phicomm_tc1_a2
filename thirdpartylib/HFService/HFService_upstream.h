
#ifndef _HFSERVICE_UPSTREAM_H_
#define _HFSERVICE_UPSTREAM_H_

/********************************************************************************
*	Description	:  数据包处理
*	Name		: uart_proto_legal_check
*	Input		: data:数据包起始地址
				  proto_len:数据包长度
*	Returns		: none
 ********************************************************************************/
void uart_data_process(unsigned char *data,int proto_len);

/********************************************************************************
*	Description	: UART 数据接收，Buf 管理，接收状态控制
*	Name		: hfservice_uart_data_recv
*	Input		: data:数据包起始地址
				  len:数据包长度
*	Returns		: none
 ********************************************************************************/
void hfservice_uart_data_recv(unsigned char *data, int len);

/********************************************************************************
*	Description	: 参数初始化，localProcess启动
*	Name		: HFService_init
*	Input		: none
*	Returns		: HFSERV_STATE_FAILE or HFSERV_STATE_OK
 ********************************************************************************/
int HFService_init(void);

//void wifi_uart_send(unsigned char* data,int len);


#endif/*_HFSERVICE_UPSTREAM_H_*/

