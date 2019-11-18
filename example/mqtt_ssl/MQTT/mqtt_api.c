
#include "hsf.h"
#include "mqttlib.h"
#include "mqtt_api.h"

#include <cyassl/openssl/ssl.h>
#include <cyassl/internal.h>
#include <cyassl/cyassl_config.h>


struct MQTT_PARA
{
	int mqtt_status;//0-disconnect, 1-connect
	mqtt_broker_handle_t mqtt_broker;
	mqtt_callback_t mqtt_callback;
	
	int sub_topic_flag[MQTT_TOPIC_MAX_NUM];//0-init, 1-sub, 2-subing, 3-sub success, 4-unsub
	char sub_topic_str[MQTT_TOPIC_MAX_NUM][MQTT_TOPIC_MAX_LEN+1];
	unsigned short sub_topic_msg_id[MQTT_TOPIC_MAX_NUM];

	int beat_time;
	int beat_flag;
};


static int mqtt_init_flag = 0;
static int mqtt_error_no = 0;
static struct MQTT_PARA g_mqtt_handle;


static void mqtt_send_callback(hfmqtt_event_id_t event, char *topic, unsigned char *data, int data_len)
{
	if(g_mqtt_handle.mqtt_callback != NULL)
		g_mqtt_handle.mqtt_callback(event, topic, data, data_len);
}

static void mqtt_set_errorno(int errorno)
{
	mqtt_error_no = errorno;
}

static int get_sub_topic_index(int flag)
{
	int i;
	for(i=0; i<MQTT_TOPIC_MAX_NUM; i++)
	{
		if(g_mqtt_handle.sub_topic_flag[i] == flag)
		{
			return i;
		}
	}

	return -1;
}

static int get_sub_topic_index_by_msg_id(int flag, unsigned short msg_id)
{
	int i;
	for(i=0; i<MQTT_TOPIC_MAX_NUM; i++)
	{
		if((g_mqtt_handle.sub_topic_flag[i] == flag) && (g_mqtt_handle.sub_topic_msg_id[i] == msg_id))
		{
			return i;
		}
	}

	return -1;
}

static int mqtt_send_packet(void* socket_info, const void* buf, unsigned int count)
{
	int ret = 0;

	ret = CyaSSL_write((CYASSL *)socket_info, buf, count);
	if(ret < 0)
		u_printf("SLL send fail\n");

	return ret;
}

static void mqtt_login_server(mqtt_broker_handle_t *broker, void *fd)
{
	broker->socket_info = (void*)fd;
	broker->send = mqtt_send_packet;

	if(mqtt_connect(broker)== -1)
		u_printf("Mqtt login server fail!\n");
	else
		u_printf("Do mqtt login ...\n");
}

static void mqtt_init_para(void)
{
	g_mqtt_handle.mqtt_status = 0;
	int i;
	for(i=0; i<MQTT_TOPIC_MAX_NUM; i++)
	{
		if(g_mqtt_handle.sub_topic_flag[i] != 0)
		{
			g_mqtt_handle.sub_topic_flag[i] = 1;
		}
		g_mqtt_handle.sub_topic_msg_id[i] = 0;
	}
	g_mqtt_handle.beat_time = 0;
	g_mqtt_handle.beat_flag = 0;
}

