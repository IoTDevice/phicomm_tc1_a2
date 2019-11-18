
#ifndef __M2M_SOCKET_H__
#define __M2M_SOCKET_H__

#include "hsf.h"

//default MQTT version: 3.1.1

#define MQTT_TOPIC_MAX_LEN 64
#define MQTT_TOPIC_MAX_NUM 5
#define MQTT_BEAT_TIME_SEC 60

/*Error No*/
#define HFMQTT_ERR_NOT_INIT				-1
#define HFMQTT_ERR_ADDR_OVERSIZE			-2
#define HFMQTT_ERR_PORT_OVERSIZE			-3
#define HFMQTT_ERR_CLIENTID_OVERSIZE		-4
#define HFMQTT_ERR_USERNAME_OVERSIZE	-5
#define HFMQTT_ERR_PASSWD_OVERSIZE		-6
#define HFMQTT_ERR_TOPIC_OVERSIZE			-7
#define HFMQTT_ERR_TOPIC_FULL				-8
#define HFMQTT_ERR_TOPIC_NOFIND			-9
#define HFMQTT_ERR_TOPIC_REPET			-10
#define HFMQTT_ERR_NOT_CONNECTED			-11
#define HFMQTT_ERR_NO_MEMORY				-12


typedef enum
{
	HFMQTT_CONNECTED=0,
	HFMQTT_DISCONNECTED,
	HFMQTT_TOPIC_SUB_SUCCESSED,
	HFMQTT_TOPIC_SUB_DATA,
	HFMQTT_TOPIC_UNSUB_SUCCESSED,
	HFMQTT_PING_RESPONSE
}hfmqtt_event_id_t;


typedef int (*mqtt_callback_t)(hfmqtt_event_id_t event, char *topic, unsigned char *data, int data_len);


int hfmqtt_init(mqtt_callback_t callback);

int hfmqtt_login(void *fd);

int hfmqtt_error(void);

int hfmqtt_set_clientid(char *clientid);

int hfmqtt_set_auth(char *username, char *password);

int hfmqtt_set_alive_time(int second);

int hfmqtt_data_parse(char *data);

int hfmqtt_sub(char *topic);

int hfmqtt_unsub(char *topic);

int hfmqtt_pub(char *topic, unsigned char *data, int data_len);

int hfmqtt_ping(void);

int hfmqtt_close(void);


#endif/*__M2M_SOCKET_H__*/

