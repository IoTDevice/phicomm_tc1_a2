
#include <stdio.h>
#if !(defined(__LPT230__)||defined(__LPT130__)||defined(__LPB130__)||defined(__LPT330__)||defined(__LPB135__))
#include <memory.h>
#endif
#include "jdsmart.h"
#include "jutils.h"
#include "packets.h"
#include "debugPro.h"
#include "nodeCache.h"
#include "Demo_Main.h"


#ifdef _WIN32
#include <winsock2.h>
#include <Mswsock.h>
#pragma comment(lib, "ws2_32.lib")
#define socklen_t int
#define msSleep(ms)	Sleep(ms)
#elif (defined(__LPT230__)||defined(__LPT130__)||defined(__LPB130__)||defined(__LPT330__)||defined(__LPB135__))
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include "hftime.h"
#include "json/cJSON.h"
#define SOCKET int
typedef struct sockaddr SOCKADDR;


//#define closesocket close
#define msSleep(ms) sleep(ms)
#define printf
#else
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#ifndef __ANDROID__
#include <ifaddrs.h>
#endif
#include <netdb.h>
#define SOCKET int
typedef struct sockaddr SOCKADDR;
#define closesocket close
#define msSleep(ms) sleep(ms)
#endif

#include "hftime.h"
#include "jdservice.h"
#include "jdota.h"
#include <hsf.h>
#include "HFService_config.h"
#include "joylink_auth_uECC.h"

#include "HFService_downstream.h"
jdNVargs_t jdArgs;


jdsmart_callback dev_callback[JD_CALLBACK_NUM];
uint32_t user_uart_data_len = 0;
struct sockaddr_in udp_sin_recv;
int udp_sin_recv_len;

static unsigned short local_port = 80;
int ret;
static jd_event_callback_t p_jd_event_callback=NULL;
uint8_t respBuffer[respBuffer_MAXLEN] = {0};
uint8_t protol_type=UDP_TRANSMIT;

char DeviceDetailV0[200];
char DeviceDetailV1[400];
static const char HeadV0[] = {
	0xaa, 0x55, 0x00, 0x00,
	0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x0d,
	0x02, 0x00, 0x00, 0x00,
	0x4f, 0x4b
};

int jd_app_ctrl_callback(int model, control_t *cmd, int cmdlen, unsigned char *rsp);
extern jl2_d_idt_t user_idt;

char SendBuffer[UDP_MTU] = { 0 };

int V0_ScanRet(void)
{
	uint8_t sum;
	int len, i;
	uint8_t* p;
	memcpy(SendBuffer, HeadV0, sizeof(HeadV0));
	SendBuffer[13] = 2;
	strcpy(SendBuffer + 19, DeviceDetailV0);
	len = 6 + strlen(DeviceDetailV0);
	sum = 0;
	p = (uint8_t*)&SendBuffer[13];
	for (i = 0; i < len; i++)
	{
		sum += *(p + i);
	}
	SendBuffer[4] = len;
	SendBuffer[12] = sum;

	return (len + 13);
}

int V1_ScanRet(void)
{
	int len = packetBuildV1((uint8_t* )SendBuffer, UDP_MTU, ET_NOTHING, PT_SCAN, (uint8_t*)1, (const uint8_t*)DeviceDetailV1, strlen(DeviceDetailV1));
	return len;
}

int V0_WriteRet(void)
{
	uint8_t sum;
	int len, i;
	uint8_t* p;
	char data[] = "{\"code\":0}";
	// 写数据
	memcpy(SendBuffer, HeadV0, sizeof(HeadV0));
	SendBuffer[13] = 4;
	strcpy(SendBuffer + 19, data);
	len = 6 + strlen(data);
	sum = 0;
	p = (uint8_t*)&SendBuffer[13];
	for (i = 0; i < len; i++)
	{
		sum += *(p + i);
	}
	SendBuffer[4] = len;
	SendBuffer[12] = sum;
	return (len + 13);
}


int V1_WriteRet(int code, char *msg)
{
	char data[100] = {0};
    sprintf(data, "{\"code\":%d, \"msg\":\"%s\"}", code, msg);
	int len = packetBuildV1((uint8_t* )SendBuffer, UDP_MTU, ET_NOTHING, PT_WRITE_ACCESSKEY, NULL, (const uint8_t*)data, strlen(data));
	return len;
}


