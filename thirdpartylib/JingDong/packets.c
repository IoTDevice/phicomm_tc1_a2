
#include "debugPro.h"

#ifdef QCOM_4004B
#include "scw_common.h"
#else
#include "hsf.h"
#include <stdio.h>
#include <stdint.h>
#include "packets.h"
#include "debugPro.h"
#include "auth/uECC.h"
#include "auth/aes.h"
#include "auth/crc.h"
#include "nodeCache.h"
#include <string.h>
#include "Demo_Main.h"

#endif

/*
对传入的数据进行版本1的打包操作
返回:打包后的数据长度
传入:
pBuf->输出数据缓冲区,数据最大长度默认为一个MTU
enctype->传入加密类型
type->传入数据包类型
payload->传入待打包的数据
length->待打包数据的长度

*/
int packetBuildV0(uint8_t* pBuf, int enctype, int type, const uint8_t* payload, int length)
{
#ifdef QCOM_4004B
#else
	common_header_t* pCommon = (common_header_t*)pBuf;
	pCommon->magic = 0x55AA;
	pCommon->enctype = 0;	// Encryption Type

	pBuf += sizeof(common_header_t);
	cmd_header* pCmd = (cmd_header*)(pBuf);
	pCmd->type = type;
	memcpy(pCmd->cmd, "OK", 2);

	char* pData = pBuf + sizeof(cmd_header);

	char* pStrJson = (char*)payload;
	memcpy(pData, pStrJson, length);

	pCommon->len = length + sizeof(cmd_header);

	unsigned char sum = 0;
	for (unsigned int i = 0; i < pCommon->len; i++)
	{
		sum += *(pBuf + i);
	}
	pCommon->checksum = sum;
	return pCommon->len + sizeof(common_header_t);
#endif
}


int packetBuildV1(uint8_t* pBuf, int buflen, EencType enctype, EpacketType cmd, uint8_t* key, const uint8_t* payload, int length)
{
	packet_t head = {
		.magic = 0x123455BB,
		.optlen = 0,
		.payloadlen = 0,
		.version = 1,
		.type = (char)cmd,
		.total = 0,
		.index = 0,
		.enctype = (char)enctype,
		.reserved = 0,
		.crc = 0		// Todo:校验CRC
	};

	char* psJson = (char*)payload;
	uint8_t* pOut = pBuf + sizeof(packet_t);
	switch (enctype)
	{
	case ET_NOTHING:
		memcpy(pOut, psJson, length);
		pOut += length;
		head.optlen = 0;
		head.payloadlen = (uint16_t)length;
		break;
	case ET_PSKAES:
		memcpy(pOut, psJson, length);
		pOut += length;
		head.optlen = 0;
		head.payloadlen = length;
		break;
	case ET_ACCESSKEYAES:
		head.optlen = 0;
		length = device_aes_encrypt((const uint8_t *)key, 16, (const uint8_t *)(key+16), (const uint8_t*)psJson, length, (uint8_t*)pOut, length + 16);
		pOut += length;
		head.payloadlen = length;
		break;
	case ET_ECDH:
		memcpy(pOut, myKey.devPubKeyC, uECC_BYTES + 1);
		pOut += (uECC_BYTES + 1);
		head.optlen = (uECC_BYTES + 1);
		uint8_t peerPubKey[uECC_BYTES * 2];
		uECC_decompress(key, peerPubKey);
		uint8_t secret[uECC_BYTES];
#ifdef QCOM_4004B
		int ret = 0;
		ret = uECC_shared_secret(peerPubKey, myKey.priKey, secret);
		//length = device_aes_encrypt((const uint8_t *)secret, 16, (const uint8_t *)(key+4(A_UINT8*)psJson), length, pOut, length + 16);
#else
		uECC_shared_secret(peerPubKey, myKey.priKey, secret);
		length = device_aes_encrypt((const uint8_t *)secret, 16, (const uint8_t *)(key + 4), (const uint8_t *)psJson, length, pOut, length + 16);
#endif
		head.payloadlen = length;
		pOut += length;
		break;
	default:
		break;
	}

	length = pOut - pBuf;
	head.crc = CRC16(pBuf + sizeof(packet_t), length - sizeof(packet_t));
	memcpy(pBuf, &head, sizeof(head));
	return length;
}

