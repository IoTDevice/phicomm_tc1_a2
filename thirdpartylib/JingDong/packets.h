#ifndef _PACKETS_H
#define _PACKETS_H
#include "debugPro.h"

#ifdef QCOM_4004B
#else
#include <stdint.h>

#include "jdsmart.h"
#endif

#pragma pack(1)
typedef struct {
	unsigned int magic;
	unsigned int len;
	unsigned int enctype;
	unsigned char checksum;
} common_header_t;

typedef struct {
	unsigned int type;
	unsigned char cmd[2];
} cmd_header;


////////////////////////以下代码用于新版局域网协议///////////////////

typedef struct {
	unsigned int	magic;
	unsigned short	optlen;
	unsigned short	payloadlen;

	unsigned char	version;	// 版本号从1开始
	unsigned char	type;		// {  1:搜索,  2:授权,  3:Json交互, 4:Bin透传}
	unsigned char	total;
	unsigned char	index;

	unsigned char	enctype;	// (0:不加密, 1:静态AES密钥, 2:ECDH协商密钥, 3:Accesskey密钥)
	unsigned char	reserved;
	unsigned short	crc;
}packet_t;
#pragma pack()

typedef enum {		// 兼容Android
	PT_UNKNOWN = 0,
	PT_SCAN=1,
	PT_WRITE_ACCESSKEY=2,
	PT_JSONCONTROL=3,
	PT_SCRIPTCONTROL=4,

	PT_OTA_ORDER =      7,
	PT_OTA_UPLOAD =     8,
	PT_AUTH = 9,
	PT_BEAT = 10,
	PT_SERVERCONTROL = 11,
	PT_UPLOAD = 12
}EpacketType;

typedef enum {		// 加密类型
	ET_NOTHING = 0,
	ET_PSKAES = 1,
	ET_ECDH = 2,
	ET_ACCESSKEYAES = 3,
	ET_SESSIONKEYAES = 4
}EencType;
typedef struct {
	int				version;	// 0:旧版协议, 1:新版带局域网控制
	EpacketType		type;
}packetparam_t;

#define PACKET_SIZE UDP_MTU
typedef struct bytebuffer{
	int pos;
	int len;
	int size;
	uint8_t data[PACKET_SIZE];
}bytebuffer;




typedef struct {
	unsigned int	 timestamp; // 设备时间
	unsigned int	random_unm; // 设备生产的随机数字
}auth_t;

typedef struct {
	unsigned int	 timestamp; // 服务器时间
	unsigned int	 random_unm; // 云端生产的随机数字
	unsigned char	session_key[];
}auth_resp_t;

typedef struct {
	unsigned int timestamp; //设备时间
	unsigned short	verion; // 固件版本
	unsigned short	rssi;
}heartbeat_t;

typedef struct {
	unsigned int	 timestamp;
	unsigned int	 code;
}heartbeat_resp_t;


typedef struct {
	unsigned int timestamp;
	unsigned int biz_code;
	unsigned int	 serial;
	unsigned char	cmd[]; //脚本解析后的数据
}control_t;

typedef struct {
	unsigned int timestamp;
	unsigned int biz_code;
	unsigned int	 serial;
	unsigned char resp_length[4];
	unsigned char	resp[1]; // 设备控制响应结果
	unsigned char	streams[1]; // 设备当前属性状态（快照） 
}control_resp_t;

typedef struct {
	unsigned int timestamp;
	unsigned char	data[18];	//需要脚本解析的数据,变长
}dataupload_t;






/*
分析接收到的UDP数据包,进行解码
返回:接收到数据包解码后明文的长度

传入:
pParam->返回数据包的描述参数
pIn->传入待分析的数据包
length->待分析数据包的长度
pOut->存放解密后的明文的空间
maxlen->传入空间的大小
*/
int packetAnalyse(packetparam_t *pParam, const uint8_t *pIn, int length, uint8_t* pOut, int maxlen);

int packetBuildV0(uint8_t* pBuf, int enctype, int type, const uint8_t* payload, int length);
int packetBuildV1(uint8_t* pBuf, int buflen, EencType enctype, EpacketType cmd, uint8_t* key, const uint8_t* payload, int length);

int serverPacketBuild(uint8_t* pBuf, int buflen, EpacketType cmd, uint8_t* key, const uint8_t* payload, int payloadLen);
int serverAnalyse(packetparam_t *pParam, const uint8_t *pIn, int length, uint8_t* pOut, int maxlen);





#endif
