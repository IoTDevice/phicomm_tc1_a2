
#ifndef _MQTT_H_
#define _MQTT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hsf.h"


#define MQTT_CONFIG_USERBIN_ADDR		0


#define MQTT_SERADDR_MAX_LEN			128
#define MQTT_CLIENTID_MAX_LEN			128
#define MQTT_USERNAME_MAX_LEN		128
#define MQTT_PASSWORD_MAX_LEN		128
#define MQTT_TOPIC_MAX_LEN			128
#define MQTT_MESSAGE_MAX_LEN			128


#define MQTT_MAGIC_HEAD 0x5A5AA5A5

#pragma pack(push)
#pragma pack(1)

struct MQTT_PRA
{
	unsigned int magic_head;
	char seraddr[MQTT_SERADDR_MAX_LEN+1];
	unsigned short port;
	char clientid[MQTT_CLIENTID_MAX_LEN+1];
	char username[MQTT_USERNAME_MAX_LEN+1];
	char password[MQTT_PASSWORD_MAX_LEN+1];
	unsigned char enable_sub;
	char sub_topic[MQTT_TOPIC_MAX_LEN+1]; //for at+mqautosub topic
	char topic[MQTT_TOPIC_MAX_LEN+1];  //for at+mqsubscribe topic
	unsigned char sub_qos;
	unsigned char mqtt_ver;
	char will_topic[MQTT_TOPIC_MAX_LEN+1];
	char will_msg[MQTT_MESSAGE_MAX_LEN+1];
	char pub_topic[MQTT_TOPIC_MAX_LEN+1];
	unsigned char crc;
};

#pragma pack(pop)


void mqtt_para_init(void);


//MQTT AT cmd
int hf_atcmd_mqclientid(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqipport(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mquserpwd(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqstatus(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqstart(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqpublish(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqsubscribe(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqunsubscribe(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqautosub(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqres(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqver(pat_session_t s,int argc,char *argv[],char *rsp,int len);

int hf_atcmd_mqwill(pat_session_t s,int argc,char *argv[],char *rsp,int len);

//THROUGH mode uart message 
void uart_message_publish(char *data, int len);

#ifdef __cplusplus
}
#endif

#endif

