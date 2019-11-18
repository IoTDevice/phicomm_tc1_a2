/*****************************************************************************************
* File Name: jdota.c
*
* Description:
*  Common JingDong OTA file.
*
*****************************************************************************************/

#include <hsf.h>
#include <stdio.h>
#include "json/cJSON.h"
#include "jdsmart.h"
#include "jutils.h"
#include "packets.h"
#include "nodeCache.h"
#include "jdota.h"
#include "jdservice.h"
#include "httpclient.h"

extern char SendBuffer[];
extern jdNVargs_t jdArgs;
extern int serverSocket;
static char *file_url=NULL;
static int ota_notice_state = 0;
static int g_ota_notice_time = 0;

void joylink_set_ota_config(HFOtaConfig *data)
{
	data->validcode = JD_OTA_VALID_NUMBER;
	data->endcode = 0x87654321;
	hfuflash_erase_page(UFLASH_HFOTA_CONFIG_ADDR, 1);
	hfuflash_write(UFLASH_HFOTA_CONFIG_ADDR, (char * )data, sizeof(HFOtaConfig));
}

void joylink_get_ota_config(HFOtaConfig *data)
{
	hfuflash_read(UFLASH_HFOTA_CONFIG_ADDR, (char * )data, sizeof(HFOtaConfig));
	if((data->validcode != JD_OTA_VALID_NUMBER)||(data->endcode != 0x87654321))
	{
		data->validcode = JD_OTA_VALID_NUMBER;
		data->version_num = JD_OTA_FIRST_VERSION;
		data->update_state = 0;
		data->update_version = 0;
		data->endcode = 0x87654321;

		joylink_set_ota_config(data);
	}
}

void joylink_dev_ota_status_upload(int process)
{
	custom_log("joylink_dev_ota_status_upload[%d]\r\n", process);
	
	cJSON *root, *data;
	char *out = NULL;
	root = cJSON_CreateObject();
	if(NULL == root)
		return;
		
	data = cJSON_CreateObject();
	if(NULL == data)
	{
		cJSON_Delete(root);
		return;
	}
	cJSON_AddStringToObject(root, "cmd", "otastat");

	cJSON_AddStringToObject(data, "feedid", jdArgs.feedid);
	char uuid[20];
	jdservice_get_uuid(uuid);
	cJSON_AddStringToObject(data, "productuuid", uuid);

	if((process > 0)&&(process < 100))
	{
		cJSON_AddNumberToObject(data, "status", 0);
		cJSON_AddStringToObject(data, "status_desc", "Downloading ...");
		cJSON_AddNumberToObject(data, "progress", process);
	}
	else if(process == 0)
	{
		cJSON_AddNumberToObject(data, "status", 3);
		cJSON_AddStringToObject(data, "status_desc", "update failed.");
		cJSON_AddNumberToObject(data, "progress", 0);
	}
	else if(process == 100)
	{
		cJSON_AddNumberToObject(data, "status", 2);
		cJSON_AddStringToObject(data, "status_desc", "update succes.");
		cJSON_AddNumberToObject(data, "progress", 100);
	}
	else
	{
		cJSON_AddNumberToObject(data, "status", 1);
		cJSON_AddStringToObject(data, "status_desc", "Installing ...");
		cJSON_AddNumberToObject(data, "progress", 50);
	}      
	cJSON_AddItemToObject(root,"data", data);

	out=cJSON_Print(root);
	cJSON_Delete(root);
	
 	int json_len = strlen(out);
	char *rsp=(char *)hfmem_malloc(json_len+10);
	if(rsp == NULL)
	{
		hfmem_free(out);
		return;
	}
	
	time_t tt = time(NULL);
    	memcpy((char *)rsp, &tt, 4);
	strcpy(rsp+4, out);
	hfmem_free(out);

	int len = serverPacketBuild((uint8_t* )SendBuffer, UDP_MTU, PT_OTA_UPLOAD, jdDev.sessionKey, (const uint8_t*)rsp, 4+json_len);
	send(serverSocket, SendBuffer, len, 0);
	
	hfmem_free(rsp);
	return;
}

