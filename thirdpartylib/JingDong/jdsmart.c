#include "jdsmart.h"
#include "jdservice.h"
#include "joylink_config.h"
#include "joylink_auth_uECC.h"

usr_ts_t _g_UT[USR_TIMESTAMP_MAX];

static int get_ip_port(char *ip_port)
{	
	char *p = strtok(ip_port,":");
	if(p){
		memset(jdArgs.server, 0, sizeof(jdArgs.server));
		memcpy(jdArgs.server, p,strlen(p));
	}
	else
		return -1;
	
	p = strtok(NULL, ":");
	if(p){
		jdArgs.port = str2int(p);
	}
	else
		return -1;
	return 1;
}
int joylink_util_hexStr2bytes(const char *hexStr, uint8_t *buf, int bufLen)
{
	int i;
	int len;

	if(NULL == hexStr)
	{
		len = 0;
	}
	else
	{
		len = (int)strlen(hexStr) / 2;
		if(bufLen < len)
		{
			len = bufLen;
        }
	}
	memset(buf, 0, bufLen);

	for(i = 0; i < len; i++)
	{
		char ch1, ch2;
		int val;

		ch1 = hexStr[i * 2];
		ch2 = hexStr[i * 2 + 1];
		if(ch1 >= '0' && ch1 <= '9')
		{
			val = (ch1 - '0') * 16;
        }
		else if (ch1 >= 'a' && ch1 <= 'f')
		{
			val = ((ch1 - 'a') + 10) * 16;
        }
		else if (ch1 >= 'A' && ch1 <= 'F')
		{
			val = ((ch1 - 'A') + 10) * 16;
        }
		else
        {
			return -1;
        }

		if(ch2 >= '0' && ch2 <= '9')
		{
			val += ch2 - '0';
        }
		else if(ch2 >= 'a' && ch2 <= 'f')
		{
			val += (ch2 - 'a') + 10;
        }
		else if (ch2 >= 'A' && ch2 <= 'F')
		{
			val += (ch2 - 'A') + 10;
        }
		else
		{
			return -1;
        }

		buf[i] = val & 0xff;
	}

	return 0;
}
static int32_t joylink_util_RSHash(const char * str)
{
    if(NULL == str){
        return E_RET_ERROR;
    }
    int32_t  b  =   378551 ;
    int32_t  a  =   63689 ;
    int32_t  hash  =   0 ;
    while(* str)
	{
        hash = hash * a + (*str++);
        a *= b;
    }
    return (hash  & 0x7FFFFFFF );
}
int joylink_util_cut_ip_port(const char *ipport, char *out_ip, int *out_port)
{
    if(NULL == ipport || 
            NULL == out_ip ||
            NULL == out_port){
        return E_RET_ERROR;
    }
    int ret = E_RET_OK;
    int offset = 0;
    int len = strlen(ipport);

    while(ipport[offset] != ':' && offset < len){
        offset++;
    }
    
    if(offset < len){
        memcpy(out_ip, ipport, offset);
        out_ip[offset] = '\0';
        *out_port = atoi(ipport + offset + 1);
    }else{
        ret = E_RET_ERROR;
    }

	return ret;
}

int joylink_is_usr_timestamp_ok(char *usr,unsigned int timestamp)
{
    char ip[64] = {0};
    int port;
    joylink_util_cut_ip_port(usr, ip, &port);
	custom_log("ip =%s,port=%d",ip,port);
    unsigned int id = joylink_util_RSHash(ip);
    int i;
	custom_log("timestamp info:->%s:%u\n", usr, timestamp);
    timestamp = timestamp & 0x7FFFFFFF;
    for(i=0; i < USR_TIMESTAMP_MAX; i++)
	{
        /*update the timestamp*/
        if((_g_UT[i].id & 0x7FFFFFFF) == (id & 0x7FFFFFFF))
		{	
            if(_g_UT[i].timestamp <= timestamp)
			{
                _g_UT[i].timestamp = timestamp;
                return 1;
            }
			else
			{
			    custom_log("timestamp error");
                return 0;
            }
        }
    }
    if(i == USR_TIMESTAMP_MAX)
	{
        /*no find usr add a usr to empty sapce*/
        for(i=0; i < USR_TIMESTAMP_MAX; i++)
		{
            if((_g_UT[i].id & 0x80000000) == 0)
			{
                _g_UT[i].timestamp = timestamp;
                _g_UT[i].id = id | 0x80000000; 
                return 1;
            }
        }
    }
    if(i == USR_TIMESTAMP_MAX)
	{
        /*no find no empty , add a usr to timeout space*/
        for(i=0; i < USR_TIMESTAMP_MAX; i++)
		{
            if(_g_UT[i].timestamp < (jdArgs.cloud_timestamp - 60*60))
			{
                _g_UT[i].timestamp = timestamp;
                _g_UT[i].id = id | 0x80000000; 
                return 1;
            }
        }
    }
    u_printf("JSon Control timstamp error: no space->%s\n", ip);
    u_printf("usr timestamp:%d, cache timestamp:%d\n", timestamp, _g_UT[i].timestamp);
    /*no space to add*/
    return 0;    
}

