/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *-pipe -fno-strict-aliasing -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror-implicit-function-declaration -Wpointer-arith -std=gnu99 -ffunction-sections -fdata-sections -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wbad-function-cast -Wwrite-strings -Wsign-compare -Waggregate-return  -Wmissing-declarations -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs -Wlong-long -Wunreachable-code -Wcast-align --param max-inline-insns-single=500
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../example.h"

//#include "hfmsgq.h"
#include "httpclient.h"

#if (EXAMPLE_USE_DEMO==USER_UPNP_DEMO)

EXTERNC const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE];

#ifdef __LPT230__
int g_module_id= HFM_TYPE_LPT230;

#define SSDP_PORT				(1900)
#define SSDP_ADDR				("239.255.255.250")

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HFM_NOPIN,		//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDO
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDI
	HFM_NOPIN,		//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	LPx30_GPIO2,	//HFGPIO_F_UART0_TX
	LPx30_GPIO23,	//HFGPIO_F_UART0_RTS
	LPx30_GPIO1,	//HFGPIO_F_UART0_RX
	LPx30_GPIO22,	//HFGPIO_F_UART0_CTS
	
	HFM_NOPIN,  	//HFGPIO_F_SPI_MISO
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CLK
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CS
	HFM_NOPIN,  	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,		//HFGPIO_F_UART1_TX,
	HFM_NOPIN,		//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,		//HFGPIO_F_UART1_RX,
	HFM_NOPIN,		//HFGPIO_F_UART1_CTS,
	
	LPx30_GPIO8,	//HFGPIO_F_NLINK
	LPx30_GPIO24,	//HFGPIO_F_NREADY
	LPx30_GPIO25,	//HFGPIO_F_NRELOAD
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_ON
	
	HFM_NOPIN,	    //HFGPIO_F_WPS
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	   	//HFGPIO_F_USER_DEFINE
	LPx30_GPIO5,	//HFGPIO_F_UPGRADE_LED
	LPx30_GPIO3,	//HFGPIO_F_UPGRADE_GPIO
};

#elif defined __LPT130__
int g_module_id= HFM_TYPE_LPT130;

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HFM_NOPIN,		//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDO
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDI
	HFM_NOPIN,		//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	LPx30_GPIO2,	//HFGPIO_F_UART0_TX
	HFM_NOPIN,		//HFGPIO_F_UART0_RTS
	LPx30_GPIO1,	//HFGPIO_F_UART0_RX
	HFM_NOPIN,		//HFGPIO_F_UART0_CTS
	
	HFM_NOPIN,  	//HFGPIO_F_SPI_MISO
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CLK
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CS
	HFM_NOPIN,  	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,		//HFGPIO_F_UART1_TX,
	HFM_NOPIN,		//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,		//HFGPIO_F_UART1_RX,
	HFM_NOPIN,		//HFGPIO_F_UART1_CTS,
	
	LPx30_GPIO22,	//HFGPIO_F_NLINK
	LPx30_GPIO23,	//HFGPIO_F_NREADY
	LPx30_GPIO3,	//HFGPIO_F_NRELOAD
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_ON
	
	HFM_NOPIN,	    //HFGPIO_F_WPS
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	   	//HFGPIO_F_USER_DEFINE
};

#elif defined __LPB130__
int g_module_id= HFM_TYPE_LPB130;

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HFM_NOPIN,		//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDO
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDI
	HFM_NOPIN,		//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	LPx30_GPIO2,	//HFGPIO_F_UART0_TX
	LPx30_GPIO23,	//HFGPIO_F_UART0_RTS
	LPx30_GPIO1,	//HFGPIO_F_UART0_RX
	LPx30_GPIO22,	//HFGPIO_F_UART0_CTS
	
	HFM_NOPIN,  	//HFGPIO_F_SPI_MISO
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CLK
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CS
	HFM_NOPIN,  	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,		//HFGPIO_F_UART1_TX,
	HFM_NOPIN,		//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,		//HFGPIO_F_UART1_RX,
	HFM_NOPIN,		//HFGPIO_F_UART1_CTS,
	
	LPx30_GPIO8,	//HFGPIO_F_NLINK
	LPx30_GPIO24,	//HFGPIO_F_NREADY
	LPx30_GPIO25,	//HFGPIO_F_NRELOAD
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_ON
	
	HFM_NOPIN,	    //HFGPIO_F_WPS
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	   	//HFGPIO_F_USER_DEFINE
};

