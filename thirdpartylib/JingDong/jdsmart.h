#ifndef _JDSMART_H_
#define _JDSMART_H_

#include "debugPro.h"
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "json/cJSON.h"
#include "jutils.h"
#include "nodeCache.h"


#define NUM_TOKENS          100
#define UDP_MTU		        1400
#define JDCONFIG_OFFSET     0
#define PRIKEY_OFFSET     560
#define JL_MAX_PACKET_LEN 1400
#define USR_TIMESTAMP_MAX      (4)

typedef enum {
	PHASE_ESTABLISH = 0,
	PHASE_AUTHENTICATE = 1,
	PHASE_READY = 2
}WORKFLOW;

typedef struct _u_t{
    unsigned int id;
    unsigned int timestamp;
}usr_ts_t;

typedef enum _ret_code{
	E_RET_ERROR_DEV_ACTIVED         = -2001,
    E_RET_ERROR_PKG_SAME            = -1001,
    E_RET_ERROR_PKG_NUM_BREAK_MAX   = -1002,
    E_RET_ERROR_PKG_BREAK_ARRAY     = -1003,
    E_RET_ERROR_PARAM_INVAID        = -3,
    E_RET_ERROR_NO_EXIST            = -2,
    E_RET_ERROR                     = -1,
    E_RET_OK                        = 0,
    E_RET_ALL_DATA_HIT              = 1001,
    E_RET_DATA_COMING               = 1002
}E_JLRetCode_t;

extern jdNVargs_t jdArgs;
typedef enum{
	LOCAL_CTL,
	REMOTE_CTL,
	
}CMD_SRC;
enum JDSMART_CALLBACK {
    JD_SET_DEVICE_STATUS_BY_RAWDATA = 0,
};

typedef enum{
	COMMON_CTL,
	CLDMENU_CTL,
	
}CMD_TYPE;

typedef void (*jdsmart_callback)(CMD_SRC, CMD_TYPE, uint8_t *, uint8_t);
int joylink_is_usr_timestamp_ok(char *usr,unsigned int timestamp);

int write_accesskey(char *recPainText);
int searchEnable(void);

#endif
