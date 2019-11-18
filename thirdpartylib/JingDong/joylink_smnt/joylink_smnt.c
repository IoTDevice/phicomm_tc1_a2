/*************************************

Copyright (c) 2015-2050, JD Smart All rights reserved. 

*************************************/
#include "hsf.h"
#include "joylink_smnt.h"
#include "joylink_config.h"
#include "hfsmtlk.h"
#include "smartlinkv7.h"
#include <stdio.h>
#include <string.h>
#include "aes.h"


#define STEP_MULTICAST_HOLD_CHANNEL				5
#define STEP_BROADCAST_HOLD_CHANNEL				4
#define STEP_BROADCAST_ERROR_HOLE_CHANNEL		2
#define PAYLOAD_MIN								(3)
#define PAYLOAD_MAX								(48+1)				
#define printf_high u_printf
static hftimer_handle_t timeout_timer=NULL;
static hftimer_handle_t smtlk_timer=NULL;

#define TIMEOUT_TIMER_ID		(55)
#define SMTLK_TIMER_ID		    (56)

static int time_count = 0;



typedef struct{
	unsigned char type;	                        // 0:NotReady, 1:ControlPacketOK, 2:BroadcastOK, 3:MulticastOK
	unsigned char encData[1+32+32+32];            // length + EncodeData
}smnt_encrypt_data_t;

typedef enum{
	SMART_CH_INIT 		=	0x1,
	SMART_CH_LOCKING 	=	0x2,
	SMART_CH_LOCKED 	=	0x4,
	SMART_FINISH 		= 	0x8
}smnt_status_t;


typedef struct{
	smnt_status_t state;	
	uint16 syncFirst;				
	uint8 syncCount;				
	uint8 syncStepPoint;			
	uint8 syncFirst_downlink;		
	uint8 syncCount_downlink;		
	uint8 syncStepPoint_downlink;	
	uint8 directTimerSkip;			
	uint8 broadcastVersion;			
	uint8 muticastVersion;			
	uint8  broadIndex;				
	uint8  broadBuffer[5];			
	uint16 lastLength;			
	uint16 lastUploadSeq;		
	uint16 lastDownSeq;		
	uint8  syncAppMac[6];		
	uint8  syncBssid[6];		
	uint16 syncIsUplink;		
	uint8 chCurrentIndex;		
	uint8 chCurrentProbability;	
	uint8 isProbeReceived;	
	uint8 payload_multicast[128];	
	uint8 payload_broadcast[128];
	smnt_encrypt_data_t result;	
}joylinkSmnt_t;

joylinkSmnt_t* pSmnt = NULL;
joylink_smnt_param_t	joylink_smnt_gobal;
static int  joylink_smnt_payLoadcheck(uint8 *payload);
static void joylink_smnt_muticastadd(uint8* pAddr);
static void joylink_smnt_broadcastadd(int ascii);
static uint8 joylink_smnt_crc(uint8 *ptr, uint8 len);
static void joylink_smnt_bufferprintf(uint8* p, int split, int len);




 void joylink_smnt_init(joylink_smnt_param_t param)
{
	u_printf("+++++++++++++SMNT INIT %s+++++++++++++++++",VERSION_SMNT);
        
    pSmnt = (joylinkSmnt_t *)malloc(sizeof(joylinkSmnt_t));
	
    memset(pSmnt, 0, sizeof(joylinkSmnt_t) );
	memset(pSmnt->payload_multicast, 0xFF, sizeof(pSmnt->payload_multicast));	
	memset(pSmnt->payload_broadcast, 0xFF, sizeof(pSmnt->payload_broadcast));
	
	memcpy(joylink_smnt_gobal.secretkey,param.secretkey,16);
	
	if(param.get_result_callback != NULL){
		joylink_smnt_gobal.get_result_callback = param.get_result_callback;
	}else{
		u_printf("get_result_callback is NULL\n");
	}

	if(param.switch_channel_callback != NULL){
		joylink_smnt_gobal.switch_channel_callback = param.switch_channel_callback;
	}else{
		u_printf("switch_channel_callback is NULL\n");
	}
	return;
}

 void joylink_smnt_release(void)
{
	memset(joylink_smnt_gobal.secretkey,0,sizeof(joylink_smnt_gobal.secretkey));
	joylink_smnt_gobal.get_result_callback 		= NULL;
	joylink_smnt_gobal.switch_channel_callback 	= NULL;

	if(pSmnt != NULL){
		free(pSmnt);
		pSmnt = NULL;
	}
}

