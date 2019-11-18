
#include <hsf.h>
#include "mqtt.h"
#include "MQTTClient.h"


#define MQTT_DEBUG_LEVEL 5


static struct MQTT_PRA g_mqtt_config;
static Client MQTTCli;
static int mqtt_is_connected = 0;

#define MQTTCliBufLen 						1024
#define MQTTCliReadbufLen 					1024
#define MQTTCli_Command_Timeout_MS 		1000
#define MQTTCli_KeepAliveInterval_S 			120


static void mqtt_status_callback(int connect)
{
	if(connect)
	{
		if(mqtt_is_connected == 0)
			hfuart_send(HFUART0, "+MQSTATUS:MQTT CONNECTED", strlen("+MQSTATUS:MQTT CONNECTED"), 0);
		mqtt_is_connected = 1;
	}
	else
	{
		if(mqtt_is_connected)
			hfuart_send(HFUART0, "+MQSTATUS:MQTT CLOSED", strlen("+MQSTATUS:MQTT CLOSED"), 0);
		mqtt_is_connected = 0;
	}
}

static void topic_message_callback(MessageData* md)
{   
	HF_Debug(MQTT_DEBUG_LEVEL, "************ Receive Message ***********\n");
	HF_Debug(MQTT_DEBUG_LEVEL, "%.*s\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	HF_Debug(MQTT_DEBUG_LEVEL, "%.*s\n", (int)md->message->payloadlen, (char*)md->message->payload);

	char *data = (char *)hfmem_malloc(md->message->payloadlen+md->topicName->lenstring.len+32);
	if(data == NULL)
		return;

	sprintf(data, "+MQD:%d,\"%.*s\"\r\n%.*s", (int)md->message->payloadlen, 
		md->topicName->lenstring.len, md->topicName->lenstring.data, (int)md->message->payloadlen, (char*)md->message->payload);
	
	hfuart_send(HFUART0, data, strlen(data), 0);
	hfmem_free(data);
}

static void topic_message_publish(char *topic, char *messagem, int message_len, int qos)
{   
	MQTTMessage message;
	
	message.qos = qos;    
	message.retained = FALSE;    
	message.dup = FALSE;    
	message.payload = (void*)messagem;    
	message.payloadlen = message_len;
	MQTTPublish(&MQTTCli, topic, &message);
}

static void MQTTClient_thread(void *arg)
{	
	int ping_time=0;
	HF_Debug(MQTT_DEBUG_LEVEL, "MQTTClient_thread start.\r\n");
	
	int ret;
	MQTTPacket_connectData MQTTConnectData = MQTTPacket_connectData_initializer; 
	Network cliNetwork;
	NewNetwork(&cliNetwork);

	char *MQTTCliBuf;
	MQTTCliBuf = (char *)hfmem_malloc(MQTTCliBufLen);
	char *MQTTCliReadbuf;
	MQTTCliReadbuf = (char *)hfmem_malloc(MQTTCliReadbufLen);
	
	while(1)
	{
		HF_Debug(MQTT_DEBUG_LEVEL, "Wait WiFi link ...");
		while(!hfwifi_sta_is_connected())
		{
			msleep(1000);
		}
		HF_Debug(MQTT_DEBUG_LEVEL, "connected.\r\n");

		if(SUCCESS != ConnectNetwork(&cliNetwork, g_mqtt_config.seraddr, g_mqtt_config.port))
		{
			HF_Debug(MQTT_DEBUG_LEVEL, "TCPSocket connect fail!\r\n");
			goto MQTT_END;
		}
		HF_Debug(MQTT_DEBUG_LEVEL, "TCPSocket connect success.\r\n");

		MQTTClient(&MQTTCli, &cliNetwork, MQTTCli_Command_Timeout_MS, (unsigned char *)MQTTCliBuf, MQTTCliBufLen, (unsigned char *)MQTTCliReadbuf, MQTTCliReadbufLen);
		if(strlen(g_mqtt_config.will_topic) > 0 && strlen(g_mqtt_config.will_msg) > 0)
		{
			MQTTConnectData.willFlag = 1;
			MQTTConnectData.will.topicName.cstring = g_mqtt_config.will_topic;
			MQTTConnectData.will.message.cstring = g_mqtt_config.will_msg;
		}
		else
			MQTTConnectData.willFlag = 0;
		MQTTConnectData.MQTTVersion = g_mqtt_config.mqtt_ver;
		MQTTConnectData.clientID.cstring = g_mqtt_config.clientid;
		MQTTConnectData.username.cstring = g_mqtt_config.username;
		MQTTConnectData.password.cstring = g_mqtt_config.password;
		MQTTConnectData.keepAliveInterval = MQTTCli_KeepAliveInterval_S;
		MQTTConnectData.cleansession = 1;
			
		if(SUCCESS != MQTTConnect(&MQTTCli, &MQTTConnectData))
		{
			HF_Debug(MQTT_DEBUG_LEVEL, "MQTTConnect fail!\r\n");
			goto MQTT_END;
		}
		HF_Debug(MQTT_DEBUG_LEVEL, "MQTTConnect success.\r\n");

		if(g_mqtt_config.enable_sub)
		{
			if(SUCCESS == MQTTSubscribe(&MQTTCli, g_mqtt_config.sub_topic, g_mqtt_config.sub_qos, topic_message_callback))
				HF_Debug(MQTT_DEBUG_LEVEL, "Subscribe[%s] success.\r\n", g_mqtt_config.sub_topic);
			else
				HF_Debug(MQTT_DEBUG_LEVEL, "Subscribe[%s] fail!\r\n", g_mqtt_config.sub_topic);
		}

		mqtt_status_callback(1);
		
		ping_time = hfsys_get_time();
		while(1)
		{
			ret = MQTTYield(&MQTTCli, 300);
			if(ret == FAILURE)
			{
				goto MQTT_END;
			}

			if(!hfwifi_sta_is_connected())
			{
				HF_Debug(MQTT_DEBUG_LEVEL, "MQTTClient_thread WiFi disconnected!\r\n");
				goto MQTT_END;
			}
			
			//应用层实现ping功能 防止 mqtt异常断开
			if(hfsys_get_time() - ping_time > (MQTTConnectData.keepAliveInterval-10)*1000)
			{
				ping_time = hfsys_get_time();
				int len = MQTTSerialize_pingreq(MQTTCli.buf, MQTTCli.buf_size);
				if(len > 0)
				{
					MQTTCli.ipstack->mqttwrite(MQTTCli.ipstack, MQTTCli.buf, len, 100);
				}
			}
		}
		
MQTT_END:	
		HF_Debug(MQTT_DEBUG_LEVEL, "MQTT end!\r\n\r\n");
		cliNetwork.disconnect(&cliNetwork);
		mqtt_status_callback(0);
		msleep(3000);
	}	

	if(MQTTCliBuf != NULL)
		hfmem_free(MQTTCliBuf);
	if(MQTTCliReadbuf != NULL)
		hfmem_free(MQTTCliReadbuf);

	HF_Debug(MQTT_DEBUG_LEVEL, "MQTTClient_thread destroy.\r\n");
	msleep(1000);
	hfthread_destroy(NULL);
}

static void mqtt_start(void)
{
	static int mqtt_init = 0;
	if(mqtt_init)
		return;
	
	if(hfthread_create(MQTTClient_thread, "MQTT_Cli", 512, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL)!= HF_SUCCESS)
	{
		HF_Debug(DEBUG_ERROR,"start MQTTClient_thread fail\n");
	}
	mqtt_init=1;
}

static unsigned char crc_calc(unsigned char *data, int len)
{
	unsigned int crc = 0;
	int i;
	for(i=0; i<len; i++)
		crc += data[i];

	return crc;
}

static void default_mqtt_config(struct MQTT_PRA *mqtt)
{
	char mac[21] = {0};
	hfnet_get_mac_address(mac);

	memset((char *)mqtt, 0, sizeof(struct MQTT_PRA));
	strcpy(mqtt->seraddr, "121.40.218.73");
	mqtt->port = 1883;
	strcpy(mqtt->clientid, mac);
	strcpy(mqtt->username, "admin");
	strcpy(mqtt->password, "admin");
	strcpy(mqtt->pub_topic, "\0");
	mqtt->enable_sub = 0;
	mqtt->mqtt_ver =4;
}

static void save_mqtt_config(struct MQTT_PRA *mqtt)
{
	mqtt->magic_head = MQTT_MAGIC_HEAD;
	mqtt->crc = crc_calc((unsigned char *)mqtt, sizeof(struct MQTT_PRA)-1);
	hffile_userbin_write(MQTT_CONFIG_USERBIN_ADDR, (char *)mqtt, sizeof(struct MQTT_PRA));
}

int hf_atcmd_mqclientid(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		sprintf(rsp, "=%s", g_mqtt_config.clientid);
	}
	else if(argc == 1)
	{
		if(strlen(argv[0]) > MQTT_CLIENTID_MAX_LEN)
			return -4;
		strcpy(g_mqtt_config.clientid, argv[0]);
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqipport(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		sprintf(rsp, "=%s,%d", g_mqtt_config.seraddr, g_mqtt_config.port);
	}
	else if(argc == 2)
	{
		if(strlen(argv[0]) > MQTT_SERADDR_MAX_LEN)
			return -4;
		if(atoi(argv[1]) > 65535)
			return -4;
		
		strcpy(g_mqtt_config.seraddr, argv[0]);
		g_mqtt_config.port = atoi(argv[1]);
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mquserpwd(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		sprintf(rsp, "=%s,%s", g_mqtt_config.username, g_mqtt_config.password);
	}
	else if(argc == 2)
	{
		if(strlen(argv[0]) > MQTT_USERNAME_MAX_LEN)
			return -4;
		if(strlen(argv[1]) > MQTT_PASSWORD_MAX_LEN)
			return -4;

		strcpy(g_mqtt_config.username, argv[0]);
		strcpy(g_mqtt_config.password, argv[1]);
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqstatus(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		if(mqtt_is_connected)
			sprintf(rsp, "=CONNECTED");
		else
			sprintf(rsp, "=DISCONNECTED");
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqstart(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		mqtt_start();
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqpublish(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc==0)
	{
		sprintf(rsp, "=%s", g_mqtt_config.pub_topic);
	}
	else if(argc==1)
	{
		char *topic = argv[0];
		if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
			return -4;
		
		strcpy(g_mqtt_config.pub_topic, topic);
		save_mqtt_config(&g_mqtt_config);
	}
	else if(argc == 2)
	{
		char *topic = argv[0];
		int len = atoi(argv[1]);
		if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
			return -4;
		if(len > 1000)
			return -4;

		char *data=(char *)hfmem_malloc(len);
		if(data == NULL)
			return -5;

		hfuart_send(HFUART0, (char *)">", 1, 0);
		len = hfuart_recv(HFUART0, data, len, 3000);
		topic_message_publish(topic, data, len, 0);
		sprintf(rsp, "=%d", len);

		hfmem_free(data);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqsubscribe(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 2)
	{
		if(strlen(argv[0]) > MQTT_TOPIC_MAX_LEN)
			return -4;
		if(atoi(argv[1]) > 2)
			return -4;

		strcpy(g_mqtt_config.topic, argv[0]);
		unsigned char qos =  atoi(argv[1]);
		if(MQTTSubscribe(&MQTTCli, g_mqtt_config.topic, qos, topic_message_callback) == SUCCESS)
			sprintf(rsp, "=SUBSCRIBE SUCCESS");
		else
			sprintf(rsp, "=SUBSCRIBE FAIL");
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqunsubscribe(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 1)
	{
		if(strlen(argv[0]) > MQTT_TOPIC_MAX_LEN)
			return -4;

		char *topic = argv[0];
		if(MQTTUnsubscribe(&MQTTCli, topic) == SUCCESS)
			sprintf(rsp, "=UNSUBSCRIBE SUCCESS");
		else
			sprintf(rsp, "=UNSUBSCRIBE FAIL");
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqautosub(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		if(g_mqtt_config.enable_sub)
			sprintf(rsp, "=%d,%s,%d", g_mqtt_config.enable_sub, g_mqtt_config.sub_topic, g_mqtt_config.sub_qos);
		else
			sprintf(rsp, "=%d", g_mqtt_config.enable_sub);
	}
	else if(argc == 1)
	{
		int enable = atoi(argv[0]);
		if(enable)
			return -4;
		g_mqtt_config.enable_sub = enable;	
		save_mqtt_config(&g_mqtt_config);
	}
	else if(argc == 3)
	{
		int enable = atoi(argv[0]);
		char *topic = argv[1];
		int qos =atoi(argv[2]);
		
		if(enable != 0 && enable != 1)
			return -4;
		if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
			return -4;
		if(qos != 0 && qos != 1 && qos != 2)
			return -4;

		g_mqtt_config.enable_sub = enable;	
		strcpy(g_mqtt_config.sub_topic, topic);
		g_mqtt_config.sub_qos = qos;
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqres(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		default_mqtt_config(&g_mqtt_config);
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqver(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		sprintf(rsp, "=%d", g_mqtt_config.mqtt_ver);
	}
	else if(argc == 1)
	{
		int ver = atoi(argv[0]);
		if(ver != 3 && ver != 4)
			return -4;

		g_mqtt_config.mqtt_ver = ver ;
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

int hf_atcmd_mqwill(pat_session_t s,int argc,char *argv[],char *rsp,int len)
{
	if(argc == 0)
	{
		sprintf(rsp, "=%s,%s", g_mqtt_config.will_topic, g_mqtt_config.will_msg);
	}
	else if(argc == 2)
	{
		char *topic = argv[0];
		char *msg = argv[1];
		if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
			return -4;
		if(strlen(msg) > MQTT_MESSAGE_MAX_LEN)
			return -4;
		
		strcpy(g_mqtt_config.will_topic, topic);
		strcpy(g_mqtt_config.will_msg, msg);
		save_mqtt_config(&g_mqtt_config);
	}
	else
		return -3;
	
	return 0;
}

void mqtt_para_init(void)
{
	unsigned char crc;
	memset((char *)&g_mqtt_config, 0, sizeof(struct MQTT_PRA));
	hffile_userbin_read(MQTT_CONFIG_USERBIN_ADDR, (char *)&g_mqtt_config, sizeof(struct MQTT_PRA));
	crc = crc_calc((unsigned char *)&g_mqtt_config, sizeof(struct MQTT_PRA)-1);
	if(MQTT_MAGIC_HEAD == g_mqtt_config.magic_head && crc == g_mqtt_config.crc)
	{
		u_printf("[MQTT] mqtt config read success\r\n");
		u_printf("[MQTT] mqtt version: %d\r\n", g_mqtt_config.mqtt_ver);
		u_printf("[MQTT] mqtt ip: %s, port: %d\r\n", g_mqtt_config.seraddr, g_mqtt_config.port);
		u_printf("[MQTT] mqtt clientid: %s\r\n", g_mqtt_config.clientid);
		u_printf("[MQTT] mqtt username: %s\r\n", g_mqtt_config.username);
		u_printf("[MQTT] mqtt password: %s\r\n", g_mqtt_config.password);
		if(g_mqtt_config.enable_sub)
		{
			u_printf("[MQTT] mqtt subscribe topic: %s\r\n", g_mqtt_config.sub_topic);
			u_printf("[MQTT] mqtt subscribe qos: %d\r\n", g_mqtt_config.sub_qos);
		}
		if(strlen(g_mqtt_config.will_topic) > 0 && strlen(g_mqtt_config.will_msg) > 0)
		{
			u_printf("[MQTT] mqtt will topic: %s\r\n", g_mqtt_config.will_topic);
			u_printf("[MQTT] mqtt will message: %s\r\n", g_mqtt_config.will_msg);
		}
	}
	else
	{
		u_printf("[MQTT] mqtt config read failed, reinit\r\n");
		
		default_mqtt_config(&g_mqtt_config);
		save_mqtt_config(&g_mqtt_config);
	}
}

void uart_message_publish(char *data, int len)
{
	if(memcmp(g_mqtt_config.pub_topic,"\0",1)==0)
	{
		u_printf("mqtt pub_topic is null,please set by \"AT+MQPUBTOPIC\".\r\n");
	}
	topic_message_publish(g_mqtt_config.pub_topic, data, len, 0);
}