SOCKET udpsocket = -1;
SOCKET serverSocket = -1;
int isConnected = 0;	// 0:Socket创建, 1:Auth阶段, 2:HB就绪
int hbLostCount = 0;


void set_sockblk(int sockfd, int block)
{
    int32_t flag = 0;
    flag = fcntl(sockfd, F_GETFL, 0);
  	if(flag < 0)
  	{
  		return ;
  	}
   #if 1
    if(block)
    {
        flag &= ~O_NONBLOCK;
    }
    else
    {
        flag |= O_NONBLOCK;
    }
   fcntl(sockfd, F_SETFL, flag);
   #endif
}

int Timer1GetTime(void)
{
	return hfsys_get_time();
}

int UdpCreate()
{
	SOCKET udpsocket;
	//int debug_cnt = 0;
	// Woonsocket's
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(local_port);
	udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpsocket == -1)
	{
		custom_log("socket() failed!");
		goto EXIT;
	}


#ifdef _WIN32
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	WSAIoctl(udpsocket, SIO_UDP_CONNRESET,
		&bNewBehavior, sizeof(bNewBehavior),
		NULL, 0, &dwBytesReturned,
		NULL, NULL);
#endif
	// Enable broadcast on socket
	int broadcastEnable = 1;
	if (setsockopt(udpsocket, SOL_SOCKET, SO_BROADCAST, (uint8_t *)&broadcastEnable, sizeof(broadcastEnable)) < 0)
	{
		//dbg(DL_ERROR, DM_SOCKET, "SO_BROADCAST ERR");
		custom_log("SO_BROADCAST ERR!");
	}

	ret = bind(udpsocket, (SOCKADDR*)&sin, sizeof(SOCKADDR));
	if (ret == -1)
	{
		//perror("绑定端口出错");
		custom_log("Bind Error");
		goto EXIT;
	}

	custom_log("udp create ok!");
	return udpsocket;

EXIT:
	close(udpsocket);
	return -1;
}