void joylink_smnt_reset(void)
{
	if(pSmnt != NULL)
	{
		memset(pSmnt,0,sizeof(joylinkSmnt_t));
	}
}

 static void  joylink_smnt_finish(void)
{
	int ret = 128;
	uint8 iv[16] = {0};
	
	joylink_smnt_result_t smnt_result;
	memset(&smnt_result,0,sizeof(smnt_result));
	if (pSmnt && (pSmnt->state== SMART_FINISH) ){

		joylink_smnt_bufferprintf(pSmnt->result.encData, 1, pSmnt->result.encData[0]);
		memset(&smnt_result,0,sizeof(smnt_result));

		ret = device_aes_decrypt((const uint8_t *)joylink_smnt_gobal.secretkey,16,(const uint8_t *)iv,pSmnt->result.encData+1,pSmnt->result.encData[0],pSmnt->result.encData+1,128);
	
		if((ret> 0) && (ret <= 96)){
			smnt_result.jd_password_len = pSmnt->result.encData[1];
			smnt_result.jd_ssid_len	 = ret - smnt_result.jd_password_len -1 -6;
			memcpy(smnt_result.jd_password,pSmnt->result.encData+2,smnt_result.jd_password_len);
			memcpy(smnt_result.jd_ssid,pSmnt->result.encData + 2 + smnt_result.jd_password_len +6,smnt_result.jd_ssid_len);	

			smnt_result.smnt_result_status = smnt_result_ok;
		}else{
		    pSmnt->state = SMART_CH_LOCKING;
			smnt_result.smnt_result_status = smnt_result_decrypt_error;
			u_printf("smnt_result.smnt_result_status = smnt_result_decrypt_error\r\n");
		}
		
		if(joylink_smnt_gobal.get_result_callback == NULL){
			u_printf("ERROR:joylink_smnt_finish->get_result_callback NULL\n");
			goto RET;
		}
		
		joylink_smnt_gobal.get_result_callback(smnt_result);
	}
RET:
	return;
}


 void USER_FUNC joylink_smnt_cyclecall(hftimer_handle_t htimer)
{
	if(pSmnt == NULL)
	{
    	return ;
  	}

	if(joylink_smnt_gobal.switch_channel_callback == NULL)
	{
		u_printf("switch channel function NULL\n");
		return ;
	}
	
    if (pSmnt->directTimerSkip)
	{
		pSmnt->directTimerSkip--;
		return ;
	}
	
	if (pSmnt->state == SMART_FINISH)
	{
		u_printf("-------------------->Finished\n");
		pSmnt->directTimerSkip = 10000/50;
		return ;
	}
	
	if (pSmnt->isProbeReceived >0 )
	{
		u_printf("-------------------->Probe Stay(CH:%d) %d\n", pSmnt->chCurrentIndex+1, pSmnt->isProbeReceived);
		pSmnt->isProbeReceived = 0;
		pSmnt->directTimerSkip = 5000 / 50;
		return ;
	}
	
	if (pSmnt->chCurrentProbability > 0)
	{
		pSmnt->chCurrentProbability--;
		u_printf("------------------->SYNC (CH:%d) %d\n", pSmnt->chCurrentIndex+1, pSmnt->chCurrentProbability);
		return ;
	}

	pSmnt->chCurrentIndex = (pSmnt->chCurrentIndex + 1) % 13;
	
	if(joylink_smnt_gobal.switch_channel_callback != NULL)
	{
		joylink_smnt_gobal.switch_channel_callback(pSmnt->chCurrentIndex+1);
	}
	
	pSmnt->state 			= SMART_CH_LOCKING;
	pSmnt->syncStepPoint = 0;
	pSmnt->syncCount = 0;
	pSmnt->chCurrentProbability = 0;
//	printf("CH=%d, T=%d\n", pSmnt->chCurrentIndex, 50);
	return ;
}

 static void  joylink_smnt_broadcastadd(int ascii)
{
	uint8 isFlag = (uint8)((ascii >> 8) & 0x1);
	uint8 is_finishpacket = 0;

	uint8 *broadbuffer=pSmnt->broadBuffer,*broadindex = &(pSmnt->broadIndex);
	if (isFlag){
		*broadindex = 0;
		*broadbuffer = (uint8)ascii;
	}else{
		*broadindex = *broadindex + 1;
		broadbuffer[*broadindex] = (uint8)ascii;

		if((((pSmnt->payload_broadcast[1] +2) / 4 + 1) == ( (*broadbuffer) >> 3))  && (pSmnt->payload_broadcast[1] != 0) && (pSmnt->payload_broadcast[1] != 0xFF))
		{
			if(*broadindex == 2){
				is_finishpacket = 1;
				*(broadbuffer + 3) = 0;
				*(broadbuffer + 4) = 0;
			}
		}
	
		if (*broadindex >= 4 || is_finishpacket)
		{
			*broadindex = 0;
			uint8 crc = (*broadbuffer) & 0x7;
			uint8 index = (*broadbuffer) >> 3;
			uint8 rCrc = joylink_smnt_crc(broadbuffer + 1, 4) & 0x7;
			
			/*not to check the last pacet crc,It is a patch for the last packet is a lot wrong which maybe leaded by the phone*/
			if (((index>0) && (index<33) && (rCrc == crc)))
			{
				memcpy(pSmnt->payload_broadcast + (index - 1) * 4, broadbuffer + 1, 4);
					
				u_printf("B(%x=%x)--%02x,%02x,%02x,%02x\n", index, broadbuffer[0], broadbuffer[1], broadbuffer[2], broadbuffer[3], broadbuffer[4]);
				index = joylink_smnt_payLoadcheck(pSmnt->payload_broadcast);

				if (pSmnt->chCurrentProbability < 30){ 
					pSmnt->chCurrentProbability += STEP_BROADCAST_HOLD_CHANNEL;
				}
				
				if (index == 0){	
					pSmnt->result.type = 2;
				}
				
				//joylink_smnt_bufferprintf(pSmnt->payload, 1, 80);
				
			}else{
				if (pSmnt->chCurrentProbability < 30){ 
					pSmnt->chCurrentProbability += STEP_BROADCAST_ERROR_HOLE_CHANNEL;
				}
			}
		}else if (*broadindex == 2){
			uint8 index = broadbuffer[0] >> 3;
			if (index == 0){
				*broadindex = 0;
				uint8 crc = broadbuffer[0] & 0x7;
				uint8 rCrc = joylink_smnt_crc(broadbuffer + 1, 2) & 0x7;
				if (rCrc == crc){
					pSmnt->broadcastVersion = broadbuffer[1];
					//u_printf("Version RX:%x\n",pSmnt->broadcastVersion);
				}
			}
		}
	} 
}

