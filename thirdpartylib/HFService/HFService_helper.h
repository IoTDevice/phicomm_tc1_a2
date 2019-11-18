
#ifndef _HFSERVICE_HELPER_H_
#define _HFSERVICE_HELPER_H_

// TODO: 存放公共函数
int handle_up_data_funtion(unsigned char* data,int len);
void check_sum_function(char* data,int len);
char get_my_wifi_rssi(void);
int USER_FUNC fireUartSend(unsigned char *data, int bytes);



#endif/*_HFSERVICE_HELPER_H_*/