int serverPacketBuild(uint8_t* pBuf, int buflen, EpacketType cmd, uint8_t* key, const uint8_t* payload, int payloadLen)
{
	packet_t head = {
		.magic = 0x123455CC,
		.optlen = 0,
		.payloadlen = 0,
		.version = 1,
		.type = (char)cmd,
		.total = 0,
		.index = 0,
		.enctype = (char)1,
		.reserved = 0,
		.crc = 0		// Todo:校验CRC
	};

	int len = 0;
	uint8_t* pOut = pBuf + sizeof(packet_t);
	switch (cmd)
	{
	case PT_AUTH:
		head.enctype = ET_ACCESSKEYAES;
		head.optlen = strlen(jdArgs.feedid) + 4;
		memcpy(pOut, jdArgs.feedid, strlen(jdArgs.feedid));
		pOut += strlen(jdArgs.feedid);
		memcpy(pOut, payload+4, 4);
		pOut += 4;

		len = device_aes_encrypt((const uint8_t*)key, 16, (const uint8_t*)(key + 16), (const uint8_t*)payload, payloadLen, pOut, 1024);
		head.payloadlen = len;
		pOut += len;

		break;
	default:
		head.enctype = ET_SESSIONKEYAES;
		head.optlen = 0;
		len = device_aes_encrypt((const uint8_t*)key, 16, (const uint8_t*)(key + 16), (uint8_t*)payload, payloadLen, pOut, 1024);
		head.payloadlen = len;
		pOut += len;
		break;
	}

	payloadLen = pOut - pBuf;
	head.crc = CRC16(pBuf + sizeof(packet_t), payloadLen - sizeof(packet_t));
	memcpy(pBuf, &head, sizeof(head));
	return payloadLen;
}

int serverAnalyse(packetparam_t *pParam, const uint8_t *pIn, int length, uint8_t* pOut, int maxlen)
{
	int ret = 0;
	int retLen = 0;
	uint8_t* p = NULL;
	packet_t* pPack = (packet_t*)pIn;
	if (0x123455CC != pPack->magic)
	{
		custom_log("pPack->magic=%s", pPack->magic);
		return 0;
	}
	switch (pPack->type)
	{
	case PT_AUTH:
		pParam->version = 1;
		pParam->type = pPack->type;
		p = (uint8_t*)pIn + sizeof(packet_t)+pPack->optlen;
		char key[33] = {0};
		memcpy(key, jdArgs.accesskey, strlen(jdArgs.accesskey));
		retLen = device_aes_decrypt((const uint8_t*)key, 16, (const uint8_t*)(&key[0] + 16), (const uint8_t*)p, pPack->payloadlen, p, 1024);
		if (retLen>0)
		{
			ret = retLen;
			memcpy(pOut, p, retLen);
			pPack->payloadlen = retLen;
		}
		break;
	default:
		pParam->version = 1;
		pParam->type = pPack->type;
		p = (uint8_t*)pIn + sizeof(packet_t)+pPack->optlen;
		retLen = device_aes_decrypt((const uint8_t*)jdDev.sessionKey, 16, (const uint8_t*)(&jdDev.sessionKey[0] + 16), (const uint8_t*)p, pPack->payloadlen, p, 1024);
		if (retLen > 0)
		{
			ret = retLen;
			memcpy(pOut, p, retLen);
			pPack->payloadlen = retLen;
		}
		break;
	}
	return ret;
}

