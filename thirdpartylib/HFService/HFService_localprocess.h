#ifndef _HFLOCALPROCESS_H_
#define _HFLOCALPROCESS_H_

#include "hsf.h"

extern hfthread_sem_t device_init_ok_sem;
extern hfthread_sem_t uart_ack_sem_start;
extern hfthread_sem_t uart_ack_sem_stop;
extern hfthread_sem_t local_mcu_sem;

/********************************************************************************
*	Description	: 设备状态初始化Process
*	Name		: process_device_init
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void USER_FUNC process_device_init();

/********************************************************************************
*	Description	: UART 数据重发Process
*	Name		: process_uart_ack
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void USER_FUNC process_uart_ack();

/********************************************************************************
*	Description	: 本地状态处理Process
*	Name		: process_local_mcu
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void USER_FUNC process_local_mcu();

void down_data_action();

#endif/*_HFLOCALPROCESS_H_*/