/*
Input: Muticast Addr
Output: -1:Unkown Packet, 0:Parse OK, 1:Normal Process
*/
 static void joylink_smnt_muticastadd(uint8* pAddr)
{
	int8 index = 0;
	
	if ((pAddr[3] >> 6) == ((pAddr[4] ^ pAddr[5]) & 0x1)){
		index = pAddr[3] & 0x3F;
	}else
		return;
	
	if ((index >= 1) && (index < PAYLOAD_MAX))		//avoid overstep leaded by error
	{
		uint8 payloadIndex = index - 1;
		if (payloadIndex > 64)
			return;

		if (pSmnt->chCurrentProbability < 20) 
			pSmnt->chCurrentProbability += STEP_MULTICAST_HOLD_CHANNEL;			// Delay CH switch!
		
		//u_printf("M%02d(CH=%d)--%02X:(%02X,%02X)\n", index, pSmnt->chCurrentIndex+1, pAddr[3], pAddr[4], pAddr[5]); 
		pSmnt->payload_multicast[payloadIndex * 2]     = pAddr[4];
		pSmnt->payload_multicast[payloadIndex * 2 + 1] = pAddr[5];

		if (joylink_smnt_payLoadcheck(pSmnt->payload_multicast) == 0){
			pSmnt->result.type = 3;
			return;
		}
	}
	return;
}

 void joylink_smnt_datahandler(PHEADER_802_11 pHeader, int length)
{
	uint8 isUplink = 1;				
	uint8 packetType = 0;					// 1-multicast packets 2-broadcast packets 0-thers
	uint8 isDifferentAddr = 0;
	uint8 *pDest, *pSrc, *pBssid;
	uint16 lastLength = 0;
	uint16 lastSeq_uplink = 0,lastSeq_downlink = 0;
	static uint8 past_channel = 0xFF;
	
	if (pSmnt == NULL)
		return;
	if ((length > 100) && (pSmnt->state != SMART_CH_LOCKED))	
		return;
	
	if (pHeader->FC.ToDs){
		isUplink = 1;				
		pBssid = pHeader->Addr1;
		pSrc = pHeader->Addr2;
		pDest = pHeader->Addr3;	

		if (!((memcmp(pDest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) == 0) || (memcmp(pDest, "\x01\x00\x5E", 3) == 0))){
			return;
		}		
		lastSeq_uplink = pSmnt->lastUploadSeq;
		pSmnt->lastUploadSeq = pHeader->Sequence;
	}else{
		pDest = pHeader->Addr1;	
		pBssid = pHeader->Addr2;
		pSrc  = pHeader->Addr3;
		
		isUplink = 0;
		//not broadcast nor multicast package ,return
		if (!((memcmp(pDest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) == 0) || (memcmp(pDest, "\x01\x00\x5E", 3) == 0))){
			return;
		}
		lastSeq_downlink = pSmnt->lastDownSeq;
		pSmnt->lastDownSeq = pHeader->Sequence;
	}
	lastLength = pSmnt->lastLength;
	pSmnt->lastLength = length;

	if (memcmp(pDest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) == 0)
	{
		if (pSmnt->state == SMART_CH_LOCKING)
		{
			if(isUplink == 1)

				{}
			else
				{}
			
		}
		packetType = 2;
	}
	else if (memcmp(pDest, "\x01\x00\x5E", 3) == 0)
	{
		if (pSmnt->state == SMART_CH_LOCKING)
			u_printf("(%02x-%04d):%02x:%02x:%02x->%d\n", *((uint8*)pHeader) & 0xFF, pHeader->Sequence, pDest[3], pDest[4], pDest[5], (uint8)length);
		packetType = 1;
	}

	if (memcmp(pSrc, pSmnt->syncAppMac, 6) != 0)
	{
		isDifferentAddr = 1;
	}

	if(pSmnt->state == SMART_CH_LOCKING)
	{
		if (packetType == 0) return;
		if ((isUplink==1) && (pHeader->Sequence == lastSeq_uplink)) return;
		if ((isUplink==0) && (pHeader->Sequence == lastSeq_downlink)) return;
		if(!isDifferentAddr)
		{
			if (packetType != 0)
			{
				if (packetType == 1)
				{
					if (((pDest[3] >> 6) == ((pDest[4] ^ pDest[5]) & 0x1)) && (pDest[3] != 0) && ((pDest[3]&0x3F) <=  PAYLOAD_MAX))
					{
						/*if receive multicast right message for two times,lock the channel*/
						if(past_channel == pSmnt->chCurrentIndex + 1)
						{
							past_channel = 0xFF;
							if (pSmnt->chCurrentProbability < 20) 
								pSmnt->chCurrentProbability = 10;

							memcpy(pSmnt->syncBssid, pBssid, 6);
							pSmnt->state = SMART_CH_LOCKED;
							hfsmtlk_lock_channel(pSmnt->chCurrentIndex+1,pSmnt->syncBssid);
						}
						else
						{
							past_channel = pSmnt->chCurrentIndex + 1;
						}

					}
					joylink_smnt_muticastadd(pDest); // Internal state machine could delay the ch switching
					return;
				}

				if(isUplink == 1)
				{
					if (lastLength == length)	return;

					int expectLength = 1 + pSmnt->syncFirst + pSmnt->syncCount%4 - (pSmnt->syncStepPoint?4:0);
					int isStep = (pSmnt->syncStepPoint == 0 && length == (expectLength - 4));	

					if ( ( length == expectLength ) || isStep)
					{
						pSmnt->syncCount++;
						pSmnt->chCurrentProbability++;

						if (isStep)		pSmnt->syncStepPoint = pSmnt->syncCount;

						if (pSmnt->syncCount >= 3)	// Achive SYNC count!
						//if (pSmnt->syncCount >= 4)	// Achive SYNC count!
						{
							pSmnt->syncFirst = pSmnt->syncFirst + pSmnt->syncStepPoint - (pSmnt->syncStepPoint ? 4 : 0);	// Save sync world
							memcpy(pSmnt->syncBssid, pBssid, 6);

							pSmnt->state 		= SMART_CH_LOCKED;
                            hfsmtlk_lock_channel(pSmnt->chCurrentIndex+1,pSmnt->syncBssid);
							/*u_printf("SYNC:(%02X%02X%02X%02X%02X%02X-%02X%02X%02X%02X%02X%02X)------->:CH=%d, WD=%d\n",
								pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4], pSrc[5],
								pBssid[0], pBssid[1], pBssid[2], pBssid[3], pBssid[4], pBssid[5],
								pSmnt->chCurrentIndex+1, pSmnt->syncFirst);*/
							
							pSmnt->syncIsUplink = isUplink;
							
							if(pSmnt->chCurrentProbability < 20)
								pSmnt->chCurrentProbability = 20;
							//u_printf("--->locked by uplink\n");
						}
						return;
					}
					if (pSmnt->syncCount)
					{
						pSmnt->syncStepPoint = 0;
						pSmnt->syncCount = 0;
						memcpy(pSmnt->syncAppMac, pSrc, 6);
						pSmnt->syncFirst = length;
						//u_printf("SYNC LOST\n");
					}
				}
				else
				{
					if (lastLength == length)	return;

					int expectLength = 1 + pSmnt->syncFirst_downlink+ pSmnt->syncCount_downlink%4 - (pSmnt->syncStepPoint_downlink?4:0);
					int isStep = (pSmnt->syncStepPoint_downlink == 0 && length == (expectLength - 4));	

					if ( ( length == expectLength ) || isStep)
					{
						pSmnt->syncCount_downlink++;
						pSmnt->chCurrentProbability++;
						
						if (isStep)			pSmnt->syncStepPoint_downlink = pSmnt->syncCount_downlink;

						if (pSmnt->syncCount_downlink>= 3)	// Achive SYNC count!
						//if (pSmnt->syncCount_downlink>= 4)	// Achive SYNC count!
						{
							pSmnt->syncFirst_downlink = pSmnt->syncFirst_downlink + pSmnt->syncStepPoint_downlink - (pSmnt->syncStepPoint_downlink? 4 : 0);	// Save sync world
							memcpy(pSmnt->syncBssid, pBssid, 6);
							pSmnt->state = SMART_CH_LOCKED;
							//u_printf("SYNC:(%02X%02X%02X%02X%02X%02X-%02X%02X%02X%02X%02X%02X)------->:CH=%d, WD=%d\n",
								//pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4], pSrc[5],
								//pBssid[0], pBssid[1], pBssid[2], pBssid[3], pBssid[4], pBssid[5],
								//pSmnt->chCurrentIndex+1, pSmnt->syncFirst_downlink);
                            hfsmtlk_lock_channel(pSmnt->chCurrentIndex+1,pSmnt->syncBssid);
							pSmnt->syncIsUplink = isUplink;
							if(pSmnt->chCurrentProbability < 20)
								pSmnt->chCurrentProbability = 20;
							u_printf("--->locked by downlink\n");
						}
						return;
					}
					pSmnt->syncStepPoint_downlink = 0;
					pSmnt->syncCount_downlink = 0;
					memcpy(pSmnt->syncAppMac, pSrc, 6);
					pSmnt->syncFirst_downlink = length;
					u_printf("SYNC LOST\n");
				}
			} 
			return;	
		}
		memcpy(pSmnt->syncAppMac, pSrc, 6);
		pSmnt->syncFirst = length;
		pSmnt->syncFirst_downlink = length;
		u_printf("Try to SYNC!\n");
		return;
	}
	else if (pSmnt->state == SMART_CH_LOCKED)
	{
		if (isDifferentAddr) return;

		if (packetType == 1)
		{
			joylink_smnt_muticastadd(pDest);
			return;
		}
		
		if ( (packetType != 1)&&(memcmp(pDest, pSmnt->syncBssid, 6) != 0) ){
			packetType = 2;
		}
		
		if(isUplink == 1)
		{
			if (length < (pSmnt->syncFirst - 1)) return;
			if ( pHeader->Sequence == lastSeq_uplink) return;
		}
		else
		{
			if (length < (pSmnt->syncFirst_downlink - 1)) return;
			if ( pHeader->Sequence == lastSeq_downlink) return;
		}
		if (packetType == 2)
		{
			int ascii;
			if(isUplink == 1)
				ascii = length - pSmnt->syncFirst + 1;
			else
				ascii = length - pSmnt->syncFirst_downlink + 1;

			if((((ascii >> 8) & 0x01) == 1) && (((ascii >> 3)&0x1F) == 0))
			{
				;
			}
			else
			{
				joylink_smnt_broadcastadd(ascii);
			}
			if (((length + 4 - lastLength) % 4 == 1)&& (length - pSmnt->syncFirst)<4)  // There are SYNC packets even ch locked.
			{
				if (pSmnt->chCurrentProbability < 20) pSmnt->chCurrentProbability++;
			}
		}
	}
	else if (pSmnt->state == SMART_FINISH)
	{
		u_printf("SMART_FINISH-1\n");
	}
	else
	{
		pSmnt->state = SMART_CH_LOCKING;
		memcpy(pSmnt->syncAppMac, pSrc, 6);
		pSmnt->syncFirst = length;
		pSmnt->syncStepPoint = 0;
		pSmnt->syncCount = 0;
		u_printf("Reset All State\n");
	}
	return;
}

 static int joylink_smnt_payLoadcheck(uint8 *payload)
{
	uint8 crc = joylink_smnt_crc(payload + 1, payload[1]+1);
	if ((payload[1] > (PAYLOAD_MIN*2)) &&
		(payload[1] < (PAYLOAD_MAX*2)) &&
        (payload[0] == crc)){

		smnt_encrypt_data_t* pRet = &pSmnt->result;
		memcpy(pRet->encData, payload+1, payload[1]+1);
		pRet->type = pSmnt->result.type;
          pSmnt->state = SMART_FINISH;
          joylink_smnt_finish();
        return 0;
	}
	return 1;
}


 static uint8 joylink_smnt_crc(uint8 *ptr, uint8 len)
{
	unsigned char crc;
	unsigned char i;
	crc = 0;
	while (len--)
	{
		crc ^= *ptr++;
		for (i = 0; i < 8; i++)
		{
			if (crc & 0x01)
			{
				crc = (crc >> 1) ^ 0x8C;
			}
			else
				crc >>= 1;
		}
	}
	return crc;
}
 static void joylink_smnt_bufferprintf(uint8* p, int split, int len)
{
	int i;
	char buf[512];
	int index = 0;
	for (i = 0; i < len; i++)
	{
		if (split != 0 && ((i + 1) % split) == 0)
		{
			index += sprintf(buf + index, "%02x,", p[i]);
		}
		else
			index += sprintf(buf + index, "%02x ", p[i]);
	}
	u_printf("Len=%d:%s\n",len, buf);
}
/*********hiflying add below**************/