void app_parse_mqttmsg(uint8_t *packet_buffer)
{
	uint8_t msg_type = 0;
	uint16_t msg_id_rcv = 0;
	char topic_name[MQTT_TOPIC_MAX_LEN+1]={0};
	int recv_len;
	int index;
	char *recv_msg;
	
	msg_type = MQTTParseMessageType(packet_buffer);
	switch(msg_type)
	{
		case MQTT_MSG_DISCONNECT:
			u_printf("MQTT_MSG_DISCONNECT\r\n");
			hfmqtt_close();
			break;
			
		case MQTT_MSG_CONNACK:
			if(packet_buffer[3] == 0)
			{
				u_printf("Mqtt login server success\n");
				index= get_sub_topic_index(1);
				if(index >= 0)
				{
					mqtt_subscribe(&g_mqtt_handle.mqtt_broker, g_mqtt_handle.sub_topic_str[index], &g_mqtt_handle.sub_topic_msg_id[index]);
					g_mqtt_handle.sub_topic_flag[index] = 2;
				}
				else
				{
					g_mqtt_handle.mqtt_status = 1;
					mqtt_send_callback(HFMQTT_CONNECTED, NULL, NULL, 0);
				}
			}
			else
			{
				u_printf("Mqtt login server fail!\n");
				hfmqtt_close();
			}
			break;

		case MQTT_MSG_SUBACK:
			msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
			index = get_sub_topic_index_by_msg_id(2, msg_id_rcv);
			
			if(index >= 0)
			{
				u_printf("Subcribe topic[%s] success\n",g_mqtt_handle.sub_topic_str[index]);
				g_mqtt_handle.sub_topic_flag[index] = 3;
				mqtt_send_callback(HFMQTT_TOPIC_SUB_SUCCESSED, g_mqtt_handle.sub_topic_str[index], NULL, 0);

				index= get_sub_topic_index(1);
				if(index >= 0)
				{
					mqtt_subscribe(&g_mqtt_handle.mqtt_broker, g_mqtt_handle.sub_topic_str[index], &g_mqtt_handle.sub_topic_msg_id[index]);
					g_mqtt_handle.sub_topic_flag[index] = 2;
				}
				else
				{
					g_mqtt_handle.mqtt_status = 1;
					mqtt_send_callback(HFMQTT_CONNECTED, NULL, NULL, 0);
				}
			}
			break;

		case MQTT_MSG_PUBLISH:
			recv_msg = (char *)hfmem_malloc(1500);
			if(recv_msg ==  NULL)
			{
				u_printf("MQTT_MSG_PUBLISH ERROR no mem\r\n");
				mqtt_set_errorno(HFMQTT_ERR_NO_MEMORY);
				break;
			}
			
			mqtt_parse_pub_topic((const uint8_t*)packet_buffer, (uint8_t*)topic_name);	
			recv_len = mqtt_parse_publish_msg((const uint8_t*)packet_buffer, (uint8_t*)recv_msg);
			u_printf("****** Topic[%s] recv msg len [%d] *****\r\n",topic_name, recv_len);

			mqtt_send_callback(HFMQTT_TOPIC_SUB_DATA, topic_name, (unsigned char *)recv_msg, recv_len);

			hfmem_free(recv_msg);
			recv_msg = NULL;
			
			int Qos_level = MQTTParseMessageQos(packet_buffer);
			msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
			u_printf("msg_id_rcv=%x ,Qos_level=%d\r\n",msg_id_rcv,Qos_level);
			if(Qos_level == 1  ||Qos_level == 2)
				mqtt_puback(&g_mqtt_handle.mqtt_broker,msg_id_rcv);
				
			break;

		case MQTT_MSG_PUBACK://Qos1
			msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
			u_printf("publish success, msg_id[%d]\n", msg_id_rcv);
			break;

		case MQTT_MSG_UNSUBACK:
			msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
			index = get_sub_topic_index_by_msg_id(4, msg_id_rcv);
			
			if(index >= 0)
			{
				u_printf("Unsubcribe topic[%s] success\n", g_mqtt_handle.sub_topic_str[index]);
				
				g_mqtt_handle.sub_topic_flag[index] = 0;
				g_mqtt_handle.sub_topic_msg_id[index] = 0;
				mqtt_send_callback(HFMQTT_TOPIC_UNSUB_SUCCESSED, g_mqtt_handle.sub_topic_str[index], NULL, 0);
			}
			break;	

		case MQTT_MSG_PINGREQ:
			u_printf("MQTT_MSG_PINGREQ\r\n");
			break;

		case MQTT_MSG_PINGRESP:
			u_printf("MQTT_MSG_PINGRESP\r\n");
			mqtt_send_callback(HFMQTT_PING_RESPONSE, NULL, NULL, 0);
			break;

		case MQTT_MSG_PUBREC://Qos2
			break;

		case MQTT_MSG_PUBCOMP://Qos3
			break;

		default:
			u_printf("Unknow mqtt msg type\n");
			break;
	}
}

int hfmqtt_init(mqtt_callback_t callback)
{
	mqtt_init_flag = 1;
	
	memset(&g_mqtt_handle, 0, sizeof(g_mqtt_handle));
	mqtt_init_para();
	
	char mac[20];
	unsigned char mac_uint8[6];
	hfwifi_read_sta_mac_address(mac_uint8);
	sprintf(mac, "%02X%02X%02X%02X%02X%02X", mac_uint8[0], mac_uint8[1], mac_uint8[2], mac_uint8[3], mac_uint8[4], mac_uint8[5]);
	
	mqtt_init(&g_mqtt_handle.mqtt_broker, mac);
	mqtt_init_auth(&g_mqtt_handle.mqtt_broker, "demo", "demo");

	g_mqtt_handle.mqtt_callback = callback;

	return HF_SUCCESS;
}

int hfmqtt_login(void *fd)
{
	mqtt_login_server(&g_mqtt_handle.mqtt_broker, fd);
	return HF_SUCCESS;
}

int hfmqtt_error(void)
{
	if(mqtt_init_flag != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_INIT);
		return -1;
	}
	
	return mqtt_error_no;
}


int hfmqtt_set_clientid(char *clientid)
{
	if(mqtt_init_flag != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_INIT);
		return -1;
	}
	
	if(strlen(clientid) > MQTT_CONF_CLIENTID_LENGTH)
	{
		mqtt_set_errorno(HFMQTT_ERR_CLIENTID_OVERSIZE);
		return -1;
	}

	strcpy(g_mqtt_handle.mqtt_broker.clientid, clientid);
	return HF_SUCCESS;
}

