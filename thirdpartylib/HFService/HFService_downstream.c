
#include "HFService_config.h"
#include "HFService_downstream.h"
#include "HFService_upstream.h"
#include "HFService_localprocess.h"
#include "HFService_helper.h"
#include <string.h>


// 标识下行命令来自Local 或Remote
int flag_local_waiting_for_rpl=0;
int flag_remote_waiting_for_rpl=0;

//保存串口下发数据，用于命令的重发
unsigned char *hfserv_down_copy_data = NULL;
int hfserv_down_copy_len=0;
void acquire_network_lq(unsigned char *rbuf);

/********************************************************************************
*	Description	: 对云端数据进行补全，增加头尾校验位
*	Name		: down_data_doIntegrate
*	Input		: type 
                              data
                              len
*	Returns		: None
 ********************************************************************************/
	
static void down_data_doIntegrate(unsigned char *data, int *len)
{
	// TODO: 下发UART之前数据完整性处理
	u_printf("****************************down_data_doIntegrate********************************\r\n");
	
}


/********************************************************************************
*	Description	: 收到云端数据后的处理行为
*	Name		: down_data_action
*	Input		: data
                              len     
*	Returns		: None
 ********************************************************************************/
void down_data_action(unsigned char *data, int len)
{
	if (DEVICE_DONT_NEED_MCU==1)//no mcu
	{
		if (device_init_ok_sem!=NULL)
			hfthread_sem_signal(device_init_ok_sem);
	}
	else
	{
		if(hfsys_get_run_mode() == HFSYS_STATE_RUN_THROUGH)
		{
			if(data != NULL)
			{
				fireUartSend(data, len);
				memcpy((char *)hfserv_down_copy_data, (char *)data, len);
				hfserv_down_copy_len = len;
			}
			else
			{
				fireUartSend(hfserv_down_copy_data, hfserv_down_copy_len);
			}
		}
	}
}

/********************************************************************************
*	Description	: 云数据接收，接收状态控制
*	Name		: hfservice_cloud_data_recv
*	Input		: type 
                              data
                              len
*	Returns		: None
 ********************************************************************************/
void hfservice_cloud_data_recv(enum CLOUD_DATA_TYPE type, unsigned char *data, int *len)
{
	if (type == HFSERV_EVENT_WIFI_CONNECT)
	{
		// TODO: WiFi 连接成功
		u_printf("WIFI连接成功\n");
	}
	else if (type == HFSERV_EVENT_WIFI_DISCONNECT)
	{
		// TODO: WiFi 断开
		
		u_printf("WIFI连接断开\n");
	}
	else if (type == HFSERV_EVENT_CLOUD_CONNECT)
	{
		// TODO: 云连接成功
		u_printf("云连接成功\n");
		uart_data_process(hfserv_down_copy_data, hfserv_down_copy_len);
	}
	else if (type == HFSERV_EVENT_CLOUD_DISCONNECT)
	{
		// TODO: 云断开
		u_printf("云连接断开\n");
	}
	else if ((type==HFSERV_CMD_LOCAL_CTRL)||(type==HFSERV_CMD_REMOTE_CTRL))
	{
		downstream(data);
		if (outLen<=0)
			return ;
		down_data_doIntegrate(dataOut, &outLen);
		if (type==HFSERV_CMD_REMOTE_CTRL)
			flag_remote_waiting_for_rpl=1;
		else 
			flag_local_waiting_for_rpl=1;
		
		if (UART_DOWN_NEED_ACK==0)
		{
			setConfirmed();
		}
		else
		{
			//char data=HFSERV_UART_ACK_START;
			hfthread_sem_signal(uart_ack_sem_start);
		}
	}
	else if (type == HFSERV_CMD_REMOTE_GET)
	{
		// TODO: 状态查询
		memcpy(data, hfserv_up_copy_data, hfserv_up_copy_len);
		*len = hfserv_up_copy_len;
	}
	else if (type == HFSERV_UPDATE_RSP)
	{
		// TODO: 主动上报回复
		HFService_device_upload = 1;
	}
	else if (type==HFSERV_CMD_MENU_CTRL)
	{
		// TODO: 处理云任务
	}
	
}

void acquire_network_lq(unsigned char *rbuf)
{
	char rsp[64]={0};//存放AT命令结果
	hfat_send_cmd("AT+WSLQ\r\n",sizeof("AT+WSLQ\r\n"),rsp,64);
	char *tmp=strstr(rsp, "%");
	if (tmp!=NULL)
	{
		*tmp=0;
		tmp=strstr(rsp,",");
		if (tmp!=NULL)
		{
			*rbuf = atoi(tmp+2);
			u_printf("[wslq]=%d\r\n",*rbuf);
		}
	}
}
 
