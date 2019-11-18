#include <hsf.h>		
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "HFService_config.h"
#include "HFService_upstream.h"
#include "HFService_localprocess.h"
#include "HFService_helper.h"


// TODO: UART协议头标识
// UART_DATA_LENGTH_MIN必须大于sizeof(uart_proto_head)
// UART_DATA_LENGTH_MIN可以为包头的长度
#define UART_DATA_LENGTH_MIN			4
#define UART_DATA_LENGTH_MAX			100
unsigned char uart_proto_head[]={0xAA};
#define HFSERV_CMD_SMTLK_OK             1
#define HFSERV_CMD_SMTLK_FAIL           0

// uart数据接收Buffer
unsigned char *hfserv_uart_recv_data = NULL;
// uart Buffer中数据长度
int hfserv_uart_recv_len=0;
// 上次接收uart 数据的时间
int hfserv_uart_last_recv;			// 上次接收时间
// 设备串口初始化标识
int HFService_device_inited=0;
// 设备主动上报成功标识
int HFService_device_upload=0;

//保存串口上报数据，用于保存本地状态
unsigned char *hfserv_up_copy_data = NULL;
int hfserv_up_copy_len=0;
unsigned char product_test_hf = 0;
int previous_len = 0;


/********************************************************************************
*	Description	:  识别包头，返回包头位置
*	Name		: uart_proto_find_head
*	Input		: data
                              len     
*	Returns		: None
 ********************************************************************************/
static int uart_proto_find_head(unsigned char *data,int len)
{
	u_printf("*********************uart_proto_find_head**************************\r\n");
	int pos=0;
	int head_len=sizeof(uart_proto_head);
	
	if (head_len==0)
		return 0;
	while (pos+head_len<len)
	{
		// TODO: 包头识别
		if (memcmp(data+pos,uart_proto_head,head_len)==0)
		{
		    u_printf("find head\r\n");
			return pos;
		}
			
		pos++;
	}
	return -1;
}

static int hfwifi_scan_test( PWIFI_SCAN_RESULT_ITEM scan_ret)
{
	if(scan_ret == NULL)
		return 0;
	unsigned char HFTEST[] = "HF-TEST";
	u_printf("%s    \n", scan_ret->ssid);	
	if((memcmp(scan_ret->ssid,HFTEST,7) == 0))
	{
		product_test_hf = 1;
	}
	
	return 0;
}

/********************************************************************************
*	Description	:  获取数据长度，返回长度值
*	Name		: uart_proto_get_len
*	Input		: data:uart收到的数据
				  head_pos:包头位置
				  len:数据总长度    
*	Returns		: >0 有效数据包长度
				  =0 数据没收完整
				  <0 无效数据包,|ret|为无效数据长度
 ********************************************************************************/
static int uart_proto_get_len(unsigned char *data,int head_pos,int len)
{
	// TODO: 获取数据长度，数据长度合理性检查，回复数据长度
	
	return len;
}

/********************************************************************************
*	Description	:   数据包合法性检查,如:CRC check,
*	Name		: uart_proto_legal_check
*	Input		: data:数据包起始地址
				  proto_len:数据包长度
*	Returns		: HFSERV_STATE_OK  or HFSERV_STATE_FAILE
 ********************************************************************************/
	static int uart_proto_legal_check(unsigned char *data,int proto_len)
	{
		// TODO: 合法性检查
		return HFSERV_STATE_OK;
	}

/********************************************************************************
*	Description	:  命令分类
*	Name		: uart_proto_legal_check
*	Input		: data:数据包起始地址
				  proto_len:数据包长度
*	Returns		: HFSERV_UART_CMD_XXXX or HFSERV_UART_DATA_XXX
 ********************************************************************************/
static int uart_cmd_classify(unsigned char *data,int proto_len_data)
{
	if (HFService_device_inited == 0)
		return HFSERV_UART_CMD_INIT;
	switch(data[0])
	{
	case 0x07:
		return HFSERV_UART_CMD_SMTLK;
		break;
	case 0x06:
		return HFSERV_UART_CMD_ERROR;
		break;
	case 0x08:
		return HFSERV_UART_CMD_WORK;
		break;
	case 0x0A:
		return HFSERV_UART_CMD_ACK;
		break;
	case 0x0B:
		return HFSERV_UART_CMD_DACTORY;
		break;
	case 0xAA:
		return HFSERV_UART_CMD_DATA;
		break;
	}
	// TODO:
	//命令分类
	return HFSERV_UART_CMD_NULL;
}