int tcpCreate(uint8_t *url,int port)
{
	int iSocketId;	
	int ret;
	struct sockaddr_in addr;
	struct sockaddr_in local_addr;
	char *addrp=(char *)url;

	fd_set rfds, wfds;
    struct timeval tv;
	
	if((memcmp(url, "HTTPS://", 8)==0)||(memcmp(url, "https://", 8)==0))
		addrp= (char *)(url+8);

	ip_addr_t dest_addr;
	addressis_ip((const char *)(addrp));
	hfnet_gethostbyname((const char *)(addrp), &dest_addr);
	inet_aton((char *)(addrp), (ip_addr_t *) &dest_addr);
	
	uint16_t local_port=(((Timer1GetTime()>>16)+(Timer1GetTime())&0xFFFF)&0x1FFF);
	local_port += 0x2FFF;

	memset((char*)&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr=dest_addr.addr;
	iSocketId = socket(AF_INET, SOCK_STREAM, 0);
	if(iSocketId<0) return -1;
	else	
	{
		memset((char *)&local_addr, 0, sizeof(local_addr));
		local_addr.sin_family = AF_INET;
		local_addr.sin_len = sizeof(local_addr);
		local_addr.sin_port = htons(local_port);
		local_addr.sin_addr.s_addr= htonl(INADDR_ANY);
		bind(iSocketId,(struct sockaddr *)&local_addr,sizeof(local_addr));

		set_sockblk(iSocketId, 0);
		ret=connect(iSocketId, (struct sockaddr *)&addr, sizeof(addr));
	 	if(0 == ret)
    	{
     	   return iSocketId;
    	}
    	else
		{
      	  if(errno == EINPROGRESS)
		  {
		   	 FD_ZERO(&rfds);
           	 FD_ZERO(&wfds);
             FD_SET(iSocketId, &rfds);
             FD_SET(iSocketId, &wfds);
             tv.tv_sec = 3;
             tv.tv_usec = 0;
		     ret = select(iSocketId + 1, &rfds, &wfds, NULL,&tv);//&tv
             switch(ret)
             {
             	case -1:
					close(iSocketId);  //new add by ZN
                	return -1;
                	//break;
            	case 0:
					close(iSocketId);  //new add by ZN
                	return -1;
                	//break;
            	default:
                	if(FD_ISSET(iSocketId, &rfds) ||
                   		FD_ISSET(iSocketId, &wfds))
              	    {
                 	   set_sockblk(iSocketId, 1);
                   	   return iSocketId;
                    }
           	  }
        	}
		  else
		  {
		  	 close(iSocketId);
             return  -1;
		  }
		
      }
	}  
}

int Server_Auth(void)
{
	auth_t auth = { 0 };
	auth.random_unm = 1;
	auth.timestamp = 10;
	int len = serverPacketBuild((uint8_t*)SendBuffer, UDP_MTU, PT_AUTH, (uint8_t* )jdArgs.accesskey, (uint8_t *)&auth, sizeof(auth));
	return len;
}

static unsigned short heartbeat_verion = 0xFFFF;
int Server_HB(void)
{
	if(heartbeat_verion == 0xFFFF)//get version number from UFLASH
	{
		HFOtaConfig data;
		joylink_get_ota_config(&data);
		heartbeat_verion = (unsigned short)data.version_num;
	}
	
	heartbeat_t heartbeat = { 0 };
	heartbeat.rssi = 1;
	heartbeat.verion = heartbeat_verion;
	heartbeat.timestamp = 10;
	int len = serverPacketBuild((uint8_t*)SendBuffer, UDP_MTU, PT_BEAT, jdDev.sessionKey, (uint8_t *)&heartbeat, sizeof(heartbeat_t));
	return len;
}

int Server_Data(void)
{
	dataupload_t data = { 0 };
	data.timestamp = 10;
	int len = serverPacketBuild((uint8_t*)SendBuffer, UDP_MTU, PT_UPLOAD, jdDev.sessionKey, (uint8_t *)&data, sizeof(data));
	return len;
}

int ServerState()
{	
	int interval = 5000;
	if (hbLostCount > 10)
	{
		if (isConnected == PHASE_READY)
		{
			custom_log("Server HB Lost, Retry Auth!\r\n");
			hbLostCount = 0;
			isConnected = PHASE_AUTHENTICATE;
		}

		if (isConnected == PHASE_AUTHENTICATE)
		{
			custom_log("Auth ERR, Reconnect!");
			hbLostCount = 0;
			isConnected = PHASE_ESTABLISH;
			close(serverSocket);

			jdservice_disconnected();
			serverSocket = -1;
		}
	}

	 if(strlen(jdArgs.server) > 7) //creat tcp until get server ip 
	 {
		switch (isConnected)    // 0:Socket创建, 1:Auth阶段, 2:HB就绪
		{
		 
		case PHASE_ESTABLISH:
		{
			  set_creat_tcp_flags(NEED_CREAT_SOCKET);
			  //serverSocket=tcpCreate((uint8_t *)jdArgs.server, jdArgs.port);
			  interval = 5000;
			  hbLostCount = 0;
			  break;
		}
		case PHASE_AUTHENTICATE:
		{
			   int len = Server_Auth();
			   send(serverSocket, SendBuffer, len, 0);
			   custom_log("Auth----->");
			   if (hbLostCount==0)
				   interval = 500;
			   interval = 500;
			   hbLostCount++;
		}break;
		case PHASE_READY:
		{
		   int len = Server_HB();
		   send(serverSocket, SendBuffer, len, 0);
		   custom_log("HB----->");
		   hbLostCount++;
		   interval = 15000;

		  // len = Server_Data();
		   //send(serverSocket, SendBuffer, len, 0);
		}
		break;
		default:
			break;
	}
	}
	return interval;
}

uint8_t recBuffer[UDP_MTU] = { 0 };
uint8_t tcpRecBuffer[UDP_MTU] = { 0 };

uint8_t recPainText[UDP_MTU] = { 0 };
/*
Select 阻塞方式
*/
int joylink_dev_get_random()
{
	/**
	 *FIXME:must to do
	 */
	return rand();
}
int joylink_util_byte2hexstr(const uint8_t *pbytes, int blen, uint8_t *o_phex, int hlen)
{
	const char tab[] = "0123456789abcdef";
	int i = 0;

	memset(o_phex, 0, hlen);
	if(hlen < blen * 2){
		blen = (hlen - 1) / 2;
    }

	for(i = 0; i < blen; i++){
		*o_phex++ = tab[*pbytes >> 4];
		*o_phex++ = tab[*pbytes & 0x0f];
		pbytes++;
	}
	*o_phex++ = 0;

	return blen * 2;
}


void initDeviceDetailV1(uint8_t *src)
{
	 unsigned char retCode[32]="scan ok";
	 DevScan_t scan;
	 int ret_sign = 0;
	 uint8_t prikey_buf[65] = {0};
     uint8_t sign_buf[65] = {0};
     uint8_t rand_buf[33] = {0};
	
	 
	 joylink_parse_scan(&scan, (const char *)src);
	 joylink_util_hexStr2bytes(jdDev.prikey, prikey_buf, 32);
    
	 ret_sign = jl3_uECC_sign(prikey_buf,(uint8_t *)scan.app_rand, strlen(scan.app_rand),sign_buf,uECC_secp256r1());
	 if(ret_sign == 1)
	 {
		custom_log("gen devsigature to cloud random success");
	 }
	 else
	 {
		custom_log("gen dev sigature to cloud random error");
		return;
	 }
	 joylink_util_byte2hexstr((const uint8_t *)sign_buf, 64, (uint8_t *)scan.dev_sign, 64*2);
	 if(!strcmp(jdDev.rand, ""))
	 {
		int ran[8];
		int i;
		ran[0] = joylink_dev_get_random();
		
		ran[1]=hfsys_get_time();
		for(i = 2; i < 8; i++)
		{
		   ran[i] = ran[i-1] ^ ran[i-0]; 
		}
		joylink_util_byte2hexstr((const uint8_t *)&ran[0], sizeof(ran), (uint8_t *)(&jdDev.rand[0]), sizeof(jdDev.rand) - 1);
	 }
	
	 if(!jdArgs.is_actived)
	 {
		 if(strlen(jdArgs.feedid) <= 1)
		 {
		    sprintf(DeviceDetailV1,"{\"code\":\"%s\",\"msg\":0,\"mac\":\"%s\",\"noSnapShort\":1,\"productuuid\":\"%s\",\"feedid\":\"\",\"devkey\":\"%s\",\"lancon\":1,\"trantype\":1,\"devtype\":0,\"d_idt\":{\"d_r\":\"%s\",\"d_s\":\"%s\"}}",
            retCode,
			jdDev.mac,
			jdDev.uuid,
			//jdArgs.feedid,
			jdDev.pubkeyS,
			jdDev.rand,
			scan.dev_sign
			);
		 }
		 else
		 {
			sprintf(DeviceDetailV1,"{\"code\":\"%s\",\"msg\":0,\"mac\":\"%s\",\"noSnapShort\":1,\"productuuid\":\"%s\",\"feedid\":\"%s\",\"devkey\":\"%s\",\"lancon\":1,\"trantype\":1,\"devtype\":0,\"d_idt\":{\"d_r\":\"%s\",\"d_s\":\"%s\"}}",
            retCode,
			jdDev.mac,
			jdDev.uuid,
			jdArgs.feedid,
			jdDev.pubkeyS,
			jdDev.rand,
			scan.dev_sign
			);
		 }
	 }
	 else
	 {
		sprintf(DeviceDetailV1,"{\"code\":\"%s\",\"msg\":0,\"mac\":\"%s\",\"productuuid\":\"%s\",\"feedid\":\"%s\",\"devkey\":\"%s\",\"lancon\":1,\"trantype\":1,\"devtype\":0}",
            retCode,
			jdDev.mac,
			jdDev.uuid,
			jdArgs.feedid,
			jdDev.pubkeyS
			);
	 }
	 custom_log("DeviceDetailV1=%s",DeviceDetailV1);
}

int jd_send_event(uint32_t event_id,void *param)
{
	jd_event_callback_t p_callback = p_jd_event_callback;
	
	if(p_callback!=NULL)
		return p_callback(event_id,param);

	return HF_SUCCESS;
}

void joylink_start_fun(void)
{	
	struct timeval  selectTimeOut;
	static int interval = 0;
	selectTimeOut.tv_usec = 0L;
	selectTimeOut.tv_sec = (long)1; // 1秒后重发
	static uint32_t serverTimer;
	timerReset(&serverTimer);

	while (1)
	{
		if(udpsocket == -1)
		{
			udpsocket =  UdpCreate();
		}
		if (isTimeOut(serverTimer, interval))
		{
			timerReset(&serverTimer);
			interval = ServerState();    
		}
		fd_set  readfds = {0};
		FD_ZERO(&readfds);
		FD_SET(udpsocket, &readfds);
		if (serverSocket != -1 && isConnected > 0)
		{
			FD_SET(serverSocket, &readfds);
		}
		int maxFd = (int)serverSocket > (int)udpsocket ? serverSocket : udpsocket;
		selectTimeOut.tv_usec = 0L;
        selectTimeOut.tv_sec = (long)1;
		ret = select(maxFd + 1, &readfds, NULL, NULL, &selectTimeOut);
		if(ret < 0)
		{
			custom_log("Select ERROR\r\n");
			continue;
		}
		if(ret == 0)
		{
			continue;
		}
		else
		{
			struct sockaddr_in sin_recv;
			int addrlen = sizeof(SOCKADDR);
			if (FD_ISSET(udpsocket, &readfds) ) // UDP 有东西就读
			{
				ret = recvfrom(udpsocket, recBuffer, sizeof(recBuffer), 0, (SOCKADDR*)&sin_recv, (u32_t *)&addrlen);
				if (ret == -1)
				{
					custom_log("UDP recvfrom:[%d]", ret);
					close(udpsocket);
					continue;
				}

			char ipStr[50];
			utilGetIPString(&sin_recv, ipStr);

			packetparam_t param;
			memset(recPainText,0,sizeof(recPainText));
			ret = packetAnalyse(&param, recBuffer, ret, recPainText, UDP_MTU);
			if (ret <= 0)
			{
				custom_log("UDP packetAnalyse:[%d]", ret);
				continue;
			}
			switch (param.type)
			{
			case PT_SCAN:
				if (param.version == 1)  
				{
					u_printf("IP:%s (Scan->Type:%d, Version:%d)\r\n", ipStr, param.type, param.version);
					initDeviceDetailV1(recPainText);
					int len = V1_ScanRet();
					ret = sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
					if(ret<=0)
					{
						sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
					}
				}
				break;

			case PT_WRITE_ACCESSKEY:
				custom_log("PT_WRITE_ACCESSKEY");
				if(!searchEnable())
					break;
				if (param.version == 1)
				{
					int len=write_accesskey((char *)recPainText);
					if(len > 0 && len < JL_MAX_PACKET_LEN)
					{ 
						ret = sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
					    if(ret < 0)
						{	
						   u_printf("[%s]:[%d] send error ret:%d\r\n",__FUNCTION__,__LINE__,ret);
						}
						ret = sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
					    if(ret < 0)
						{	
						   u_printf("[%s]:[%d] send error ret:%d\r\n",__FUNCTION__,__LINE__,ret);
						}
						ret = sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
					    if(ret < 0)
						{	
						   u_printf("[%s]:[%d] send error ret:%d\r\n",__FUNCTION__,__LINE__,ret);
						}
					}
					else
					{
					   u_printf("[%s]:[%d] packet error ret:%d\r\n",__FUNCTION__,__LINE__,ret);
				    }
				}
				break;

			case PT_JSONCONTROL:
				if (param.version == 1 && joylink_is_usr_timestamp_ok(ipStr,*((int*)(recPainText))))
				{
					char data[] = "{\"code\":0,\"data\":[{\"stream_id\":\"switch\",\"current_value\":1},{\"stream_id\":\"light\",\"current_value\":\"on\"}]}";
					int len = packetBuildV1((uint8_t*)SendBuffer, UDP_MTU, ET_ACCESSKEYAES, PT_JSONCONTROL, (uint8_t* )jdArgs.accesskey, (const uint8_t* )data, strlen(data));
					sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
					custom_log("Control->%s\r\n", recPainText);
				}
				break;

			case PT_SCRIPTCONTROL:
				custom_log("SCRIPT Control serial->%d", *((int*)(recPainText + 8)));
                int32_t biz_code = (int)(*((int *)(recPainText  + 4)));
				if(biz_code == JOYLINK_CTRL_CODE_SET || biz_code == JOYLINK_CTRL_CODE_GET)
				{
					if (param.version == 1 && joylink_is_usr_timestamp_ok(ipStr,*((int*)(recPainText))))
					{
						#if 1
						protol_type=UDP_TRANSMIT;
						custom_log("SCRIPT Control->%s", recPainText + 12);
						memcpy(&udp_sin_recv,&sin_recv,sizeof(struct sockaddr_in));
						udp_sin_recv_len=addrlen;
						control_t* p = (control_t*)recPainText;
						custom_log("------------Local recv cmd [%d]", p->biz_code);
						unsigned char rsp[150];
						if(jd_app_ctrl_callback(0, p, ret-respHeadLen, rsp) > 0);
						{	
							int len = packetBuildV1((uint8_t *)SendBuffer, UDP_MTU, ET_ACCESSKEYAES, PT_JSONCONTROL, (uint8_t*)jdArgs.localkey, respBuffer, user_uart_data_len+respHeadLen);
							sendto(udpsocket, SendBuffer, len, 0, (SOCKADDR*)&sin_recv, addrlen);
						}
						#endif
					}
				}
				break;

			default:
				custom_log("UDP default");
				break;
			}//switch (param.type)
		}//if (FD_ISSET(udpsocket, &readfds)) // 有东西就读

			else if ((serverSocket != -1)&&(FD_ISSET(serverSocket, &readfds))) // 服务器已经连接,而且有数据
			{
				ret = recv(serverSocket, tcpRecBuffer, 1024, 0);
				if (ret <= 0)
				{
					//int id = WSAGetLastError();

				close(serverSocket);
				serverSocket = -1;
				isConnected = 0;
				custom_log("TCP Server close, Reconnect!\r\n");
				jdservice_disconnected();
				continue;
			}
			u_printf("ret=%d\r\n",ret);
			tcpRecBuffer[ret]=0;
			packetparam_t param;
			ret = serverAnalyse(&param, tcpRecBuffer, ret, recPainText, UDP_MTU);
			if (ret < 1)
			{
				custom_log("TCP packetAnalyse:[%d]", ret);
				continue;
			}
			switch (param.type)
			{
				case PT_AUTH:
					{
						auth_resp_t* p = (auth_resp_t*)recPainText;
						memcpy(jdDev.sessionKey, p->session_key, 32);

						time_t t = p->timestamp;
						custom_log("OK===>Rand=%u,Time:%s", p->random_unm, ctime(&t));
						isConnected = 2;

						struct timeval tv;
						tv.tv_usec=0;
						tv.tv_sec= p->timestamp;
						settimeofday(&tv, NULL);
						jdservice_connected();
					}
					break;
				
				case PT_BEAT:
				{
					heartbeat_resp_t* p = (heartbeat_resp_t*)recPainText;
					hbLostCount = 0;
					time_t t = p->timestamp;
					jdArgs.cloud_timestamp = p->timestamp;
					custom_log("OK===>Code=%u,Time:%s", p->code, ctime(&t));
				}
					break;
				
				case PT_SERVERCONTROL:
				{
					control_t* p = (control_t*)recPainText;
					protol_type=TCP_TRANSMIT;
					custom_log("------------Server recv cmd [%d]", p->biz_code);
					unsigned char rsp[150];
					if(jd_app_ctrl_callback(0, p, ret-respHeadLen, rsp) > 0);
					{	
						byte2hexstr(p->cmd, ret-respHeadLen, (uint8_t*)SendBuffer, UDP_MTU);
						int len = serverPacketBuild((uint8_t*)SendBuffer, UDP_MTU, PT_SERVERCONTROL, jdDev.sessionKey, (const uint8_t*)respBuffer, user_uart_data_len+respHeadLen);
						int send_tcp_len=send(serverSocket, SendBuffer, len, 0);
						u_printf("send_tcp_len=%d\r\n",send_tcp_len);
					}
				}
				    break;

				case PT_OTA_ORDER:
				{
					custom_log("Joylink server ota cmd");
					joylink_proc_server_ota_order((recPainText + 4), (ret - 4));
				}
					break;
					
				case PT_OTA_UPLOAD:
				{
					custom_log("Joylink server ota upload response.");
					joylink_proc_server_ota_upload((recPainText + 4), (ret - 4));
				}
					break;
					
				default:
					custom_log("Other Data");
					break;
			}
		}//if (FD_ISSET(serverSocket, &readfds)) 

			joylink_ota_check_process();
		}
	}//while (1)
}

extern uint8_t dhcp_ok; 
void jdsmart_hander_set_device_status_by_rawdata(CMD_SRC cmd_src, CMD_TYPE cmd_type, uint8_t *cmd, int cmd_len)
{
	enum CLOUD_DATA_TYPE type = HFSERV_CMD_REMOTE_CTRL;
	if(cmd_type == COMMON_CTL)
	{
		if(cmd_src == LOCAL_CTL)
		{
			type = HFSERV_CMD_LOCAL_CTRL;
		}
		else if(cmd_src == REMOTE_CTL)
		{
			type = HFSERV_CMD_REMOTE_CTRL;
		}
	}
	else if(cmd_type == CLDMENU_CTL)
	{
		type = HFSERV_CMD_MENU_CTRL;
	}
	
	hfservice_cloud_data_recv(type, (unsigned char *)cmd,&cmd_len);
}

int jd_thread(void)
{
	while(dhcp_ok== 0)
	{
		custom_log("Network is down,waiting.......");
		msleep(2000);
		continue;
	}
	dev_callback[JD_SET_DEVICE_STATUS_BY_RAWDATA] = (jdsmart_callback)jdsmart_hander_set_device_status_by_rawdata;
	eccContexInit();
	joylink_start_fun();
	hfthread_destroy(NULL);
	return 0;
}

int creat_tcp_thread(void)
{
	int ret;
	int creat_tcp=0;
	int init_timeout=500;
	while(1)
	{	
	    creat_tcp=get_creat_tcp_flags();
		if(creat_tcp == NEED_CREAT_SOCKET)
		{
			custom_log("creat socket");
			serverSocket=tcpCreate((uint8_t *)jdArgs.server, jdArgs.port);
			if (serverSocket == -1)
			{
			  custom_log("Server connect failed!");
			  isConnected = 0;
			}
			else
			{
			   set_creat_tcp_flags(NEEDON_CREAT_SOCKET);
			   custom_log("Server connect ok!");
			   isConnected = 1;
			}	
		}
		msleep(500);
	}
}



int USER_FUNC jd_main(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)jd_thread, "JD_THREAD", 1024*4, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		custom_log("JD thread create failed!\r\n");
	}	
	if(hfthread_create((PHFTHREAD_START_ROUTINE)creat_tcp_thread, "JD_THREAD", 1024, NULL, HFTHREAD_PRIORITIES_LOW,NULL,NULL)!= HF_SUCCESS)
	{
		custom_log("JD thread create failed!\r\n");
	}	
	return 1;
}


