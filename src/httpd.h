#ifndef _WEBUI_H_
#define _WEBUI_H_

#include <hsf.h>

int USER_FUNC httpd_add_page(const char *url, void (*callback)(char *url, char *rsp));
int USER_FUNC httpd_arg_find(char *url,char *slot0,char *slot1,char *slot2,char *slot3,char *slot4,char *slot5);

void USER_FUNC httpd_init(void);

#endif