#elif defined __LPT330__
int g_module_id= HFM_TYPE_LPT330;

const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HFM_NOPIN,		//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDO
	HFM_NOPIN,		//HFGPIO_F_JTAG_TDI
	HFM_NOPIN,		//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	LPx30_GPIO2,	//HFGPIO_F_UART0_TX
	HFM_NOPIN,		//HFGPIO_F_UART0_RTS
	LPx30_GPIO1,	//HFGPIO_F_UART0_RX
	HFM_NOPIN,		//HFGPIO_F_UART0_CTS
	
	HFM_NOPIN,  	//HFGPIO_F_SPI_MISO
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CLK
	HFM_NOPIN,	  	//HFGPIO_F_SPI_CS
	HFM_NOPIN,  	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,		//HFGPIO_F_UART1_TX,
	HFM_NOPIN,		//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,		//HFGPIO_F_UART1_RX,
	HFM_NOPIN,		//HFGPIO_F_UART1_CTS,
	
	LPx30_GPIO8,		//HFGPIO_F_NLINK
	LPx30_GPIO24,	//HFGPIO_F_NREADY
	LPx30_GPIO25,	//HFGPIO_F_NRELOAD
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_RQ
	HFM_NOPIN,	    //HFGPIO_F_SLEEP_ON
	
	HFM_NOPIN,	    //HFGPIO_F_WPS
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	   	//HFGPIO_F_USER_DEFINE
};
#else
#error "invalid project !you must define module type(__LPT230__/__LPT130__/__LPB130__/__LPT330__)"
#endif

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{NULL,NULL,NULL,NULL} //the last item must be null
};

extern int HSF_API hfnet_enable_multicast(int enable);
static int send_msearch_rsp_packet(int fd,char *pkt_buffer,int len,uint32_t if_addr,struct sockaddr_in *to);

static int sys_event_callback( uint32_t event_id,void * param)
{
	switch(event_id)
	{
		case HFE_WIFI_STA_CONNECTED:
			u_printf("wifi sta connected!!\n");
			break;
		case HFE_WIFI_STA_DISCONNECTED:
			u_printf("wifi sta disconnected!!\n");
			break;
		case HFE_CONFIG_RELOAD:
			u_printf("reload!\n");
			break;
		case HFE_DHCP_OK:
			{
				uint32_t *p_ip;
				p_ip = (uint32_t*)param;
				u_printf("dhcp ok %s!\n", inet_ntoa(*p_ip));
			}
			break;
		case HFE_SMTLK_OK:
			{
				u_printf("smtlk ok!\n");
				char *pwd=(char*)param;
				if(pwd == NULL)
					u_printf("key is null!\n");
				else
				{
					int i;
					for(i=0; i<(int)strlen(pwd); i++)
					{
						if(pwd[i]==0x1b)
							break;
					}
					for(i++; i<(int)strlen(pwd); i++)
						u_printf("user info 0x%02x-[%c]\n", pwd[i], pwd[i]);
				}
				msleep(100);
				return 1;
			}
			break;

		case HFE_WPS_OK:
			{
				u_printf("wps ok, key: %s!\n", (char*)param);
				//return 1;
			}
			break;
	}
	return 0;
}