int hfmqtt_set_auth(char *username, char *password)
{
	if(mqtt_init_flag != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_INIT);
		return -1;
	}
	
	if(strlen(username) > MQTT_CONF_USERNAME_LENGTH)
	{
		mqtt_set_errorno(HFMQTT_ERR_USERNAME_OVERSIZE);
		return -1;
	}

	if(strlen(password) > MQTT_CONF_PASSWORD_LENGTH)
	{
		mqtt_set_errorno(HFMQTT_ERR_PASSWD_OVERSIZE);
		return -1;
	}

	strcpy(g_mqtt_handle.mqtt_broker.username, username);
	strcpy(g_mqtt_handle.mqtt_broker.password, password);
	return HF_SUCCESS;
}

int hfmqtt_set_alive_time(int second)
{
	mqtt_set_alive(&g_mqtt_handle.mqtt_broker, second);
	return HF_SUCCESS;
}

int hfmqtt_data_parse(char *data)
{
	app_parse_mqttmsg((uint8_t *)data);
	return HF_SUCCESS;
}

int hfmqtt_sub(char *topic)
{
	if(mqtt_init_flag != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_INIT);
		return -1;
	}
	
	if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
	{
		mqtt_set_errorno(HFMQTT_ERR_TOPIC_OVERSIZE);
		return -1;
	}

	int i;
	for(i=0; i<MQTT_TOPIC_MAX_NUM; i++)
	{
		if((g_mqtt_handle.sub_topic_flag[i] != 0) && (strcmp(g_mqtt_handle.sub_topic_str[i], topic) == 0))
		{
			mqtt_set_errorno(HFMQTT_ERR_TOPIC_REPET);
			return -1;
		}
	}
	
	for(i=0; i<MQTT_TOPIC_MAX_NUM; i++)
	{
		if(g_mqtt_handle.sub_topic_flag[i] == 0)
		{
			g_mqtt_handle.sub_topic_flag[i] = 1;
			strcpy(g_mqtt_handle.sub_topic_str[i], topic);
			g_mqtt_handle.sub_topic_msg_id[i] = 0;
			return HF_SUCCESS;
		}
	}

	mqtt_set_errorno(HFMQTT_ERR_TOPIC_FULL);
	return -1;
}

int hfmqtt_unsub(char *topic)
{
	if(mqtt_init_flag != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_INIT);
		return -1;
	}
	
	if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
	{
		mqtt_set_errorno(HFMQTT_ERR_TOPIC_OVERSIZE);
		return -1;
	}

	int i;
	for(i=0; i<MQTT_TOPIC_MAX_NUM; i++)
	{
		if((g_mqtt_handle.sub_topic_flag[i] != 0) && (strcmp(g_mqtt_handle.sub_topic_str[i], topic) == 0))
		{
			if((g_mqtt_handle.sub_topic_flag[i] == 3) && (g_mqtt_handle.mqtt_status == 1))
			{
				g_mqtt_handle.sub_topic_flag[i] = 4;
				mqtt_unsubscribe(&g_mqtt_handle.mqtt_broker, g_mqtt_handle.sub_topic_str[i], &g_mqtt_handle.sub_topic_msg_id[i] );
			}
			else
			{
				g_mqtt_handle.sub_topic_flag[i] = 0;
				g_mqtt_handle.sub_topic_msg_id[i] = 0;
				mqtt_send_callback(HFMQTT_TOPIC_UNSUB_SUCCESSED, g_mqtt_handle.sub_topic_str[i], NULL, 0);
			}
			return HF_SUCCESS;
		}
	}

	mqtt_set_errorno(HFMQTT_ERR_TOPIC_NOFIND);
	return -1;
}

int hfmqtt_pub(char *topic, unsigned char *data, int data_len)
{
	if(mqtt_init_flag != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_INIT);
		return -1;
	}
	
	if(strlen(topic) > MQTT_TOPIC_MAX_LEN)
	{
		mqtt_set_errorno(HFMQTT_ERR_TOPIC_OVERSIZE);
		return -1;
	}

	if(g_mqtt_handle.mqtt_status != 1)
	{
		mqtt_set_errorno(HFMQTT_ERR_NOT_CONNECTED);
		return -1;
	}

	extern int upload_len;
	upload_len = data_len;
	mqtt_publish(&g_mqtt_handle.mqtt_broker, (const char*)topic, (const char*)data, 0);
	return HF_SUCCESS;
}

int hfmqtt_ping(void)
{
	mqtt_ping(&g_mqtt_handle.mqtt_broker);
	return HF_SUCCESS;
}

int hfmqtt_close(void)
{
	mqtt_init_para();
	return HF_SUCCESS;
}


