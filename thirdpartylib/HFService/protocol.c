#include "hsf.h"
#include <string.h>
#include "custom_util.h"
#include "protocol.h"

/*PROJECT_ID:NW2398*/

uint8 *dataOut=NULL;
int dataOut_size=0;
int outLen;
int device_updated;

struct _FEILD_
{
	int idx;
	int len;
	int pVal;
};

struct _MENU_STRUCT_
{
	int stepNumPos;
	int menuIdPos;
	int stepPos;
} menuStruct;

typedef struct _KEYS_UP_ StrUpKeys;
struct _KEYS_UP_
{
	uint8 SN_POWER;
	uint8 SN_CHILD_SAFETY_LOCK;
	uint8 UV_LIGHT;
	uint8 SN_NEGATIVE_ION;
	uint8 SN_LED_LIGHT;
	uint8 BEEP;
	uint8 SN_RESET_FILTER_MESH;
	uint8 FILTER_WARN;
	uint8 SN_MODE;
	uint16 LEFT_TIMER_OFF;
	uint8 SN_FANSPEED;
	uint16 SN_PM25;
	uint16 SN_VOC;
	uint16 SN_CO2;
	uint16 SN_FILTER_MESH_HOURS;
	uint8 ERRORCODE;
};
//char *upKeys[]={"SN_POWER","SN_CHILD_SAFETY_LOCK","UV_LIGHT","SN_NEGATIVE_ION","SN_LED_LIGHT","BEEP","SN_RESET_FILTER_MESH","FILTER_WARN","SN_MODE","LEFT_TIMER_OFF","SN_FANSPEED","SN_PM25","SN_VOC","SN_CO2","SN_FILTER_MESH_HOURS","ERRORCODE"};
StrUpKeys strUpKeysNow,strUpKeysDirty;

typedef struct _KEYS_DOWN_ StrDownKeys;
struct _KEYS_DOWN_
{
	uint8 SN_POWER;
	uint8 SN_CHILD_SAFETY_LOCK;
	uint8 UV_LIGHT;
	uint8 SN_NEGATIVE_ION;
	uint8 SN_LED_LIGHT;
	uint8 BEEP;
	uint8 SN_RESET_FILTER_MESH;
	uint8 SN_MODE;
	uint16 TIMER_OFF;
	uint8 SN_FANSPEED;
};
//char *downKeys[]={"SN_POWER","SN_CHILD_SAFETY_LOCK","UV_LIGHT","SN_NEGATIVE_ION","SN_LED_LIGHT","BEEP","SN_RESET_FILTER_MESH","SN_MODE","TIMER_OFF","SN_FANSPEED"};
StrDownKeys strDownKeysNow,strDownKeysDirty;

void structUpSynch()
{
	// TODO:To synchronize upstream struct to downstream struct
	int size=(sizeof(StrUpKeys)>sizeof(StrDownKeys))?sizeof(StrDownKeys):sizeof(StrUpKeys);
	memcpy(&strDownKeysNow,&strUpKeysNow,size);
}

