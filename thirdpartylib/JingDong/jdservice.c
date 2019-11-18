
/*****************************************************************************************
* File Name: jdservice.c
*
* Description:
*  Common JingDong Service file.
*
*****************************************************************************************/

#include <hsf.h>
#include "jdsmart.h"
#include "jdservice.h"
#include "joylink_config.h"

//#define JD_UUID	"HN9WFD"//Hi-flying test UUID


//#define PLUG_DEMO
int tcp_flag = NEEDON_CREAT_SOCKET;

#ifdef PLUG_DEMO
static unsigned char power_state = 0;
#endif

static unsigned char jd_connect_event=0;

/*
*name:	int jdservice_get_uuid(char *uuid)
*Para:	uuid
*return:	uuid len
*/
int jdservice_get_uuid(char *uuid)
{
	memcpy(uuid, DEVICE_UUID, strlen(DEVICE_UUID));
	return strlen(DEVICE_UUID);
}

/*
*name:	void jdservice_disconnected(void)
*Para:	none
*return:	none
*/
void jdservice_disconnected(void)
{
    jd_connect_event = 0;
	jd_send_event(JD_CLO_DISCONNECTED, NULL); 
	return;
}

/*
*name:	void jdservice_connected(void)
*Para:	none
*return:	none
*/
void jdservice_connected(void)
{
    if(jd_connect_event == 0)
    {
		jd_send_event(JD_CLO_CONNECTED, NULL);
		jd_connect_event = 1;

		//extern int jd_is_server_connected;
	    //jd_is_server_connected = 1;
    }
#ifdef PLUG_DEMO
	jdservice_upload_data((char *)&power_state, 1);
#endif
	
	return;
}

/*
*name:	int jdservice_cmd_control(int code, char *cmd, int cmd_len, char *rsp, int rsp_max)
*Para:	code: 1002/1004/1050
              cmd
              cmd_len
              rsp
              rsp_max
*return:	len
*/
int jdservice_cmd_control(int code, char *cmd, int cmd_len, char *rsp, int rsp_max)
{

#ifdef PLUG_DEMO
	//A Plug test
	if(code == JOYLINK_CTRL_CODE_SET)
	{
		if(cmd[0] == 0)
		{
			HF_Debug(10, "Cmd: OFF\r\n");
		}
		else if(cmd[0] == 1)
		{
			HF_Debug(10, "Cmd: ON\r\n");
		}
		
		power_state = cmd[0];
	}
	else if(code == JOYLINK_CTRL_CODE_GET)
	{
		
	}
	else if(code == JOYLINK_CTRL_CODE_TASK)
	{
		
	}

	rsp[0] = power_state;
	return 1;
	
#else

	return 0;

#endif

}

/*
*name:	void jdservice_main_loop(void)
*Para:	none
*return:	none
*/
void jdservice_main_loop(void)
{

}

void set_creat_tcp_flags(int flag)
{
	tcp_flag=flag;
}
int get_creat_tcp_flags(void)
{
	return tcp_flag;
}