/********************************************************************************
*	Description	:  数据包处理
*	Name		: uart_proto_legal_check
*	Input		: data:数据包起始地址
				  proto_len:数据包长度
*	Returns		: none
 ********************************************************************************/

void uart_data_process(unsigned char *data,int proto_len)
{

	int uart_cmd=uart_cmd_classify(data,proto_len);
	
	if (uart_cmd==HFSERV_UART_CMD_SMTLK)
	{
	    // TODO: 开始SmartLink
		hfsmtlk_start();
	}
	else if (uart_cmd==HFSERV_UART_CMD_RELD)
	{
		// TODO: 开始恢复用户Config
		hfsys_reload();
	}
	else if (uart_cmd==HFSERV_UART_CMD_DACTORY)
	{
		// TODO: 产测指令
		hfwifi_scan(hfwifi_scan_test);
		
	}
	else if (uart_cmd==HFSERV_UART_CMD_RESET)
	{
		// TODO: 开始模块复位
		hfsys_reset();
	}	
	else if (uart_cmd==HFSERV_UART_CMD_INIT)
	{
		// TODO: 设备init数据
		
		if(hfserv_down_copy_data != NULL)
		{
			HFService_device_inited = 1;
			HFService_device_upload = 0;
			memcpy((char *)hfserv_down_copy_data, (char *)data, proto_len);
			hfserv_down_copy_len = proto_len;

			hfthread_sem_signal(device_init_ok_sem);
		}
	}
	else if (uart_cmd==HFSERV_UART_CMD_DATA)
	{
		// TODO: 业务数据包处理
		upstream(data);			// 调用代码生成工具生成的protocol_XXXX.c的函数
		if (outLen<=0)
			return;
		if ((UART_DOWN_NEED_ACK != 0)&&(flag_local_waiting_for_rpl + flag_remote_waiting_for_rpl > 0))
		{
			//char data=HFSERV_UART_ACK_STOP;
			hfthread_sem_signal(uart_ack_sem_stop);
		}
		
		if (flag_local_waiting_for_rpl)
		{
			hfservice_cloud_data_send(HFSERV_RSP_LOCAL_CTRL, dataOut, outLen);
			flag_local_waiting_for_rpl=0;
		}
		else if (flag_remote_waiting_for_rpl)
		{
			hfservice_cloud_data_send(HFSERV_RSP_REMOTE_CTRL, dataOut, outLen);
			flag_remote_waiting_for_rpl=0;
		}
		else
		{
			hfservice_cloud_data_send(HFSERV_UPDATE_DATA, dataOut, outLen);
			HFService_device_upload = 1;
		}
		memcpy((char *)hfserv_up_copy_data, (char *)dataOut, outLen);
		hfserv_up_copy_len = outLen;
	}
	
}

/********************************************************************************
*	Description	: UART数据完整性检查，并做下一步处理，
*	Name		: uart_proto_legal_check
*	Input		: data:数据包起始地址
				  len:数据包长度
*	Returns		: 返回时为已被处理的数据长度
 ********************************************************************************/
static int uart_proto_integrateCheck(unsigned char *data,int *len)
{
	int ret=0;
	int proto_head=uart_proto_find_head(data,*len);
	int proto_len;
	if (proto_head!=-1)			// 找到包头位置
	{
		proto_len=uart_proto_get_len(data,proto_head,*len);
		if (proto_len>0)			// 收到完整数据
		{
			if (uart_proto_legal_check(data+proto_head,proto_len)==HFSERV_STATE_OK)
			{
				uart_data_process(data+proto_head,proto_len);
				ret=HFSERV_STATE_OK;
				*len=proto_head+proto_len;
			}
			else
			{
				ret=HFSERV_STATE_FAILE;
				*len=proto_head+proto_len;
			}
		}
		else	 if (proto_len==0)		// 还没收完数据
		{
			ret=HFSERV_STATE_NONE;
			*len=proto_head;
		}
		else //if (proto_len<0)		// 收到无效数据
		{
			ret=HFSERV_STATE_FAILE;
			*len=proto_head+(-proto_len);
		}
	}
	else							// 没有包头，留下包标识长度的数据
	{
		if (*len>sizeof(uart_proto_head))
		{
			ret=HFSERV_STATE_FAILE;
			*len=(*len-sizeof(uart_proto_head));
		}
	}
	return ret;
}