//User API, add by WJT
int jd_app_ctrl_callback(int model, control_t *cmd, int cmdlen, unsigned char *rsp)//model:1-server
{
	control_t *ctrl_rsp=(control_t *)respBuffer;
	ctrl_rsp->timestamp = cmd->timestamp + 5;
	ctrl_rsp->serial = cmd->serial;
	
	if (cmd->biz_code == 1002)
	{
		ctrl_rsp->biz_code = 102;
		if(dev_callback[JD_SET_DEVICE_STATUS_BY_RAWDATA] == NULL)
			custom_log("dev_callback== null");
	    else
	    {
		   if(model == LOCAL_CTL)
			   dev_callback[JD_SET_DEVICE_STATUS_BY_RAWDATA](LOCAL_CTL, COMMON_CTL, (uint8_t*)(cmd->cmd), cmdlen);//lan 控制
		   else if(model == REMOTE_CTL)
		   	   dev_callback[JD_SET_DEVICE_STATUS_BY_RAWDATA](REMOTE_CTL, CLDMENU_CTL, (uint8_t*)(cmd->cmd), cmdlen); //wann控制
	    }
        return 1;
	}
	else if (cmd->biz_code == 1004)
	{
		ctrl_rsp->biz_code = 104;	
		return 1;
	}
	else if (cmd->biz_code == 1050)
	{
		ctrl_rsp->biz_code = 150;
		return 1;
	}

	return 0;
}

