#include "ali_iot_mqtt.h"
#include "hsf.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include "MQTTHF_LPx30.h"
#include "hfwifi.h"
#include "time.h"
#include "ali_iot_mqtt.h"

#define Ali_MQTTCliBufLen 1024
#define Ali_MQTTCliReadbufLen   1024
#define Ali_MQTTCli_Command_Timeout_MS 		1000
#define Ali_MQTTCli_KeepAliveInterval_S 	60

ALI_MQTT_CONNECT_PARA AliMqttConnectPara;

static char ali_mqtt_server_addr[128]={0};

static char mac_str[12+1]={0};

static Client Ali_MQTTCli;

/*publish topic*/
char mqtt_pub_topic[256]={0};

/*subscribe topic*/
static char mqtt_sub_topic[256]={0};

int cstr2hex( char *str, unsigned char *hex )
{
	int len = 0;
	int val;

	while( *str ){
		if( *str >= '0' && *str <= '9'  )
			val = *str - '0';
		else if( *str >= 'a' && *str <= 'f' )
			val = *str - 'a' + 10;
		else if( *str >= 'A' && *str <= 'F' )
			val = *str - 'A' + 10;
		else
			return len;
		hex[ len ] += val << 4;
		str++;
		if( *str >= '0' && *str <= '9'  )
			val = *str - '0';
		else if( *str >= 'a' && *str <= 'f' )
			val = *str - 'a' + 10;
		else if( *str >= 'A' && *str <= 'F' )
			val = *str - 'A' + 10;
		else
			return len;		
		hex[ len ] += val;
		str++;
		len++;
	}
	
	return len;
}

void chex2str( unsigned char *hex, int len, char *str )
{
#define VAL2HEX( a )	( ( a ) < 10 ? ( ( a ) + '0' ) : ( ( a ) - 10 + 'A' ) )
	int idx;

	for( idx = 0; idx < len; idx++ ){	
		str[ idx * 2 ] = VAL2HEX( ( hex[ idx ] & 0xF0 ) >> 4 );
		str[ idx * 2 + 1 ] = VAL2HEX( hex[ idx ] & 0x0F );
	}
}

//阿里MQTT TOPIC初始化
static void ali_mqtt_topic_init(void)
{
    memset(mqtt_pub_topic,0,sizeof(mqtt_pub_topic));
    sprintf(mqtt_pub_topic,"/%s/%s/%s",IOT_PRODUCTKEY,DEVICENAME,AliMqttConnectPara.device_pubtopic);

    memset(mqtt_sub_topic,0,sizeof(mqtt_sub_topic));
    sprintf(mqtt_sub_topic,"/%s/%s/%s",IOT_PRODUCTKEY,DEVICENAME,AliMqttConnectPara.device_subtopic);
}

//阿里MQTT服务器地址初始化
static void ali_mqtt_server_addr_init(void)
{
    memset(ali_mqtt_server_addr,0,sizeof(ali_mqtt_server_addr));
    sprintf(ali_mqtt_server_addr,"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com",IOT_PRODUCTKEY);
    u_printf("ali_mqtt_server_addr:%s port:%d\r\n",ALI_MQTT_SERVER_ADDR,ALI_MQTT_SERVER_PORT);
}

//阿里MQTT 数据发送接口
int ali_mqtt_message_publish(char *topic,const char *data,int len,enum QoS qos)
{   
	MQTTMessage message;
	message.qos = qos;  
	message.retained = FALSE;    
	message.dup = FALSE;

	message.payload = (void*)data;    
	message.payloadlen = len;
	return MQTTPublish(&Ali_MQTTCli, topic, &message);
}

