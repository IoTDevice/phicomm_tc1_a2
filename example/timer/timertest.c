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

#if (EXAMPLE_USE_DEMO==USER_TIMER_DEMO)

EXTERNC const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE];

#ifdef __LPT230__
int g_module_id= HFM_TYPE_LPT230;

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

static int test_data=0;
hfthread_mutex_t test_lock=NULL_MUTEX;
void test_thread_start(void);

#define PRINTF(...)	HF_Debug(DEBUG_LEVEL,__VA_ARGS__)

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

#define TEST_TIMER_ID		(1)
#define TEST_TIMER2_ID		(2)
#define TEST_TIMER3_ID		(3)

hftimer_handle_t test_timer_hardware=NULL;

void USER_FUNC test_timer_callback( hftimer_handle_t htimer )
{
        if(hftimer_get_timer_id(htimer)==TEST_TIMER_ID)
        {
            //u_printf("TEST_TIMER_ID active\n");
            if(hfgpio_fpin_is_high(HFGPIO_F_NREADY))
                hfgpio_fset_out_low(HFGPIO_F_NREADY);
            else
                hfgpio_fset_out_high(HFGPIO_F_NREADY);
            //hftimer_start(htimer);//如果create的时候auto_reload设置为false，手动再次启动timer 
        }
        else if(hftimer_get_timer_id(htimer)==TEST_TIMER2_ID)
        {
            //u_printf("TEST_TIMER_ID active\n");
            if(hfgpio_fpin_is_high(HFGPIO_F_NLINK))
            {
                hftimer_change_period(htimer,1000);
                hfgpio_fset_out_low(HFGPIO_F_NLINK);
            }
            else
            {
                hftimer_change_period(htimer,3000);
                hfgpio_fset_out_high(HFGPIO_F_NLINK);
            }
            
            //hftimer_start(htimer);        
        }
        else if(hftimer_get_timer_id(htimer)==TEST_TIMER3_ID)
        {
            if(hfgpio_fpin_is_high(HFGPIO_F_USER_DEFINE))
                hfgpio_fset_out_low(HFGPIO_F_USER_DEFINE);
            else
                hfgpio_fset_out_high(HFGPIO_F_USER_DEFINE);
        }
        else
        {
            u_printf("%p\n",htimer);
        }
}

USER_FUNC static void test_thread_func(void* arg)
{
    int fd,id;
    int tmp=1,recv_num=0;
    char recv[32]={0};  
    //char *p;
    struct sockaddr_in addr;
    hftimer_handle_t test_timer=NULL;
    hftimer_handle_t test_timer2=NULL;
    
    id = (int)arg;
    memset((char*)&addr,0,sizeof(addr));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10001+id);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    // = socket()
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    tmp=1;
    setsockopt(fd, SOL_SOCKET,SO_BROADCAST,&tmp,sizeof(tmp));
    
    //创建一个自动定时器，每1s钟触发一次。
    if((test_timer = hftimer_create("TEST-TIMER",1000,TRUE,TEST_TIMER_ID,test_timer_callback,0))==NULL)
    {
        u_printf("create timer 1 fail\n");
    }
    if((test_timer2 = hftimer_create("TEST-TIMER2",1000,FALSE,TEST_TIMER2_ID,test_timer_callback,0))==NULL)
    {
        u_printf("create timer 2 fail\n");
    }

#if 0
    //0.5ms hardware timer,硬件定时器周期以微秒计算,只能建一个硬件定时器.
    if((test_timer_hardware=hftimer_create("HDW-TIMER",500,true,TEST_TIMER3_ID,test_timer_callback,HFTIMER_FLAG_HARDWARE_TIMER))==NULL)
#else
    if((test_timer_hardware=hftimer_create("HDW-TIMER",110000,TRUE,TEST_TIMER3_ID,test_timer_callback,HFTIMER_FLAG_HARDWARE_TIMER))==NULL)
#endif
    {
        u_printf("create  hardware timer fail\n");
    }
    //启动定时器
    hftimer_start(test_timer);
    hftimer_start(test_timer2);
    hftimer_start(test_timer_hardware);
    #if 0 //LLJ CHANGE
    while(1)
    {
        REG_TIM0_CTRL |= TIME_CTRL_CNT_CLEAR;
        u_printf("cnt0=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL,REG_TIM0_CNT_INIT);
        u_printf("cnt1=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL,REG_TIM0_CNT_INIT);
        u_printf("cnt2=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL,REG_TIM0_CNT_INIT);
        REG_TIM0_CTRL |= TIME_CTRL_CNT_CLEAR;
        u_printf("cnt3=%d %08X %d\n",REG_TIM0_CNT,REG_TIM0_CTRL);
        msleep(1);
    }
    #endif
    while(1)
    {
        hfthread_mutext_lock(test_lock);
        test_data=id;
        HF_Debug(DEBUG_LEVEL_LOW,"thread %d is running\n",test_data);
        msleep(3000);
        hfthread_mutext_unlock(test_lock);
        tmp = sizeof(addr);
        //recv_num = recvfrom(fd, recv, 32, 0, (struct sockaddr *)&addr, (socklen_t*)&tmp);
        if(recv_num>0)
        {
            HF_Debug(DEBUG_LEVEL,"thread %d recvnum=%d\n",id,recv_num);
            sprintf(recv,"thread %d\r\n",id);
            sendto(fd,recv,strlen(recv),0,(struct sockaddr *)&addr,sizeof(addr));
        }
    }
    
    //hftimer_delete(test_timer);
}

USER_FUNC void test_thread_and_timer_start(void)
{
	if(hfthread_mutext_new(&test_lock)!=0)
	{
		HF_Debug(DEBUG_LEVEL,"create mutex fail\n");
		return;
	}
	
	hfthread_create(test_thread_func,"app_main_test",256,(void*)1,1,NULL,NULL);
	//hfthread_create(test_thread_func,"app_main_test1",256,(void*)2,1,NULL,NULL);
	
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
	
    test_thread_and_timer_start();
    
	return 1;
}
#endif