int jdservice_upload_data(char *senddata, int datalen)
{
	//only upload to server
	uint8_t respBuffer_tmp[respBuffer_MAXLEN] = {0};
	dataupload_t *ctrl_rsp=(dataupload_t *)respBuffer_tmp;
	//int send_len = 0;

	ctrl_rsp->timestamp = 10;
	memcpy(ctrl_rsp->data, senddata, datalen);
	
	int pack_len = serverPacketBuild((uint8_t*)SendBuffer, UDP_MTU, PT_UPLOAD, jdDev.sessionKey, (const uint8_t*)ctrl_rsp,sizeof(unsigned int)+datalen);
	return send(serverSocket, SendBuffer, pack_len, 0);	
}

int jdservice_servercontrol_data(char * data,int len)
{
    protol_type=AUTO_TRANSMIT;
	int pack_len = serverPacketBuild((uint8_t*)SendBuffer, UDP_MTU, PT_SERVERCONTROL, jdDev.sessionKey, (const uint8_t*)respBuffer,respHeadLen+len);
	return send(serverSocket, SendBuffer, pack_len, 0);	
}
int jdservice_lann_control_data(char * data,int len)
{
    protol_type=AUTO_TRANSMIT;
    int pack_len = packetBuildV1((uint8_t *)SendBuffer, UDP_MTU, ET_ACCESSKEYAES, PT_JSONCONTROL, (uint8_t*)jdArgs.localkey, respBuffer, user_uart_data_len+respHeadLen);
	return sendto(udpsocket, SendBuffer, pack_len, 0, (SOCKADDR*)&udp_sin_recv, udp_sin_recv_len);
}