// upstream code
void up_key_SN_POWER(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=1;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_POWER=getBits(data,pos,0x1,0);
	putByte(dataOut+outLen,strUpKeysDirty.SN_POWER);
	outLen+=1;
}
void up_key_SN_CHILD_SAFETY_LOCK(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=2;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_CHILD_SAFETY_LOCK=getBits(data,pos,0x2,1);
	putByte(dataOut+outLen,strUpKeysDirty.SN_CHILD_SAFETY_LOCK);
	outLen+=1;
}
void up_key_UV_LIGHT(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=3;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.UV_LIGHT=getBits(data,pos,0x4,2);
	putByte(dataOut+outLen,strUpKeysDirty.UV_LIGHT);
	outLen+=1;
}
void up_key_SN_NEGATIVE_ION(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=4;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_NEGATIVE_ION=getBits(data,pos,0x8,3);
	putByte(dataOut+outLen,strUpKeysDirty.SN_NEGATIVE_ION);
	outLen+=1;
}
void up_key_SN_LED_LIGHT(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=5;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_LED_LIGHT=getBits(data,pos,0x10,4);
	putByte(dataOut+outLen,strUpKeysDirty.SN_LED_LIGHT);
	outLen+=1;
}
void up_key_BEEP(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=6;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.BEEP=getBits(data,pos,0x20,5);
	putByte(dataOut+outLen,strUpKeysDirty.BEEP);
	outLen+=1;
}
void up_key_SN_RESET_FILTER_MESH(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=7;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_RESET_FILTER_MESH=getBits(data,pos,0x40,6);
	putByte(dataOut+outLen,strUpKeysDirty.SN_RESET_FILTER_MESH);
	outLen+=1;
}
void up_key_FILTER_WARN(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=8;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.FILTER_WARN=getBits(data,pos,0x80,7);
	putByte(dataOut+outLen,strUpKeysDirty.FILTER_WARN);
	outLen+=1;
}
void up_key_SN_MODE(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=9;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_MODE=getByte(data,pos);
	putByte(dataOut+outLen,strUpKeysDirty.SN_MODE);
	outLen+=1;
}
void up_key_LEFT_TIMER_OFF(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=10;
	dataOut[outLen++]=2;
	if (data!=NULL)
		strUpKeysDirty.LEFT_TIMER_OFF=getWord(data,pos);
	putWord(dataOut+outLen,strUpKeysDirty.LEFT_TIMER_OFF);
	outLen+=2;
}
void up_key_SN_FANSPEED(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=11;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.SN_FANSPEED=getByte(data,pos);
	putByte(dataOut+outLen,strUpKeysDirty.SN_FANSPEED);
	outLen+=1;
}
void up_key_SN_PM25(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=12;
	dataOut[outLen++]=2;
	if (data!=NULL)
		strUpKeysDirty.SN_PM25=getWord(data,pos);
	putWord(dataOut+outLen,strUpKeysDirty.SN_PM25);
	outLen+=2;
}
void up_key_SN_VOC(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=13;
	dataOut[outLen++]=2;
	if (data!=NULL)
		strUpKeysDirty.SN_VOC=getWord(data,pos);
	putWord(dataOut+outLen,strUpKeysDirty.SN_VOC);
	outLen+=2;
}
void up_key_SN_CO2(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=14;
	dataOut[outLen++]=2;
	if (data!=NULL)
		strUpKeysDirty.SN_CO2=getWord(data,pos);
	putWord(dataOut+outLen,strUpKeysDirty.SN_CO2);
	outLen+=2;
}
void up_key_SN_FILTER_MESH_HOURS(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=15;
	dataOut[outLen++]=2;
	if (data!=NULL)
		strUpKeysDirty.SN_FILTER_MESH_HOURS=getWord(data,pos);
	putWord(dataOut+outLen,strUpKeysDirty.SN_FILTER_MESH_HOURS);
	outLen+=2;
}
void up_key_ERRORCODE(uint8 *data,int pos,int len,uint8 keyIdx)
{
	dataOut[outLen++]=16;
	dataOut[outLen++]=1;
	if (data!=NULL)
		strUpKeysDirty.ERRORCODE=getByte(data,pos);
	putByte(dataOut+outLen,strUpKeysDirty.ERRORCODE);
	outLen+=1;
}
void up_fun_up(uint8 *data)
{
	if (data!=NULL)
	{
		memset((char *)&strUpKeysDirty,0,sizeof(StrUpKeys));
		up_key_SN_POWER(data,6,0,1);
		up_key_SN_CHILD_SAFETY_LOCK(data,6,0,2);
		up_key_UV_LIGHT(data,6,0,3);
		up_key_SN_NEGATIVE_ION(data,6,0,4);
	    up_key_SN_LED_LIGHT(data,6,0,5);
		up_key_BEEP(data,6,0,6);
		up_key_SN_RESET_FILTER_MESH(data,6,0,7);
		up_key_FILTER_WARN(data,6,0,8);
		up_key_SN_MODE(data,7,1,9);
		up_key_LEFT_TIMER_OFF(data,8,2,10);
		up_key_SN_FANSPEED(data,10,1,11);
		up_key_SN_PM25(data,11,2,12);
		up_key_SN_VOC(data,13,2,13);
		up_key_SN_CO2(data,15,2,14);
		up_key_SN_FILTER_MESH_HOURS(data,17,2,15);
	}
	else
		up_key_ERRORCODE(data,19,1,16);
	if (memcmp((char *)&strUpKeysNow,(char *)&strUpKeysDirty,sizeof(StrUpKeys))==0)
		device_updated=0;
	else
	{
		memcpy((char *)&strUpKeysNow,(char *)&strUpKeysDirty,sizeof(StrUpKeys));
		device_updated=1;
		structUpSynch();
	}
}
void upstream(uint8 *data)
{
	memset(dataOut,0,dataOut_size);
	outLen=0;
	int funCode0=data[0];
	if(funCode0==0xAA)
		up_fun_up(data);
}

