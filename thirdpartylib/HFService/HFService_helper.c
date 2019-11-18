
#include "HFService_config.h"
#include "HFService_helper.h"
#include <string.h>
#include "hsf.h"
hfuart_handle_t huart0 = NULL; //串口句柄，用来和串口通讯
hfthread_mutex_t g_uartMutex = 0;//串口下发数据的锁
int uart_timeouts = 500;


char get_my_wifi_rssi(void)
{
	char rsp[64]={0};//存放AT命令结果
	char rsp_char=0;
	char *words[6]={NULL};
	int rsp_int=0;
	hfat_send_cmd("AT+WSLQ\r\n",sizeof("AT+WSLQ\r\n"),rsp,64);
	hfat_get_words(rsp,words,6);
	u_printf("CMD::AT+WSLQ\n");
	if (strncmp(words[1],"Disconnected",sizeof("Disconnected"))!=0)
	{
	    
		rsp_int=atoi(words[2]);
		u_printf("rsp_int is %d\r\n",rsp_int);
		/*if((0< rsp_int) &&(rsp_int<= 20))
			rsp_char=1;
		else if((20< rsp_int) &&(rsp_int<= 40))
			rsp_char=2;
		else if((40< rsp_int) &&(rsp_int<= 60))
			rsp_char=3;
		else if((60< rsp_int) &&(rsp_int<= 80))
			rsp_int=4;
		else if((80< rsp_int) &&(rsp_int<= 100))
			rsp_char=5;*/
		rsp_char=(char)rsp_int;	
	}
	else
	{
		rsp_char=0;
	}
	u_printf("####$$$@ sign streng is %d\r\n",rsp_int);
	u_printf("####$$$@ sign streng is %d\r\n",rsp_char);
	return rsp_char;
}

int USER_FUNC fireUartSend(unsigned char *data, int bytes)
{
    if(g_uartMutex == 0)
    {
        if(HF_SUCCESS != hfthread_mutext_new(&g_uartMutex))
        {
            u_printf("HF_SUCCESS != hfthread_mutext_new(&g_uartMutex)!!!\r\n");
            return 0;
        }
    }
    hfthread_mutext_lock(g_uartMutex);
    int iSendedNum = hfuart_send(HFUART0, (char*)data, bytes, uart_timeouts);
    hfthread_mutext_unlock(g_uartMutex);

    return iSendedNum;
}




