/*****************************************************************************************
* File Name: jdota.h
*
* Description:
*  Common JingDong OTA file.
*
*****************************************************************************************/

#ifndef _JDOTA_H_
#define _JDOTA_H_

#define JD_OTA_VALID_NUMBER (0x12345678)
#define JD_OTA_FIRST_VERSION 3

#define UFLASH_HFOTA_CONFIG_ADDR	0
#define UFLASH_HFOTA_CONFIG_SIZE	0x1000

#define JL_MAX_UUID_LEN             (10)
#define JL_MAX_FEEDID_LEN           (33)
#define JL_MAX_VERSION_NAME_LEN    	(100)
#define JL_MAX_URL_LEN             	(100)
#define JL_MAX_MSG_LEN 				(1024)
#define JL_MAX_STATUS_DESC_LEN      (100)

typedef struct{
	int serial;
    char feedid[JL_MAX_FEEDID_LEN];
    char productuuid[JL_MAX_UUID_LEN];
    int version;
    char versionname[JL_MAX_VERSION_NAME_LEN];
    unsigned int crc32;
    char url[JL_MAX_URL_LEN];
}JLOtaOrder_t;

typedef struct{
    char feedid[JL_MAX_FEEDID_LEN];
    char productuuid[JL_MAX_UUID_LEN];
    int status;
    char status_desc[JL_MAX_STATUS_DESC_LEN];
    int progress;
}JLOtaUpload_t;

typedef struct {
	int code;
	char msg[JL_MAX_MSG_LEN];
}JLOtaUploadRsp_t;

typedef struct {
	int validcode;//JD_OTA_VALID_NUMBER
	int version_num;
	int update_state;// 1- updating, 2, notice, 3-success
	int update_version;
	int endcode;//0x87654321
}HFOtaConfig;


void joylink_set_ota_config(HFOtaConfig *data);
void joylink_get_ota_config(HFOtaConfig *data);
void joylink_proc_server_ota_order(uint8_t* json, int json_len);
void joylink_proc_server_ota_upload(uint8_t* json, int json_len);
void joylink_ota_check_process(void);

#endif
