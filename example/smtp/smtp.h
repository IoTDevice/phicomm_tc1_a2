
#ifndef _SMTP_H_
#define _SMTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hsf.h"


#define EMAIL_BUF_SIZE 1500

#define EMAIL_SERVER_PORT 25
#define EMAIL_SERVER_ADDR "smtp.xxxx.com"
#define EMAIL_FROM_ACCOUNT "yyyy@xxxx.com"
#define EMAIL_FROM_PASSWD "abcd1234"
#define EMAIL_TO_ACCOUNT "zzzz@xxxx.com"


void start_smtp_demo(void);


#ifdef __cplusplus
}
#endif

#endif

