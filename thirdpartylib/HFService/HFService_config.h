#ifndef _HFSERVICE_CONFIG_H_
#define _HFSERVICE_CONFIG_H_

#include "hsf.h"
#include "HFService_helper.h"

#define HFService_VER "HFService-1.0"

// TODO: HFService Config
#define DATA_OUT_MAX_SIZE				512

#define UART_DATA_BYTE_INTERVAL		500		// unit: ms
#define UART_DOWN_NEED_ACK			0
#define UART_DOWN_ACK_TIMEOUT		1000	// unit: ms
#define UART_DOWN_ACK_RETRY_NUM		3
#define DEVICE_DONT_NEED_MCU			0
/* HFService Config end*/

#define HFSERV_STATE_FAILE				-1
#define HFSERV_STATE_NONE				0
#define HFSERV_STATE_OK				1

#define HFSERV_UART_CMD_NONE				0
#define HFSERV_UART_CMD_SMTLK				1
#define HFSERV_UART_CMD_RELD				2
#define HFSERV_UART_CMD_RESET				3
#define HFSERV_UART_CMD_INIT				4
#define HFSERV_UART_CMD_DATA				5
#define HFSERV_UART_CMD_ERROR               6
#define HFSERV_UART_CMD_WORK                7
#define HFSERV_UART_CMD_ACK                 8
#define HFSERV_UART_CMD_DACTORY             9
#define HFSERV_UART_CMD_NULL				0xFF

#define HFSERV_UART_ACK_START				1
#define HFSERV_UART_ACK_STOP				2
#define CONNECT_ROUTER            1
#define DISCONNECT_SN_CLOUD       0
enum CLOUD_DATA_TYPE
{
	HFSERV_EVENT_WIFI_CONNECT=0x00,
	HFSERV_EVENT_WIFI_DISCONNECT,
	HFSERV_EVENT_CLOUD_CONNECT,
	HFSERV_EVENT_CLOUD_DISCONNECT,
	HFSERV_EVENT_DHCP_OK,
	
	HFSERV_CMD_LOCAL_CTRL,
	HFSERV_CMD_REMOTE_CTRL,
	HFSERV_CMD_MENU_CTRL,
	HFSERV_CMD_REMOTE_GET,
	
	HFSERV_RSP_LOCAL_CTRL,
	HFSERV_RSP_REMOTE_CTRL,
	HFSERV_RSP_MENU_CTRL,
	
	HFSERV_UPDATE_DATA,
	HFSERV_UPDATE_RSP,
	HFSERV_CMD_MAX_NUM,
	
	HFSERV_EVENT_SN_CONNECT
};
enum SMARTLINK_TYPE
{
	HFSERVICE_ALI = 0,
	HFSERVICE_JD = 1,
	HFSERVICE_SN = 2,
	HFSERVICE_HW = 3,
	HFSERVICE_OTHER = 0xFF,
};
#define HFSERVICE_SMTLK_TYPE HFSERVICE_SN;
// defined in protocol_XXX.c
extern unsigned char * dataOut;
extern int outLen;
extern int device_updated;
extern char check_flag[10];

// defined in HFService_upstream.c
extern int HFService_device_upload;
extern unsigned char *hfserv_up_copy_data;
extern int hfserv_up_copy_len;

// defined in HFService_downstream.c
extern int flag_local_waiting_for_rpl;
extern int flag_remote_waiting_for_rpl;
extern unsigned char *hfserv_down_copy_data;
extern int hfserv_down_copy_len;

// defined in protocol_XXX.c
extern void setConfirmed();
extern void upstream(unsigned char *data);
extern void downstream(unsigned char *data);

extern void hfservice_cloud_start(void);
//extern void hfservice_device_id_set(char *device_id);
extern void hfservice_cloud_data_send(enum CLOUD_DATA_TYPE type, unsigned char *data, int len);

#endif

