/*****************************************************************************************
* File Name: jdservice.h
*
* Description:
*  Common JingDong Service file.
*
*****************************************************************************************/

#ifndef _JDSERVICE_H_
#define _JDSERVICE_H_


#define JOYLINK_CTRL_CODE_SET 1002
#define JOYLINK_CTRL_CODE_GET 1004
#define JOYLINK_CTRL_CODE_TASK 1050

#define respHeadLen (3*sizeof(unsigned int))   
#define respBuffer_MAXLEN  (128)
enum
{
	NEED_CREAT_SOCKET,
	NEEDON_CREAT_SOCKET
};
/*
*name:	int jd_main(void);
*Para:	none
*return:	none
*/
int jd_main(void);

/*
*name:	int jdservice_get_uuid(char *uuid)
*Para:	uuid
*return:	uuid len
*/
int jdservice_get_uuid(char *uuid);

/*
*name:	void jdservice_disconnected(void)
*Para:	none
*return:	none
*/
void jdservice_disconnected(void);

/*
*name:	void jdservice_connected(void)
*Para:	none
*return:	none
*/
void jdservice_connected(void);

/*
*name:	int jdservice_cmd_control(int code, char *cmd, int cmd_len, char *rsp, int rsp_max)
*Para:	code: 1002/1003/1004
              cmd
              cmd_len
              rsp
              rsp_max
*return:	rsp_len
*/
int jdservice_cmd_control(int code, char *cmd, int cmd_len, char *rsp, int rsp_max);

/*
*name:	int jdservice_upload_data(char *senddata, int datalen)
*Para:	senddata
              datalen
*return:	send_len
*/
int jdservice_upload_data(char *senddata, int datalen);

/*
*name:	void jdservice_main_loop(void)
*Para:	none
*return:	none
*/
void jdservice_main_loop(void);
int V1_WriteRet(int code, char *msg);

void USER_FUNC hfservice_cloud_start(void);

typedef enum __JD_EVENT__
{
	JD_CLO_CONNECTED=0,
	JD_CLO_DISCONNECTED=1,
	JD_CLO_UPLOAD_RSP,
	JD_CLO_SOFTAP_START,
}JD_EVENT;
enum PROTO
{
	UDP_TRANSMIT,
	TCP_TRANSMIT,
	AUTO_TRANSMIT
};
typedef enum
{
    CTL_RESP,
    AUTO_UPDATE,
    
}RAWDATA_T;

#define JD_CALLBACK_NUM 1

typedef int (*jd_event_callback_t)( JD_EVENT /*event_id*/,void * /*param*/);

int jd_send_event(uint32_t event_id,void *param);
void set_creat_tcp_flags(int flag);

int get_creat_tcp_flags(void);


#endif
