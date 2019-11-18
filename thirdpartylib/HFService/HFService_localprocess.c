
#include "HFService_config.h"
#include "HFService_localprocess.h"
#include "HFService_downstream.h"
#include "HFService_helper.h"
#include "hsf.h"

hfthread_sem_t device_init_ok_sem;
hfthread_sem_t uart_ack_sem_start;
hfthread_sem_t uart_ack_sem_stop;
hfthread_sem_t local_mcu_sem;
static hftimer_handle_t quary_state_timer = NULL;
hfthread_sem_t device_state_query_sem;
unsigned char quary_devices_state[4] = {0xAA,0xAA,0xA1,0x4F};

#define PROC_ACK_WAITING		0
#define PROC_ACK_SENDED		    1
extern void acquire_network_lq(unsigned char *rbuf);
/********************************************************************************
*	Description	: UART 发送命令查询设备的初始状态
*	Name		: hfservice_device_init_cmd
*	Input		: none
*	Returns		: none
 ********************************************************************************/
static void hfservice_device_init_cmd(void)
{
	if(hfsys_get_run_mode() == HFSYS_STATE_RUN_THROUGH)
	{
		// TODO: UART下发状态查询命令
		//extern int wificonnect_flag;
		//extern int cloudconnect_flag;
		unsigned char init_cmd[8]={0xAA, 0xAA, 0x05, 0x03,  0x00, 0x00, 0x00, 0x5C};
		down_data_action(init_cmd, sizeof(init_cmd));
	}
}

static void USER_FUNC timer_callback_quary_state( hftimer_handle_t htimer )
{
	hfthread_sem_signal(device_state_query_sem);
}

/********************************************************************************
*	Description	: 设备状态查询Process
*	Name		: process_device_state
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void  process_device_state()
{
	int ret;
	int init_timeout=500;
	
	hfthread_sem_new (&device_state_query_sem, 0);
	if((quary_state_timer = hftimer_create("SMARTLINK-TIMER", 5*1000, true,
        4, timer_callback_quary_state, 0)) == NULL)
	{
		u_printf("create SMARTLINK-TIMER fail\n");
	}
	hftimer_start(quary_state_timer);
	
	while (1)
	{
		ret=hfthread_sem_wait(device_state_query_sem,init_timeout);
		if (ret>0)
		{
			fireUartSend(quary_devices_state, 4);
		}
	}
}


/********************************************************************************
*	Description	: 设备状态初始化Process
*	Name		: process_device_init
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void USER_FUNC process_device_init()
{
	int ret;
	int init_timeout=1000;
	while (1)
	{
		ret=hfthread_sem_wait(device_init_ok_sem,init_timeout);
		if (ret>0)
		{
		    u_printf("start connect cloud\r\n");
			hfservice_cloud_start();
			hfthread_sem_free(device_init_ok_sem);
			device_init_ok_sem=NULL;
			hfthread_destroy(NULL);
			break;
		}
		else
		{
			hfservice_device_init_cmd();
		}
	}
}

#if UART_DOWN_NEED_ACK
/********************************************************************************
*	Description	: UART 数据重发Process
*	Name		: process_uart_ack
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void USER_FUNC process_uart_ack()
{
	int state=PROC_ACK_WAITING;
	int  wait_timer;
	int uart_cmd_retry = 0;
	int ret;

	while (1)
	{
		if (state==PROC_ACK_WAITING)
		{
			ret= hfthread_sem_wait(uart_ack_sem_start,0);
			if (ret>0)
			{
				state=PROC_ACK_SENDED;
				uart_cmd_retry = 0;
			}
		}
		else if (state==PROC_ACK_SENDED)
		{
			ret=hfthread_sem_wait(uart_ack_sem_stop,UART_DOWN_ACK_TIMEOUT);
			if (ret>0)
			{
				state = PROC_ACK_WAITING;
			}
			else
			{
				if(uart_cmd_retry < UART_DOWN_ACK_RETRY_NUM)
				{
					down_data_action(NULL, 0);
					uart_cmd_retry++;
				}
				else
				{
					uart_cmd_retry = 0;
					state = PROC_ACK_WAITING;
				}
			}
		}
	}
}

#endif

#if DEVICE_DONT_NEED_MCU
/********************************************************************************
*	Description	: 本地状态处理Process
*	Name		: process_local_mcu
*	Input		: none
*	Returns		: none
 ********************************************************************************/
void USER_FUNC process_local_mcu()
{
	int ret;
	while (1)
	{
		ret= hfthread_sem_wait(local_mcu_sem,0);
		if (ret>0)
		{
			// TODO: 本地状态回复
			uart_data_process(data, len);
		}
	}
}

#endif