static int jd_update_thread(void)
{
	msleep(2000);
	joylink_dev_ota_status_upload(1);
	msleep(100);
	
	httpc_req_t  http_req;
	char *content_data=NULL;
	char *temp_buf=NULL;
	parsed_url_t url={0};
	http_session_t hhttp=0;
	int total_size,read_size=0;

	bzero(&http_req,sizeof(http_req));
	http_req.type = HTTP_GET;
	http_req.version=HTTP_VER_1_1;
	
	if((temp_buf = (char*)hfmem_malloc(256))==NULL)
		goto exit;
	int temp_buf_len=sizeof(temp_buf);
	memset(temp_buf,0,temp_buf_len);
	if(hfhttp_parse_URL(file_url,temp_buf , 256, &url)!=HF_SUCCESS)
		goto exit;

	if(hfhttp_open_session(&hhttp,file_url,0,NULL,3)!=HF_SUCCESS)
		goto exit;
    hfthread_disable_softwatchdog(NULL);
	hfupdate_start(HFUPDATE_SW);
	http_req.resource = url.resource;
	hfhttp_prepare_req(hhttp,&http_req,HDR_ADD_CONN_CLOSE);
	if(hfhttp_send_request(hhttp,&http_req)!=HF_SUCCESS)
		goto exit;
	
	content_data = (char*)hfmem_malloc(1024);
	if(content_data==NULL)
		goto exit;
	
	total_size=0;
	bzero(content_data,1024);
	while((read_size=hfhttp_read_content(hhttp,content_data,1024))>0)
	{
		hfupdate_write_file(HFUPDATE_SW, total_size,(uint8_t *)content_data, read_size);
		total_size+=read_size;
	}

	if(hfupdate_complete(HFUPDATE_SW,total_size)!=HF_SUCCESS)
	{
		custom_log("UPDAE DEBUG: do_update_version fail!\r");
	}
	else
	{
		custom_log( "UPDAE DEBUG: do_update_version success, reset module!\r");

		joylink_dev_ota_status_upload(101);
		HFOtaConfig data;
		joylink_get_ota_config(&data);
		data.update_state = 3;//start
		data.version_num = data.update_version;//start
		joylink_set_ota_config(&data);
		
		msleep(2000);
		hfsys_reset();
	}
	
exit:
	if(temp_buf!=NULL)	
		hfmem_free(temp_buf);
	if(content_data!=NULL)
		hfmem_free(content_data);
	if(hhttp!=0)
		hfhttp_close_session(&hhttp);	

	hfmem_free(file_url);
	file_url=NULL;
	g_ota_notice_time=0;
	//joylink_dev_ota_status_upload(0);
	HFOtaConfig data;
	joylink_get_ota_config(&data);
	data.update_state = 2;
	joylink_set_ota_config(&data);

	ota_notice_state = 0;
    hfthread_enable_softwatchdog(NULL,30);
	hfthread_destroy(NULL);
	return 0;
}

static int joylink_dev_ota(JLOtaOrder_t *otaOrder)
{
	if(file_url != NULL)
		return -1;
	
	if((file_url = (char*)hfmem_malloc(JL_MAX_URL_LEN))==NULL)
	{
		return -1;
	}	

	custom_log("JD start update, version[%d] url[%s]\r\n", otaOrder->version, otaOrder->url);
	//strcpy(file_url, "http://192.168.11.106/mfw.bin");//test
	strcpy(file_url, otaOrder->url);

	HFOtaConfig data;
	joylink_get_ota_config(&data);
	data.update_state = 1;//start
	data.update_version = otaOrder->version;
	joylink_set_ota_config(&data);

	g_ota_notice_time = hfsys_get_time();
	hfthread_create((PHFTHREAD_START_ROUTINE)jd_update_thread, "JD OTA", 512, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL);

	return 0;
}

