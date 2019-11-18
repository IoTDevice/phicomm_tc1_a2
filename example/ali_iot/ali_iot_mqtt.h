#ifndef _ALI_IOT_MQTT_
#define _ALI_IOT_MQTT_

#include "MQTTClient.h"

#define ALI_MQTT_SERVER_ADDR ali_mqtt_server_addr
#define ALI_MQTT_SERVER_PORT 1883

#define IOT_PRODUCTKEY (AliMqttConnectPara.ProductKey)
#define DEVICENAME (AliMqttConnectPara.DeviceName)
#define HMACSHA1_KEY (AliMqttConnectPara.DeviceSecret)
#define HMACSHA1_KEY_LEN strlen((AliMqttConnectPara.DeviceSecret))

#define SIGNMETHOD "hmacsha1"
#define TLS_DIRECT_CONNECT_MODE 2
#define TCP_DIRECT_CONNECT_MODE 3


#define ALI_MQTT_CONNECT_PARA_UFLASH_ADDR 0
#define ALI_MQTT_CONNECT_PARA_HEAD 0XAA
#define ALI_MQTT_CONNECT_PARA_TAIL 0XCC

#pragma pack (1) /*指定按1字节对齐*/
typedef struct _ALI_MQTT_CONNECT_PARA
{
    unsigned char head;
    char ProductKey[64];
    char DeviceName[64];
    char DeviceSecret[64];
    char device_pubtopic[64];
    char device_subtopic[64];
    unsigned char tail;
}ALI_MQTT_CONNECT_PARA;
#pragma pack () /*取消指定对齐，恢复缺省对齐*/

extern ALI_MQTT_CONNECT_PARA AliMqttConnectPara;

extern char mqtt_pub_topic[256];

extern void hmac_sha1(unsigned char *key,int key_length,unsigned char *data,int data_length,unsigned char *digest);

int Ali_MQTT_thread_start(void);

int ali_mqtt_message_publish(char *topic,const char *data,int len,enum QoS qos);

#endif