int packetAnalyse(packetparam_t *pParam, const uint8_t *pIn, int length, uint8_t* pOut, int maxlen)
{
	int ret = 0;
	int retLen = 0;
	common_header_t* pCommon = (common_header_t*)pIn;
	if (0x123455bb == pCommon->magic)
	{
		packet_t* pPack = (packet_t*)pIn;
		// 计算CRC校验.
		if ((length != (pPack->optlen + pPack->payloadlen + sizeof(packet_t))) ||
			(pPack->payloadlen > UDP_MTU))
			return 0;
        if(pPack->type == 4 && pPack->enctype == 0)
			return 0;
		custom_log("pPack->enctype:%d pPack->type:%d", pPack->enctype, pPack->type);

		char key[33];
		if(pPack->type == 4)
			memcpy(key, jdArgs.localkey,strlen(jdArgs.localkey));
		else
			memcpy(key, jdArgs.accesskey,strlen(jdArgs.accesskey));
	    
		pParam->version = pPack->version;
		switch (pPack->enctype)
		{
		case ET_NOTHING:{
							pParam->type = pPack->type;
							char* pJsonStr = (char*)pIn + sizeof(packet_t)+pPack->optlen;
							retLen = pPack->payloadlen;
							memcpy(pOut, pJsonStr, retLen);
							return retLen;
							break;
		}
		case ET_ACCESSKEYAES:{
							char* pJsonStr = (char*)pIn + sizeof(packet_t)+pPack->optlen;// malloc(pPack->payloadlen);
							uint8_t retLen = device_aes_decrypt((const uint8_t*)key, 16, (const uint8_t*)(&key[0]+ 16), (const uint8_t*)(pIn + sizeof(packet_t)+pPack->optlen), pPack->payloadlen, (uint8_t*)pJsonStr, 1024);
						    pParam->type = pPack->type;
							memcpy(pOut, pJsonStr, retLen);
							return retLen;
							break;
		}
		case ET_ECDH:{
						 uint8_t devPubKey[uECC_BYTES * 2];
						 uECC_decompress(pIn + sizeof(packet_t), devPubKey);
						 uint8_t secret[uECC_BYTES];
						 ret = uECC_shared_secret(devPubKey, myKey.priKey, secret);
						 char* pJsonStr = (char*)pIn + sizeof(packet_t)+pPack->optlen;// malloc(pPack->payloadlen);
						 retLen = device_aes_decrypt((const uint8_t*)secret, 16, (const uint8_t*)(secret+4), (const uint8_t*)(pIn + sizeof(packet_t)+pPack->optlen), pPack->payloadlen, (uint8_t*)pJsonStr, 1024);
						// printf("-----after device_aes_decrypt:%d-----\n", clock_seconds());
						 pParam->type = pPack->type;
						 memcpy(pOut, pJsonStr, retLen);
						 return retLen;
						 break;
		}
		default:
			break;
		}
		return 0;
	}
	else if (0x55AA == pCommon->magic)
	{
		char* pCommonHeader = (char*)pIn + sizeof(common_header_t);
#ifdef QCOM_4004B
		int type = WAPI_POP_U32(pCommonHeader);
#else
		int type = ((cmd_header*)(pCommonHeader))->type;
#endif
		pParam->version = 0;
		switch (type)
		{
		case 1:
		case 2:
			pParam->type = PT_SCAN;
			break;
		case 3:
		case 4:
			pParam->type = PT_WRITE_ACCESSKEY;
			break;
		default:
			pParam->type = PT_UNKNOWN;
			return 0;
			break;
		}

		unsigned char sum = 0;
		unsigned int i = 0;
		for (i = 0; i < pCommon->len; i++)
		{
			sum += *(pCommonHeader + i);
		}
		if (pCommon->checksum != sum)
		{
			pParam->type = PT_UNKNOWN;
			return 0;
		}
		char* pJsonStr = pCommonHeader + sizeof(cmd_header);
		retLen = strlen(pJsonStr);
		memcpy(pOut, pJsonStr, retLen);
		return retLen;
	}
	return 0;
}