void jd_post_device_rawdata(uint8_t* data, int data_len,RAWDATA_T type) 
{   
    user_uart_data_len = data_len;
	
	if(user_uart_data_len > respBuffer_MAXLEN-respHeadLen)
	{
		custom_log("\r\nError:user uart data buff overflow, max:%d\r\n", respBuffer_MAXLEN-respHeadLen);
		user_uart_data_len = respBuffer_MAXLEN-respHeadLen;
	}
	else if(user_uart_data_len <= 0)
	{
		user_uart_data_len = 0;
		return;
	}
	memcpy(respBuffer+respHeadLen, data, user_uart_data_len);
	
	switch(type)
	{
		case CTL_RESP:	
			if(protol_type == UDP_TRANSMIT)
				jdservice_lann_control_data((char *)data,data_len);
			if(protol_type == TCP_TRANSMIT)
				jdservice_servercontrol_data((char *)data,data_len);
			break;
		case AUTO_UPDATE:
			//only upload to server
			jdservice_upload_data((char *)data,user_uart_data_len);
			break;

		default:
			break;
	}

}
void hfservice_cloud_data_send(enum CLOUD_DATA_TYPE type, unsigned char *data, int len)
{
    if(dhcp_ok != 1)
		return;
	
	if(type == HFSERV_RSP_LOCAL_CTRL)
	{
		jd_post_device_rawdata((uint8_t* )data, len, CTL_RESP);
		jd_post_device_rawdata((uint8_t* )data, len, AUTO_UPDATE);
	}
	else if(type == HFSERV_RSP_REMOTE_CTRL)
	{
		jd_post_device_rawdata((uint8_t* )data, len, CTL_RESP);
	}
	else if(type == HFSERV_UPDATE_DATA)
	{
		jd_post_device_rawdata((uint8_t* )data, len, AUTO_UPDATE);
	}
}

static int jd_event_callback_func(JD_EVENT event_id,void * param)
{
	if(event_id == JD_CLO_CONNECTED)
	{
		//jd_connect_flag = 1;
		hfservice_cloud_data_recv(HFSERV_EVENT_CLOUD_CONNECT, NULL, 0);
	}
	else if(event_id == JD_CLO_DISCONNECTED)
	{	
		//jd_connect_flag = 0;
		hfservice_cloud_data_recv(HFSERV_EVENT_CLOUD_DISCONNECT, NULL, 0);
	}
	else if(event_id == JD_CLO_UPLOAD_RSP)
	{	
		hfservice_cloud_data_recv(HFSERV_UPDATE_RSP, NULL, 0);
	}

	return 0;
}

void jd_register_system_event(jd_event_callback_t p_callback)
{
	p_jd_event_callback = p_callback;
	return ;
}

void USER_FUNC hfservice_cloud_start(void)
{
    jd_register_system_event(jd_event_callback_func);
	jd_main();
}