static int get_if_ip_addr(void)
{
    char *words[6]={NULL};
    char rsp[64]={0};
    char is_ap_mode=0;

    if(hfat_send_cmd("AT+WMODE\r\n",sizeof("AT+WMODE\r\n"),rsp,64)!=0)
    	return 0;

    if(hfat_get_words(rsp,words, 6)>0)
    {
    	if(strcmp("STA",words[1])==0)
    	{
    		is_ap_mode=0;
    	}
    }

    if(is_ap_mode)
    {
    	hfat_send_cmd("AT+LANN\r\n",sizeof("AT+LANN\r\n"),rsp,64);
    }
    else
    	hfat_send_cmd("AT+WANN\r\n",sizeof("AT+WANN\r\n"),rsp,64);

    if(hfat_get_words(rsp,words, 6)>0)
    {
    	//u_printf("\nresult:%s\nmode:%s\nIP:%s\nMASK:%s\nGW:%s\n",words[0],words[1],words[2],words[3],words[4]);
    	if(is_ap_mode)
    		return inet_addr(words[1]);
    	else
    		return inet_addr(words[2]);
    }
    return 0;
}   

static int send_packet(int fd, const void *data, size_t len,struct sockaddr_in *to)
{
    int slen=0;
    static struct sockaddr_in toaddr;

    //DEBUG_PRINTF("%s len=%d fd=%d %d\n",__FUNCTION__,len,fd,svr->sockfd);
    if (to==NULL)
    {
        memset(&toaddr, 0, sizeof(struct sockaddr_in));
        toaddr.sin_family = AF_INET;
        toaddr.sin_port = htons(SSDP_PORT);
        toaddr.sin_len = sizeof(struct sockaddr_in);
        toaddr.sin_addr.s_addr = inet_addr(SSDP_ADDR);
        to = &toaddr;
    }
    
    slen = sendto(fd, (const char*)data, len, 0, (struct sockaddr *) to, sizeof(struct sockaddr_in));
    if(slen<0)
        u_printf("slen %d\n",slen);
    
    return slen;
}

#if 0
static int send_msearch_packet(int fd)
{
	char *msearch="M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\nST: LPB100\r\n\r\n";
	send_packet(fd,msearch,strlen(msearch),NULL);
	return 0;
}
#endif

static int send_alive_packet(int fd,char *pkt_buffer,int len,uint32_t if_addr)
{
	char *alivepkt="NOTIFY * HTTP/1.1\r\n"\
"Host:239.255.255.250:1900\r\n"\
"NT:urn:www.hi-flying.com:service:X_HF_LPB100:1\r\n"\
"NTS:ssdp:alive\r\n"\
"Location:%d.%d.%d.%d.48899\r\n"\
"Cache-Control:max-age=900\r\n"\
"Server:HI-FLYING/LPB100 UPnP/1.0 UPnP-Device-Host/1.0\r\n\r\n";
	sprintf(pkt_buffer,alivepkt,if_addr&0xFF,(if_addr>>8)&0xff,(if_addr>>16)&0xff,(if_addr>>24)&0xff);
	send_packet(fd,alivepkt,strlen(alivepkt),NULL);
	return 0;
}

static int create_multicast_socket(uint32_t if_addr)
{
    char optval;
    int r;
    int sd;
    int on=0;
    struct ip_mreq mreq;
    struct sockaddr_in serveraddr;

    sd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        return sd;
    }
    
    if ((r = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on))) < 0)
    {
        closesocket(sd);
        return r;
    }

        /* bind to an address */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SSDP_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* receive multicast */
    if ((r = bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
    {
        closesocket(sd);
        return -1; 
    }
    optval =0;
    if ((r = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &optval, sizeof(optval))) < 0)
    {
        return r;
    }
    /* Set multicast interface. */
    struct in_addr addr;    
    memset((void *)&addr, 0, sizeof(struct in_addr));
    addr.s_addr = if_addr;
    r = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF,(char *)&addr, sizeof(addr));
    if (r == -1)
    {
        closesocket(sd);
        return -1;
    }
    /*
    * setup time-to-live
    */
    optval = 10; /* Hop count */
    r = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, &optval, sizeof(optval));
    if (r)  
    {
        closesocket(sd);
        return r;
    }
    // add membership to receiving socket
    memset(&mreq, 0, sizeof(struct ip_mreq));
    mreq.imr_interface.s_addr = if_addr;
    mreq.imr_multiaddr.s_addr = inet_addr(SSDP_ADDR);
    if ((r = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq))) < 0)
    {
        closesocket(sd);
        return r;
    }
        
    return sd;
}