static int joylink_parse_server_ota_order_req(JLOtaOrder_t* otaOrder, const char * pMsg)
{
	int ret = -1;
	if(NULL == pMsg || NULL == otaOrder)
	{
		custom_log("--->:ERROR: pMsg is NULL\n");
		return ret;
	}
	
	cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson)
	{
		custom_log("--->:cJSON_Parse is error:%s\n", pMsg);
		return ret;
	}

	cJSON * pData = cJSON_GetObjectItem(pJson, "serial");
	if(NULL != pData)
	{
		otaOrder->serial = pData->valueint;
	}

	pData = cJSON_GetObjectItem(pJson, "data");
	if(NULL != pData)
	{
		cJSON * pSub = cJSON_GetObjectItem(pData, "feedid");
		if(NULL != pSub)
			strcpy(otaOrder->feedid, pSub->valuestring);
		pSub = cJSON_GetObjectItem(pData, "productuuid");
		if(NULL != pSub)
			strcpy(otaOrder->productuuid, pSub->valuestring);
		pSub = cJSON_GetObjectItem(pData, "version");
		if(NULL != pSub)
			otaOrder->version = pSub->valueint;                                                                                          
		pSub = cJSON_GetObjectItem(pData, "versionname");
		if(NULL != pSub)
			strcpy(otaOrder->versionname, pSub->valuestring);
		pSub = cJSON_GetObjectItem(pData, "crc32");
		if(NULL != pSub)
			otaOrder->crc32 = pSub->valueint;
		pSub = cJSON_GetObjectItem(pData, "url");
		if(NULL != pSub)
			strcpy(otaOrder->url, pSub->valuestring);
	}

	cJSON_Delete(pJson);
	ret = 0;
	return ret;
}

void joylink_proc_server_ota_order(uint8_t* json, int json_len)
{
	int ret = -1;
	int len = 0;
	JLOtaOrder_t *otaOrder = (JLOtaOrder_t *)hfmem_malloc(sizeof(JLOtaOrder_t));
	if(otaOrder == NULL)
		return;
	bzero(otaOrder, sizeof(JLOtaOrder_t));

	ret = joylink_parse_server_ota_order_req(otaOrder, (char *)json);
	if(ret != 0)
	{
		hfmem_free(otaOrder);
		return;
	}
	
	ret = joylink_dev_ota(otaOrder);
	if(ret != 0)
	{
		custom_log("dev ota error:ret:%d\n", ret);
		hfmem_free(otaOrder);
		return;
	}

	memset(json, 0 , 200);
	char *rsp=(char *)(json+4);
	
	time_t tt = time(NULL);
	memcpy((char *)json, &tt, 4);
	sprintf(rsp, "{\"code\":0,\"serial\":%d,\"msg\":\"success\"}", otaOrder->serial);

	custom_log("Send OTA cmd response: %s\r\n", rsp);
	len = serverPacketBuild((uint8_t* )SendBuffer, UDP_MTU, PT_OTA_ORDER, jdDev.sessionKey, (const uint8_t*)json, strlen(rsp)+4);
	send(serverSocket, SendBuffer, len, 0);

	hfmem_free(otaOrder);
	return;
}

void joylink_proc_server_ota_upload(uint8_t* json, int json_len)
{
	if((ota_notice_state == 1)||(ota_notice_state == 2))//update success
	{
		HFOtaConfig data;
		joylink_get_ota_config(&data);
		data.update_state = 0;//start
		joylink_set_ota_config(&data);
	
		ota_notice_state = 3;
	}
	
	return;
}

void joylink_ota_check_process(void)
{
	if(g_ota_notice_time != 0)
	{
		if(hfsys_get_time() - g_ota_notice_time >= 3*60*1000)
			hfsys_softreset();
	}
	
	static int ota_notice_time = 0;
	extern int isConnected;
	if(isConnected != 2)//not connected to server
		return;

	if(hfsys_get_time() - ota_notice_time <= 5*1000)
		return;
	ota_notice_time = hfsys_get_time(); 
	
	if(ota_notice_state == 0)
	{
		HFOtaConfig data;
		joylink_get_ota_config(&data);
		if(data.update_state != 0)
		{
			if(data.update_state == 3)
				ota_notice_state = 1;
			else
				ota_notice_state = 2;
		}
		else
			ota_notice_state = 3;
	}

	if(ota_notice_state == 1)//update success
	{
		joylink_dev_ota_status_upload(100);
	}

	if(ota_notice_state == 2)//update failed
	{
		joylink_dev_ota_status_upload(0);
	}
}

