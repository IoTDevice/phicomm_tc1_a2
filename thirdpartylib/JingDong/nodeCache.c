#include "debugPro.h"

#ifdef QCOM_4004B
#include "scw_common.h"
#else
#include <stdio.h>
#include "hsf.h"

#include <stdlib.h>
#include "nodeCache.h"
#include "jutils.h"
#include "debugPro.h"
#include "auth/uECC.h"
#include "auth/aes.h"
#include "jdsmart.h"
#include "joylink_config.h"
#include "jdservice.h"
jdkey_t		jdKey[MAX_KEYLIST] = {0};		// 缓存N个Key
#endif
extern jdNVargs_t jdArgs;
jddevice_t  jdDev = {0};		// 在内存中维护N个设备
int devCount = 0;

eccContex_t myKey = {0};
int joylink_dev_get_private_key(char *out)
{
	//char jd_key[128]="615D8C9A5A90C49A9DA053BFDFA275E28F074827322963850AC7461E5D229712";
	memcpy(out,read_prikey(),64);
	return 0;
}
int joylink_check_cpu_mode(void)  
{  
    union
    {  
        int data_int;  
        char data_char;  
    } temp;

    temp.data_int = 1;  
    if(temp.data_char != 1)
	{
		return -1;
    }
	else
	{
		return 0;
    }
}

static void joylink_dev_init()
{
	/**
     *NOTE: If your model_code_flag is configed in cloud, 
     *Please set _g_pdev->model_code_flag = 1. 
     */
    /*_g_pdev->model_code_flag = 1;*/
	char mac[16] = {0};
	char key[68] = {0};

	u_printf("\n/**************info********************/\n");
	u_printf("sdk version: %s\n", _VERSION_);
	u_printf("dev version: %d\n",JLP_VERSION);
	u_printf("dev uuid: %s\n", DEVICE_UUID);
	u_printf("dev type: %d\n", 0);

	if(joylink_dev_get_private_key(key) < 0)
	{
    	custom_log("dev key: error!\n");
	}
	else
	{
		memcpy(jdDev.prikey,key,strlen(key));
		custom_log("dev key: %s\n", key);
	}
	jdDev.lancon = 1;
	jdDev.trantype = 1;
	if(joylink_check_cpu_mode() == 0)
	{
        custom_log("plt mode: --ok! little-endian--\n");
	}
	else
	{
		custom_log("plt mode: --error! big-endian--\n");
	}
	u_printf("/*************************************/\n\n");
}
jl2_d_idt_t user_idt = 
{
	.type = 0,
	.cloud_pub_key = CLOUD_PUB_KEY,

	.sig = "01234567890123456789012345678901",
	.pub_key = "01234567890123456789012345678901",

	.f_sig = "01234567890123456789012345678901",
	.f_pub_key = "01234567890123456789012345678901",
};

int eccContexInit(void)
{
	joylink_dev_init();
	if (!myKey.isInited)
	{
		myKey.isInited = 1;
		if (!uECC_make_key(myKey.devPubKey, myKey.priKey))
		{
			u_printf("uECC_make_key() failed\n");
			return 1;
		}
		uECC_compress(myKey.devPubKey, myKey.devPubKeyC);
	}

	memcpy(jdDev.pubkeyC, myKey.devPubKeyC, uECC_BYTES + 1);

	byte2hexstr(myKey.devPubKeyC, uECC_BYTES + 1, (uint8_t*)jdDev.pubkeyS, uECC_BYTES * 2 + 3);
	u_printf("DevicePubKey:\t%s\n", jdDev.pubkeyS);

	getProfile();

     if(hfsys_get_reset_reason()&HFSYS_RESET_REASON_SMARTLINK_OK)
	 {
		if(jdArgs.is_actived != 0)
		{  
			jdArgs.is_actived = 0;
			u_printf("&&&smart link ok\r\n");
		}
	}	
	char uuid[MAX_UUIDLEN];
	int uuid_len;
	uuid_len = jdservice_get_uuid(uuid);
	if(uuid_len < MAX_UUIDLEN)
	memcpy(jdDev.uuid, uuid, uuid_len);
	
	memcpy(jdDev.mac, getMac(), 18);
	custom_log("feed_id:   [%s]", jdArgs.feedid);
	custom_log("access_key:[%s]", jdArgs.accesskey);
	custom_log("localkey:  [%s]", jdArgs.localkey);
	custom_log("url:[%s], port:[%d]", jdArgs.server, jdArgs.port);
    custom_log("mac=%s",jdDev.mac);
	extern char DeviceDetailV0[200];
	sprintf(DeviceDetailV0, "{\"mac\":\"V0-Device\",\"productuuid\":\"NWE786\",\"feedid\":\"%s\",\"devkey\":\"%s\"}",
		"",
		jdDev.pubkeyS
	);

    return 0;
}


int isNodeExist(jddevice_t* dev)
{
	return 0;
}
char sys_mac[18];
char * getMac()
{
	//uint8_t sys_mac[18];
	const char cmd[] = "AT+WSMAC\r\n";
	char *words[6]={NULL};
	char rsp[64]={0};

	hfat_send_cmd((char*)cmd, sizeof(cmd), rsp, 64);
	if(hfat_get_words(rsp, words, 6)>0)
	{
		char* pMac = words[1];
		for(int i=0; i<6; i++)
		{
			sys_mac[2*i+0] = pMac[2*i+0];
			sys_mac[2*i+1] = pMac[2*i+1];
			//sys_mac[3*i+2] = '-';
		}
		sys_mac[2*5+2] = 0;
	}
	return sys_mac;
}

void clr_jdArgs(void)
{
   jdNVargs_t jdArgs_erase;
   jdArgs_erase.magic[0] = 0xFF;
   hffile_userbin_write(JDCONFIG_OFFSET, (char*)&jdArgs_erase, sizeof(jdNVargs_t));
}

char jd_prikey[128]={0};
char* read_prikey(void)
{  
   memset(jd_prikey,0,sizeof(jd_prikey));
   hffile_userbin_read(PRIKEY_OFFSET, jd_prikey, 64);
   return jd_prikey;
}

void write_prikey(char *prikey)
{
   hffile_userbin_write(PRIKEY_OFFSET,prikey, 64);
}

void getProfile()
{
	memset(&jdArgs,0,sizeof(jdNVargs_t));
	hffile_userbin_read(JDCONFIG_OFFSET, (char*)&jdArgs, sizeof(jdNVargs_t));
	if(memcmp(jdArgs.magic, "HF_JD", 5))
	{   //not same 
		//custom_log("not name HF_JD clear arg");
		clr_jdArgs();
		hffile_userbin_read(JDCONFIG_OFFSET, (char*)&jdArgs, sizeof(jdNVargs_t));
	}
}