/********************************************************************************
*	Description	: UART 数据接收，Buf 管理，接收状态控制
*	Name		: hfservice_uart_data_recv
*	Input		: data:数据包起始地址
				  len:数据包长度
*	Returns		: none
 ********************************************************************************/
void hfservice_uart_data_recv(unsigned char *data, int len)
{
	int tmp_len;
	int proto_len,state;

	// 如果间隔太长，则丢弃前面的数据
	if ((UART_DATA_BYTE_INTERVAL!=0)&&(hfsys_get_time()-hfserv_uart_last_recv>UART_DATA_BYTE_INTERVAL))
	{
		hfserv_uart_recv_len=0;
	}
	hfserv_uart_last_recv=hfsys_get_time();

	tmp_len=len;
	if (hfserv_uart_recv_len+len>DATA_OUT_MAX_SIZE)
		tmp_len=sizeof(hfserv_uart_recv_data)-hfserv_uart_recv_len;

	// 新数据放入Buffer
	memcpy(hfserv_uart_recv_data+hfserv_uart_recv_len,data,tmp_len);
	hfserv_uart_recv_len+=tmp_len;

	// 如果数据长度大于UART_DATA_LENGTH_MIN, 则处理数据
	while (hfserv_uart_recv_len>=UART_DATA_LENGTH_MIN)
	{
		proto_len=hfserv_uart_recv_len;
		//uart_data_process(data,len);
		//state=1;
		state=uart_proto_integrateCheck(hfserv_uart_recv_data,&proto_len);
		// return of proto_len is the length of non-used (or processed) data
		if (proto_len!=0)
		{
			memcpy(hfserv_uart_recv_data,hfserv_uart_recv_data+proto_len,hfserv_uart_recv_len-proto_len);
			// 注意memcpy是否会有BUG，
			hfserv_uart_recv_len-=proto_len;
		}
		if (state==HFSERV_STATE_NONE)
		{
			// 数据长度大于UART_DATA_LENGTH_MIN，但数据没收全
			break;
		}
	}
}


/********************************************************************************
*	Description	: 参数初始化，localProcess启动
*	Name		: HFService_init
*	Input		: none
*	Returns		: HFSERV_STATE_FAILE or HFSERV_STATE_OK
 ********************************************************************************/
int HFService_init(void)
{
	extern void setHeap(void *heap, int size);

	hfserv_uart_recv_data  = hfmem_malloc(DATA_OUT_MAX_SIZE);
	if(hfserv_uart_recv_data == NULL)
		return HFSERV_STATE_FAILE;
	hfserv_down_copy_data = hfmem_malloc(DATA_OUT_MAX_SIZE);
	if(hfserv_down_copy_data == NULL)
		return HFSERV_STATE_FAILE;

	hfserv_up_copy_data = hfmem_malloc(DATA_OUT_MAX_SIZE);
	if(hfserv_up_copy_data == NULL)
	{
		hfmem_free(hfserv_down_copy_data);
		return HFSERV_STATE_FAILE;
	}

	char *heap = hfmem_malloc(DATA_OUT_MAX_SIZE);
	if(heap == NULL)
	{
		hfmem_free(hfserv_down_copy_data);
		hfmem_free(hfserv_up_copy_data);
		return HFSERV_STATE_FAILE;
	}
	setHeap((void *)heap, DATA_OUT_MAX_SIZE);
	
	if (UART_DOWN_NEED_ACK==1)
	{
		hfthread_sem_new (&uart_ack_sem_start, 0);
		hfthread_sem_new (&uart_ack_sem_stop, 0);
		hfthread_create(process_uart_ack,"process_uart_ack", 256, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL);
	}
	hfthread_sem_new (&device_init_ok_sem, 0);
	hfthread_create(process_device_init,"process_device_init", 512, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL);
    
	if (DEVICE_DONT_NEED_MCU==1)
	{
		hfthread_sem_new (&local_mcu_sem, 0);
		hfthread_create(process_local_mcu,"process_local_mcu", 256, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL);
	}
	return HFSERV_STATE_OK;
}