// downstream code
void dn_key_SN_POWER()
{
	putBits(dataOut+6,strDownKeysDirty.SN_POWER,0x1,0);
	outMaxLen(6,1,&outLen);
	putByte(dataOut+0,0xAA);
	outMaxLen(0,1,&outLen);
}
void dn_key_SN_CHILD_SAFETY_LOCK()
{
	putBits(dataOut+6,strDownKeysDirty.SN_CHILD_SAFETY_LOCK,0x2,1);
	outMaxLen(6,1,&outLen);
}
void dn_key_UV_LIGHT()
{
	putBits(dataOut+6,strDownKeysDirty.UV_LIGHT,0x4,2);
	outMaxLen(6,1,&outLen);
}
void dn_key_SN_NEGATIVE_ION()
{
	putBits(dataOut+6,strDownKeysDirty.SN_NEGATIVE_ION,0x8,3);
	outMaxLen(6,1,&outLen);
}
void dn_key_SN_LED_LIGHT()
{
	putBits(dataOut+6,strDownKeysDirty.SN_LED_LIGHT,0x10,4);
	outMaxLen(6,1,&outLen);
}
void dn_key_BEEP()
{
	putBits(dataOut+6,strDownKeysDirty.BEEP,0x20,5);
	outMaxLen(6,1,&outLen);
}
void dn_key_SN_RESET_FILTER_MESH()
{
	putBits(dataOut+6,strDownKeysDirty.SN_RESET_FILTER_MESH,0x40,6);
	outMaxLen(6,1,&outLen);
}
void dn_key_SN_MODE()
{
	putByte(dataOut+7,strDownKeysDirty.SN_MODE);
	outMaxLen(7,1,&outLen);
}
void dn_key_TIMER_OFF()
{
	putWord(dataOut+8,strDownKeysDirty.TIMER_OFF);
	outMaxLen(8,2,&outLen);
}
void dn_key_SN_FANSPEED()
{
	putByte(dataOut+10,strDownKeysDirty.SN_FANSPEED);
	outMaxLen(10,1,&outLen);
}
int getFeild(uint8 *data,int *pos,struct _FEILD_ *feild)
{
	feild->idx=*(data+*pos);
	feild->len=*(data+*pos+1);
	if (feild->len==0 || feild->idx==0)
		return 0;
	feild->pVal=*pos+2;
	(*pos)+=2+feild->len;
	return 1;
}
void setStruct(uint8 *data)
{
	int pos=0;
	struct _FEILD_ feild;
	while(getFeild(data,&pos,&feild)==1)
	{
		switch(feild.idx)
		{
		case 1:
			strDownKeysDirty.SN_POWER=getByte(data,feild.pVal);
			break;
		case 2:
			strDownKeysDirty.SN_CHILD_SAFETY_LOCK=getByte(data,feild.pVal);
			break;
		case 3:
			strDownKeysDirty.UV_LIGHT=getByte(data,feild.pVal);
			break;
		case 4:
			strDownKeysDirty.SN_NEGATIVE_ION=getByte(data,feild.pVal);
			break;
		case 5:
			strDownKeysDirty.SN_LED_LIGHT=getByte(data,feild.pVal);
			break;
		case 6:
			strDownKeysDirty.BEEP=getByte(data,feild.pVal);
			break;
		case 7:
			strDownKeysDirty.SN_RESET_FILTER_MESH=getByte(data,feild.pVal);
			break;
		case 8:
			strDownKeysDirty.SN_MODE=getByte(data,feild.pVal);
			break;
		case 9:
			strDownKeysDirty.TIMER_OFF=getWord(data,feild.pVal);
			break;
		case 10:
			strDownKeysDirty.SN_FANSPEED=getByte(data,feild.pVal);
			break;
		}
	}
}
void act_SET(uint8 *data,int *pos)
{
	memcpy((char *)&strDownKeysDirty,(char *)&strDownKeysNow, sizeof(StrDownKeys));
	setStruct(data);
	dn_key_SN_POWER();
	dn_key_SN_CHILD_SAFETY_LOCK();
	dn_key_UV_LIGHT();
	dn_key_SN_NEGATIVE_ION();
	dn_key_SN_LED_LIGHT();
	dn_key_BEEP();
	dn_key_SN_RESET_FILTER_MESH();
	dn_key_SN_MODE();
	dn_key_TIMER_OFF();
	dn_key_SN_FANSPEED();
}
void downstream(uint8 *data)
{
	memset(dataOut,0,dataOut_size);
	outLen=0;
	int actCode=ACT_CODE_SET;	//getWord(data,0);
	int pos=0;
	if (actCode==ACT_CODE_SET)
		act_SET(data,&pos);
}

// autoResponse()
void autoResponse()
{
	memset(dataOut,0,dataOut_size);
	outLen=0;
	// TODO:auto response
	memcpy((char *)&strUpKeysDirty,(char *)&strDownKeysDirty, sizeof(StrDownKeys));
}
void setHeap(void *heap, int size)
{
	dataOut = (uint8 *)heap;
	dataOut_size = size;
}
void setConfirmed()
{
	//memcpy(device_id, "NW2398", strlen("NW2398"));
}