//阿里IOT套件网络数据回调
static void ali_iot_mqtt_message_callback(MessageData* md)
{   
	MQTTMessage* message = md->message;

	u_printf("************ Receive Message ***********\r\n");
	u_printf("%.*s\r\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	u_printf("%.*s\r\n", (int)message->payloadlen, (char*)message->payload);
    hfuart_send(HFUART0,(char*)message->payload,(int)message->payloadlen,0);
}

//阿里MQTT连接参数初始化
static int Ali_MQTT_connect_para_init(char *ali_mqtt_connect_clientId,char *ali_mqtt_connect_userName,char *ali_mqtt_connect_passWord_hexstr)
{
    int ret=-1;

    sprintf(ali_mqtt_connect_clientId,"%s|securemode=%d,signmethod=%s|",\
        mac_str,TCP_DIRECT_CONNECT_MODE,SIGNMETHOD);
    sprintf(ali_mqtt_connect_userName,"%s&%s",DEVICENAME,IOT_PRODUCTKEY);
    
    char content[256]={0};
    sprintf(content,"clientId%sdeviceName%sproductKey%s",\
        mac_str,DEVICENAME,IOT_PRODUCTKEY);

    unsigned char ali_mqtt_connect_passWord_byte[20+1];
    hmac_sha1((unsigned char *)HMACSHA1_KEY, HMACSHA1_KEY_LEN, (unsigned char *)content, strlen(content), (unsigned char *)ali_mqtt_connect_passWord_byte);
    chex2str(ali_mqtt_connect_passWord_byte,20,ali_mqtt_connect_passWord_hexstr);
    ret=0;
    
    return ret;
}

//阿里MQTT线程
static void Ali_MQTTClient_thread(void *arg)
{   
    int ret;
    int ping_time=0;
    char ali_mqtt_connect_clientId[128];
    char ali_mqtt_connect_userName[128];
    char ali_mqtt_connect_passWord_hexstr[64];
    MQTTPacket_connectData MQTTConnectData = MQTTPacket_connectData_initializer; 
    Network cliNetwork;
    NewNetwork(&cliNetwork);

    char *MQTTCliBuf;
    MQTTCliBuf = (char *)hfmem_malloc(Ali_MQTTCliBufLen);
    char *MQTTCliReadbuf;
    MQTTCliReadbuf = (char *)hfmem_malloc(Ali_MQTTCliReadbufLen);
    
    while(1)
    {
        u_printf("Wait WiFi link ...\r\n");
        while(!hfwifi_sta_is_connected())
        {
            msleep(1000);
        }
        u_printf("WiFi connected.\r\n");

        if(SUCCESS != ConnectNetwork(&cliNetwork, ALI_MQTT_SERVER_ADDR, ALI_MQTT_SERVER_PORT))
        {
            u_printf("TCPSocket connect fail!\r\n");
            goto MQTT_END;
        }
        u_printf("TCPSocket connect success.\r\n");

        memset(ali_mqtt_connect_clientId,0,sizeof(ali_mqtt_connect_clientId));
        memset(ali_mqtt_connect_userName,0,sizeof(ali_mqtt_connect_userName));
        memset(ali_mqtt_connect_passWord_hexstr,0,sizeof(ali_mqtt_connect_passWord_hexstr));
        Ali_MQTT_connect_para_init(ali_mqtt_connect_clientId,ali_mqtt_connect_userName,ali_mqtt_connect_passWord_hexstr);
        
        MQTTClient(&Ali_MQTTCli, &cliNetwork, Ali_MQTTCli_Command_Timeout_MS, (unsigned char *)MQTTCliBuf, Ali_MQTTCliBufLen, (unsigned char *)MQTTCliReadbuf, Ali_MQTTCliReadbufLen);
        MQTTConnectData.willFlag = 0;
        MQTTConnectData.MQTTVersion = 3;
        MQTTConnectData.clientID.cstring = ali_mqtt_connect_clientId;
        MQTTConnectData.username.cstring = ali_mqtt_connect_userName;
        MQTTConnectData.password.cstring = ali_mqtt_connect_passWord_hexstr;
        MQTTConnectData.keepAliveInterval = Ali_MQTTCli_KeepAliveInterval_S;
        MQTTConnectData.cleansession = 1;
        u_printf("MQTTConnectData.clientID:%s\r\n",MQTTConnectData.clientID.cstring);
        u_printf("MQTTConnectData.username:%s\r\n",MQTTConnectData.username.cstring);
        u_printf("MQTTConnectData.password:%s\r\n",MQTTConnectData.password.cstring);
        
        if(SUCCESS != MQTTConnect(&Ali_MQTTCli, &MQTTConnectData))
        {
            u_printf("MQTTConnect fail!\r\n");
            goto MQTT_END;
        }
        u_printf("MQTTConnect success.\r\n");

        
        if(SUCCESS == MQTTSubscribe(&Ali_MQTTCli, mqtt_sub_topic, QOS1, ali_iot_mqtt_message_callback))
        {
            u_printf("Subscribe[%s] success.\r\n", mqtt_sub_topic);
        }
        else
            u_printf("Subscribe[%s] fail!\r\n", mqtt_sub_topic);

        while(1)
        {
			ret = MQTTYield(&Ali_MQTTCli, 300);
			if(ret == FAILURE)
			{
				goto MQTT_END;
			}

			if(!hfwifi_sta_is_connected())
			{
				u_printf("MQTTClient_thread WiFi disconnected!\r\n");
				goto MQTT_END;
			}
			
			//应用层实现ping功能 防止 mqtt异常断开
			if(hfsys_get_time() - ping_time > (MQTTConnectData.keepAliveInterval-10)*1000)
			{
				ping_time = hfsys_get_time();
				int len = MQTTSerialize_pingreq(Ali_MQTTCli.buf, Ali_MQTTCli.buf_size);
				if(len > 0)
				{
					Ali_MQTTCli.ipstack->mqttwrite(Ali_MQTTCli.ipstack, Ali_MQTTCli.buf, len, 100);
				}
			}
		}
        
MQTT_END:   
        u_printf("MQTT end!\r\n\r\n");
        cliNetwork.disconnect(&cliNetwork);
        msleep(3000);
    }   

    if(MQTTCliBuf != NULL)
        hfmem_free(MQTTCliBuf);
    if(MQTTCliReadbuf != NULL)
        hfmem_free(MQTTCliReadbuf);

    u_printf("MQTTClient_thread destroy.\r\n");
    msleep(1000);
    hfthread_destroy(NULL);
}

void get_clientid(void)
{
    hfnet_get_mac_address(mac_str);
    u_printf("get_clientid:%s\r\n",mac_str);
}

void get_ali_mqtt_connect_para(void)
{
    hfuflash_read(ALI_MQTT_CONNECT_PARA_UFLASH_ADDR,(char *)(&AliMqttConnectPara),sizeof(ALI_MQTT_CONNECT_PARA));
    if((AliMqttConnectPara.head!=ALI_MQTT_CONNECT_PARA_HEAD)||(AliMqttConnectPara.tail!=ALI_MQTT_CONNECT_PARA_TAIL))
    {
        memset((char *)(&AliMqttConnectPara),0,sizeof(ALI_MQTT_CONNECT_PARA));
        hfuflash_erase_page(ALI_MQTT_CONNECT_PARA_UFLASH_ADDR,1);
        u_printf("default configuration:>>>\r\n");
        AliMqttConnectPara.head=ALI_MQTT_CONNECT_PARA_HEAD;
        AliMqttConnectPara.tail=ALI_MQTT_CONNECT_PARA_TAIL;
        strcpy(AliMqttConnectPara.ProductKey,"a1yZ3q68cvD");
        strcpy(AliMqttConnectPara.DeviceName,"edI2nVxcnvA3GxJY7leK");
        strcpy(AliMqttConnectPara.DeviceSecret,"6EeevPsDyIFCKRSvMtjTKAcWuyPUdfgk");
        strcpy(AliMqttConnectPara.device_pubtopic,"update");
        strcpy(AliMqttConnectPara.device_subtopic,"get");
        hfuflash_write(ALI_MQTT_CONNECT_PARA_UFLASH_ADDR,(char *)(&AliMqttConnectPara),sizeof(ALI_MQTT_CONNECT_PARA));
    }
    
    u_printf("ProductKey:%s\r\nDeviceName:%s\r\nDeviceSecret:%s\r\ndevice_pubtopic:%s\r\ndevice_subtopic:%s\r\n",\
        AliMqttConnectPara.ProductKey,AliMqttConnectPara.DeviceName,AliMqttConnectPara.DeviceSecret,\
        AliMqttConnectPara.device_pubtopic,AliMqttConnectPara.device_subtopic);
}

int Ali_MQTT_thread_start(void)
{
    int ret=-1;
    static bool start_flag=false;
    if(start_flag)
    {
        u_printf("Ali_MQTTClient_thread can't be created repeat.");
        return ret;
    }
    
    get_clientid();

    get_ali_mqtt_connect_para();
    
    ali_mqtt_topic_init();

    ali_mqtt_server_addr_init();
    
    if(hfthread_create(Ali_MQTTClient_thread, "Ali_MQTTClient_thread", 512, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL)!= HF_SUCCESS)
	{
		u_printf("start Ali_MQTTClient_thread fail\r\n");
        ret=-1;
	}
    ret=0;
    start_flag=true;
    return ret;
}


