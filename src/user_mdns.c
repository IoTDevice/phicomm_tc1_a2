#include "appmain.h"
#include "user_function.h"
#include <hsf.h>
#include <hfnet.h>
#include <string.h>
#include <mdns.h>

void mdns_thread( void *arg );

static char module_mac[21];
static char txt_record[] = "name=TC1_A2:model=com.iotserv.devices.phicomm_tc1_a1:mac=GHS00GG:id=GHS00GG";//max len is 255

static struct mdns_service openIoTHub = {
	.servname = module_mac,
	.servtype = "iotdevice",
	.domain   = "local",
	.proto    = MDNS_PROTO_TCP,
	.port     = 80,
	.keyvals = "",
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
	
	char *temp_txt = malloc(250);
	sprintf(temp_txt, "mac=%s:", module_mac);
  //sprintf(temp_txt, "%sfirmware-version=%s:", temp_txt, "1.0");
  sprintf(temp_txt, "%sname=%s:", temp_txt, "TC1_A2");
  sprintf(temp_txt, "%sauthor=%s:", temp_txt, "Farry");
  sprintf(temp_txt, "%semail=%s:", temp_txt, "newfarry@126.com");
  //sprintf(temp_txt, "%shome-page=%s:", temp_txt, "github.com//iotdevice");
  sprintf(temp_txt, "%sfirmware-respository=%s:", temp_txt, "github.com/IoTDevice/phicomm_tc1_a2");
  sprintf(temp_txt, "%smodel=%s:", temp_txt, "com.iotserv.devices.phicomm_tc1_a1");
  sprintf(temp_txt, "%sid=%s", temp_txt, module_mac);
	
	mdns_set_txt_rec(&openIoTHub, temp_txt);
	mdns_announce_service(&openIoTHub, "STA");
}