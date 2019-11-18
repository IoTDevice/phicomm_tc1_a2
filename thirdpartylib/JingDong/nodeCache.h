#ifndef _NODECACHE_H
#define _NODECACHE_H

#ifdef QCOM_4004B
#include "uECC.h"
#else
#include "uECC.h"
#endif


typedef struct {
	int isInited;
	uint8_t priKey[uECC_BYTES];
	uint8_t devPubKey[uECC_BYTES * 2];
	uint8_t devPubKeyC[uECC_BYTES + 1];
}eccContex_t;

extern eccContex_t myKey;

#define MAX_IPLEN     20
#define MAX_MACLEN    128
#define MAX_UUIDLEN   10
#define MAX_FEEDIDLEN 33
#define MAX_PKEYBIN   21
#define MAX_PKEYSTR   21*2+1
#define MAX_DBGLEN    128
#define MAX_OPTLEN    128
#define IDT_D_RNAD_LEN    (33)
#define IDT_D_PK_LEN            (34)
#define IDT_C_PK_LEN            (67)
#define IDT_D_SIG_LEN           (65)
#define IDT_D_RNAD_LEN          (33)
#define IDT_F_SIG_LEN           (65)
#define IDT_F_PK_LEN            (34)
#define IDT_A_RNAD_SIG_LEN      (33)
#define IDT_C_SIG_LEN           (65)

typedef struct {
    int      type;
	char	 cloud_pub_key[IDT_C_PK_LEN];
	char	 pub_key[IDT_D_PK_LEN];
	char	 sig[IDT_D_SIG_LEN];
	char	 rand[IDT_D_RNAD_LEN];
	char	 f_sig[IDT_F_SIG_LEN];
	char	 f_pub_key[IDT_F_PK_LEN];
	char     a_rand_sig[IDT_A_RNAD_SIG_LEN];		
	char     cloud_sig[IDT_C_SIG_LEN];		
}jl2_d_idt_t;

typedef struct {
	int	     isUsed;
	int      version;			// 协议版本
	//char     ip[MAX_IPLEN];		// IP地址
	//int      port;				// 工作端口

	char     mac[MAX_MACLEN];	// MAC地址
	char     uuid[MAX_UUIDLEN];	// ProductUUID
	char     prikey[65];        //jd 密钥
	int      lancon;			// 是否局域网可控
	int      trantype;			// 脚本类型(1:Lua, 2:Js)

	//char     feedid[MAX_FEEDIDLEN];
	//char     accesskey[33];
	//char     localkey[33];
	uint8_t pubkeyC[MAX_PKEYBIN];		// 压缩格式的公钥
	char    pubkeyS[MAX_PKEYSTR];		// 字符串格式的公钥

	//char     devdbg[MAX_DBGLEN];		// 调试信息
	char     servropt[MAX_OPTLEN];		// 服务器可选项
    char	 rand[IDT_D_RNAD_LEN];
	uint8_t sharedkey[uECC_BYTES];		// 设备的共享密钥
    jl2_d_idt_t idt;
	uint8_t sessionKey[32];				// 暂定16字节
	//uint8_t serverIP[100];
}jddevice_t;

typedef struct 
{
	char magic[6];
	char feedid[MAX_FEEDIDLEN];
	char accesskey[33];
	char localkey[33];
	char server[20];
	int port;
	char cloud_pub_key[128];
	char cloud_sig[256]; //add by wjt 20180404
	unsigned int  cloud_timestamp; 
	char is_actived;             //add 2018/05/14 设备激活状态
}jdNVargs_t;

typedef struct{
	uint8_t pubkeyC[MAX_PKEYBIN];		// 压缩格式的公钥
	uint8_t sharedkey[uECC_BYTES];		// 设备的共享密钥
}jdkey_t;

extern jddevice_t  jdDev;

#define MAX_KEYLIST  50
extern jdkey_t  jdKey[MAX_KEYLIST];

int eccContexInit(void);
int isNodeExist(jddevice_t* dev);

/*
更新在线设备列表
传入:
dev,当前发现的设备

返回:
节点是否存在
*/
int nodeUpdate(jddevice_t* dev);
void nodeClean(void);
int nodeFormatJson(uint8_t* pBuffer, int length);
char * getMac(void);
void getProfile(void);
void clr_jdArgs(void);//clear local feeid
char *read_prikey(void);
void write_prikey(char *prikey);

#endif