#define JOYLINK_SCANNING 0
#define JOYLINK_CHLLOCKED 1
#define JOYLINK__FINISHED 2
//static int joylink_smnt_mode = JOYLINK_SCANNING;





void adp_changeCh(unsigned char i)
{
	//hfsmtlk_set_sniffer_channel(i,0);
	rda5981_start_sniffer(i,1,0,0,0xFFFF);
}

joylink_smnt_result_t my_ssid_key_info;
void get_result_callback_function(joylink_smnt_result_t ssid_key_info)
{
	 my_ssid_key_info=ssid_key_info;
}
void joylink_config_init(void)
{
	joylink_smnt_param_t pDataBuf;
	my_ssid_key_info.smnt_result_status=smnt_result_decrypt_error;
	pDataBuf.switch_channel_callback=&adp_changeCh;
	pDataBuf.get_result_callback=&get_result_callback_function;
	memcpy(pDataBuf.secretkey,SECRETKEY,16);
	joylink_smnt_init(pDataBuf);
	return ;
}
 static int joylink_smnt_recv_callback( void *buffer, uint32_t size, uint8_t channel) 
{ 
	joylink_smnt_datahandler((PHEADER_802_11)buffer, size);
	if(my_ssid_key_info.smnt_result_status == smnt_result_ok)
	{
	    printf_high("----------------joylink smnt sucess----------------\r\n");
		 char connect_ssid[33]={0};
		 char connect_key[64]={0};
		 //char connect_ssid_len = 0;
		 char connect_key_len = 0;
		strcpy(connect_ssid,my_ssid_key_info.jd_ssid);
		strcpy(connect_key,my_ssid_key_info.jd_password);
		//connect_ssid_len=my_ssid_key_info.jd_ssid_len;
		connect_key_len=my_ssid_key_info.jd_password_len;
#ifdef IS_FULL_LOG			
		printf_high("get connect_ssid[%d]: %s\n",connect_ssid_len,connect_ssid);
		printf_high("get connect_key[%d]: %s\n",connect_key_len,connect_key);
		printf_high("get router bssid: %02X%02X%02X%02X%02X%02X\n",pSmnt->syncBssid[0],pSmnt->syncBssid[1],pSmnt->syncBssid[2],pSmnt->syncBssid[3],pSmnt->syncBssid[4],pSmnt->syncBssid[5]);
#endif
		hfsmtlk_finished_ok(connect_key,connect_key_len,2,pSmnt->syncBssid,connect_ssid);
	}
	return 0;//don't do HF smartlink
}
 void USER_FUNC timeout_timer_callback( hftimer_handle_t htimer )
 {
	 if(hftimer_get_timer_id(htimer)==TIMEOUT_TIMER_ID)
	 {
		 time_count++;
		 if(time_count>=120)// 1minute
		 {
			 printf_high("~~~~~smartlink time out reset!!!~~~~~~\n");
			 hfsys_reset();
		 }
	 }		 
 }
USER_FUNC static void joylink_thread_func(void* arg)
{
    joylink_config_init();
	if((smtlk_timer = hftimer_create("SMTLK_time",100,true,SMTLK_TIMER_ID,joylink_smnt_cyclecall,1))==NULL)
	{
		u_printf("create timer fail\n");
	}
    if((timeout_timer = hftimer_create("timeout_time",1000,true,TIMEOUT_TIMER_ID,timeout_timer_callback,0))==NULL)
	{
		u_printf("create timer fail\n");
	}
	hfsmtlk_register(joylink_smnt_recv_callback);
}


void start_joylink_smnt_config(void)
{
	printf_high("start joylink smnt config!!!\n");
	hfthread_create(joylink_thread_func,"joylink_fun",2048,(void*)1,1,NULL,NULL);
}
/*********end 20170216**************/


