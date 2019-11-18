
#include "appmain.h"
#include "user_function.h"
#include <hsf.h>
#include <hfnet.h>
#include <string.h>
#include <mdns.h>

#include "mdns.h"

#define SYS_TXT_LEN  256

static struct mdns_service mico_system_service;
static char keyvals[SYS_TXT_LEN];

void mdns_thread( void *arg );

USER_FUNC void user_mdns_init( void )
{
   	hfthread_create(mdns_thread,"mdns",1024,(void*)1,1,NULL,NULL);  //∆Ù∂Ømdnsœﬂ≥Ã
}

/*create mdns socket*/
void USER_FUNC mdns_thread( void *arg )
{
	char *val;

	memset(&mico_system_service, 0x0, sizeof(struct mdns_service));
	mico_system_service.servname = "servname";
	mico_system_service.servtype = "easylink";
	mico_system_service.port = 80;
	mico_system_service.proto = MDNS_PROTO_TCP;

	val = "MAC";
	snprintf(keyvals, SYS_TXT_LEN, "MAC=%s.", val);
	free(val);

	val = "Firmware";
	snprintf(keyvals, SYS_TXT_LEN, "%sFirmware Rev=%s.", keyvals, val);
	free(val);

	val = "Hardware";
	snprintf(keyvals, SYS_TXT_LEN, "%sHardware Rev=%s.", keyvals, val);
	free(val);

	val = "os";
	snprintf(keyvals, SYS_TXT_LEN, "%sMICO OS Rev=%s.", keyvals, val);
	free(val);

	mdns_set_txt_rec(&mico_system_service, keyvals);

	mdns_start("start1", "start2");
	mdns_announce_service(&mico_system_service, NULL);
}