int write_accesskey(char *recPainText)
{
	int timestamp;
	uint8_t pubkey[33] = {0}; 
	uint8_t sig[64] = {0}; 
	int len = 0;
	memcpy(&timestamp, recPainText, 4);
	custom_log("=======Time:%d", timestamp);
	custom_log("=======jdArgs.is_actived:%x", jdArgs.is_actived);
	if(jdArgs.is_actived)
	{
	    int len = V1_WriteRet(E_RET_ERROR_DEV_ACTIVED, "dev is alread actived");   
        custom_log("dev is alread actived, not response write key");
		return len;
	}
	else
	{
		cJSON * jDevice = cJSON_Parse(recPainText+4);
		jDevice= cJSON_GetObjectItem(jDevice,"data");
		cJSON * pItem = cJSON_GetObjectItem(jDevice,"feedid");
		strcpy(jdArgs.feedid, pItem->valuestring);
		pItem = cJSON_GetObjectItem(jDevice,"accesskey");
		strcpy(jdArgs.accesskey, pItem->valuestring);
		pItem = cJSON_GetObjectItem(jDevice,"localkey");
		strcpy(jdArgs.localkey, pItem->valuestring);
				
		pItem = cJSON_GetObjectItem(jDevice,"joylink_server");
	    //add by wjt
	    cJSON * idt = cJSON_GetObjectItem(jDevice, "c_idt");
	    if(NULL != idt)
		{
	         cJSON * pSub = cJSON_GetObjectItem(idt, "cloud_sig");
	        if(NULL != pSub)
			{
	            strcpy(jdArgs.cloud_sig, pSub->valuestring);
				custom_log("#cloud_sig:[%s]", jdArgs.cloud_sig);
	        }                                                                                                         
	    }
		memset(jdArgs.cloud_pub_key,0,sizeof(jdArgs.cloud_pub_key));
		memcpy(jdArgs.cloud_pub_key,CLOUD_PUB_KEY,sizeof(CLOUD_PUB_KEY));
		custom_log("jdArgs.cloud_pub_key:[%s]", jdArgs.cloud_pub_key);
		joylink_util_hexStr2bytes(jdArgs.cloud_pub_key, pubkey, sizeof(pubkey));
	    joylink_util_hexStr2bytes(jdArgs.cloud_sig, sig, sizeof(sig));
		if(1 == jl3_uECC_verify_256r1((uint8_t *)pubkey, (uint8_t *)jdDev.rand,strlen(jdDev.rand),(uint8_t *)sig))
		{
			pItem = cJSON_GetObjectItem(jDevice,"joylink_server");
			cJSON * serverip_list = pItem->child;
			while(serverip_list)
			{
				get_ip_port(serverip_list->valuestring);
				serverip_list = serverip_list->next;
			}
			jdArgs.is_actived = 1;
		    len = V1_WriteRet(0, "write accesskey ok");
			custom_log("write accesskey ok");
		}
		else
		{
	        len = V1_WriteRet(-1, "verify cloud sig error");
			custom_log("get jd ip port fail");
		}
		memset(jdArgs.magic,0,sizeof(jdArgs.magic));
		memcpy(jdArgs.magic, "HF_JD", 5);
	    hffile_userbin_write(JDCONFIG_OFFSET, (char*)&jdArgs, sizeof(jdNVargs_t));
		
		custom_log("feed_id:   [%s]", jdArgs.feedid);
		custom_log("access_key:[%s]", jdArgs.accesskey);
		custom_log("localkey:  [%s]", jdArgs.localkey);
		custom_log("url:[%s], port:[%d]", jdArgs.server, jdArgs.port);
		cJSON_Delete(jDevice);
		return len;
    }
}

int searchEnable()
{
	int smtlk = 0;
	hfsys_nvm_read(0,(char *)&smtlk,4);
	if(!((hfsys_get_time() < 2000*60) /*&& (smtlk == 1)*/)) //Á½·ÖÖÓÄÚÇÒÅäÍøok
	{
		smtlk = 0;
		hfsys_nvm_write(0,(char *)&smtlk,4);
		return 0;
	}
	else
		return 1;
}