USER_FUNC static void upnp_main_thread(void* arg)
{
    int upnp_fd;
    uint32_t if_addr;
    fd_set sockfd_set;
    int max_fd ;
     struct timeval tv; 
     char *pkt_buffer;
     int ret;
#define UPNP_RECV_PACKET_SIZE      (1024)
    hfnet_enable_multicast(1);
    pkt_buffer = hfmem_malloc(UPNP_RECV_PACKET_SIZE);
    if(pkt_buffer==NULL)
    {
        u_printf("no mem\n");
        return;
    }
    while(1)
    {
        if((if_addr=get_if_ip_addr())==0)
        {
            msleep(500);
            continue;
        }
        upnp_fd = create_multicast_socket(if_addr);
        if(upnp_fd<0)
            return;
        else
            break;
    }
    max_fd =upnp_fd;
    
    while(1)
    {
        int recvsize=0;
        FD_ZERO(&sockfd_set);
        FD_SET(upnp_fd,&sockfd_set);
        tv.tv_sec=30;
        tv.tv_usec=0;
        u_printf("time=%d\n",hfsys_get_time());
        ret=select(max_fd+1,&sockfd_set,NULL,NULL,&tv);
        if(ret<=0)
        {
            send_alive_packet(upnp_fd,pkt_buffer,UPNP_RECV_PACKET_SIZE,if_addr);
            continue;
        }
        if(FD_ISSET(upnp_fd, &sockfd_set))
        {
            struct sockaddr_in fromaddr;
            socklen_t sockaddr_size = sizeof(struct sockaddr_in);
            memset(pkt_buffer,0,UPNP_RECV_PACKET_SIZE);
            recvsize = recvfrom(upnp_fd, (char*)pkt_buffer, UPNP_RECV_PACKET_SIZE-4, 0,(struct sockaddr *) &fromaddr, &sockaddr_size);
            if (recvsize < 0)
            {
                u_printf("recv() fail\n");
            }
            else
            {
                u_printf("recv length=%d\n",recvsize);
                if(strstr(pkt_buffer,"ssdp:discover")!=NULL&&strstr(pkt_buffer,"M-SEARCH")!=NULL)
                {
                    send_msearch_rsp_packet(upnp_fd,pkt_buffer,UPNP_RECV_PACKET_SIZE,if_addr,&fromaddr);
                }
                u_printf("%s\n",pkt_buffer);
            }
            
        }
    }

    if(pkt_buffer!=NULL)
        hfmem_free(pkt_buffer);
    
    return ;
}

USER_FUNC void upnp_start(void)
{
	hfthread_create(upnp_main_thread,"upnp_main",256,(void*)1,1,NULL,NULL);
}

static int send_msearch_rsp_packet(int fd,char *pkt_buffer,int len,uint32_t if_addr,struct sockaddr_in *to)
{
	char rsp_pkt[]="HTTP/1.1 200 OK\r\n"\
"Cache-Control: max-age=1800\r\n"\
"ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"\
"USN: uuid:12342409-1234-1234-5678-28c68ea3946a::urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"\
"EXT:\r\n"
"Server: OS 1.0 UPnP/1.0 Netgear/V1.3\r\n"\
"Location: %d.%d.%d.%d.48899\r\n\r\n";

	sprintf(pkt_buffer,rsp_pkt,if_addr&0xFF,(if_addr>>8)&0xff,(if_addr>>16)&0xff,(if_addr>>24)&0xff);
	return 0;
}

