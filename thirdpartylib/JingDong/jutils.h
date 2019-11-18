#ifndef _UTILS_H
#define _UTILS_H
//#include "debugPro.h"
//#include "jdsmart.h"


#ifdef QCOM_4004B
int tolower(int ch);
#else
//#include <stdint.h>
#endif
//#include <hsf.h>

#define JL_UTILS_P_FMT        (1)
#define JL_UTILS_P_NO_FMT     (0)

void joylink_util_print_buffer(const char *msg, int is_fmt, int num_line, const uint8_t *buff, int len);

#define joylink_util_fmt_p(msg, buff, len) joylink_util_print_buffer(msg, JL_UTILS_P_FMT, 16, buff, len) 


int byte2hexstr(const uint8_t *pBytes, int srcLen, uint8_t *pDstStr, int dstLen);
int hexStr2bytes(const char *hexStr, uint8_t *buf, int bufLen);
//int utilGetIPString(struct sockaddr_in* pPeerAddr, char* str);
int utilGetIPString(struct sockaddr_in* pPeerAddr, char* str);

void timerReset(uint32_t *timestamp);
int isTimeOut(uint32_t timestamp, uint32_t timeout);
int addressis_ip(const char * ipaddr);
int str2int(char* pStr);


void *joylink_util_malloc(size_t size);
void joylink_util_free(void *p);
uint8_t joyTLVDataAdd(uint8_t *buf,uint8_t tag, uint8_t lc, uint8_t *value);

#endif /* utils.h */
