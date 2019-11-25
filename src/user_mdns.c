#include "appmain.h"
#include "user_function.h"
#include <hsf.h>
#include <hfnet.h>
#include <string.h>
#include <mdns.h>

void mdns_thread( void *arg );

static char module_mac[21];

static struct mdns_service test_service1 = {
	.servname = module_mac,
	.servtype = "iotdevice",
	.domain   = "local",
	.proto    = MDNS_PROTO_TCP,
	.port     = 80,
	.keyvals = "DynagteID=1234567890",
	.kvlen = sizeof("DynagteID=1234567890")
};

USER_FUNC void user_mdns_init( void )
{
	hfthread_create(mdns_thread,"mdnscontrol",1024,(void*)1,1,NULL,NULL);
}

void USER_FUNC mdns_thread( void *arg )
{
	while(!hfnet_wifi_is_active())
	{
		msleep(50);
	}
	hfnet_get_mac_address(module_mac);
	mdns_start("local", module_mac);

	mdns_announce_service(&test_service1, "STA");
}