static int USER_FUNC socketa_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	if(event==HFNET_SOCKETA_DATA_READY)
		HF_Debug(DEBUG_LEVEL_LOW,"socketa recv %d bytes %d\n",len,buf_len);
	else if(event==HFNET_SOCKETA_CONNECTED)
		u_printf("socket a connected!\n");
	else if(event==HFNET_SOCKETA_DISCONNECTED)
		u_printf("socket a disconnected!\n");
	
	return len;
}

static int USER_FUNC socketb_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	if(event==HFNET_SOCKETB_DATA_READY)
		HF_Debug(DEBUG_LEVEL_LOW,"socketb recv %d bytes %d\n",len,buf_len);
	else if(event==HFNET_SOCKETB_CONNECTED)
		u_printf("socket b connected!\n");
	else if(event==HFNET_SOCKETB_DISCONNECTED)
		u_printf("socket b disconnected!\n");
			
	return len;
}

static int USER_FUNC uart_recv_callback(uint32_t event,char *data,uint32_t len,uint32_t buf_len)
{
	HF_Debug(DEBUG_LEVEL_LOW,"[%d]uart recv %d bytes data %d\n",event,len,buf_len);
	if(hfsys_get_run_mode() == HFSYS_STATE_RUN_CMD)
		return len;
	
	return len;
}

static void show_reset_reason(void)
{
	uint32_t reset_reason=0;
	reset_reason = hfsys_get_reset_reason();
	
#if 1
	u_printf("reset_reasion:%08x\n",reset_reason);
#else	
	if(reset_reason&HFSYS_RESET_REASON_ERESET)
	{
		u_printf("ERESET\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_IRESET0)
	{
		u_printf("IRESET0\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_IRESET1)
	{
		u_printf("IRESET1\n");
	}
	if(reset_reason==HFSYS_RESET_REASON_NORMAL)
	{
		u_printf("RESET NORMAL\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_WPS)
	{
		u_printf("RESET FOR WPS\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_SMARTLINK_START)
	{
		u_printf("RESET FOR SMARTLINK START\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_SMARTLINK_OK)
	{
		u_printf("RESET FOR SMARTLINK OK\n");
	}
	if(reset_reason&HFSYS_RESET_REASON_WPS_OK)
	{
		u_printf("RESET FOR WPS OK\n");
	}
#endif
	
	return;
}

void app_init(void)
{
	u_printf("app_init\n");
}

int USER_FUNC app_main (void)
{
	HF_Debug(DEBUG_LEVEL,"sdk version(%s),the app_main start time is %s %s\n",hfsys_get_sdk_version(),__DATE__,__TIME__);
	if(hfgpio_fmap_check(g_module_id)!=0)
	{
		while(1)
		{
			HF_Debug(DEBUG_ERROR,"gpio map file error\n");
			msleep(1000);
		}
	}

	hfsys_register_system_event(sys_event_callback);
	show_reset_reason();

	while(hfsmtlk_is_start())
		msleep(100);
	
	if(hfnet_start_uart(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)uart_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start uart fail!\n");
	}
	
	while(!hfnet_wifi_is_active())
	{
		msleep(50);
	}

	//See Wi-Fi Config tools APP for detailed usage of this thread
	if(hfnet_start_assis(ASSIS_PORT)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start assis fail\n");
	}
	
	//AT+NETP socket
	if(hfnet_start_socketa(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)socketa_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketa fail\n");
	}

	//AT+SOCKB socket
	if(hfnet_start_socketb(HFTHREAD_PRIORITIES_LOW,(hfnet_callback_t)socketb_recv_callback)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start socketb fail\n");
	}
	
	//Web Server
	if(hfnet_start_httpd(HFTHREAD_PRIORITIES_MID)!=HF_SUCCESS)
	{
		HF_Debug(DEBUG_WARN,"start httpd fail\n");
	}
	
    upnp_start();
    
	return 1;
}
#endif

