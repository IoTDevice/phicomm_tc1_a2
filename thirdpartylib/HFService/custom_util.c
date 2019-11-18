#include "custom_util.h"

uint8 getBits(uint8 *data,int pos,uint8 bitmask,int shift)
{
	uint8 ret;
	ret=*(data+pos);
	ret=(ret & bitmask)>>shift;
	return ret;
}

void putBits(uint8 *data,uint8 val,uint8 bitmask,int shift)
{
	val=(val<<shift)&bitmask;
	*data=(*data) | val;
}

uint8 getByte(uint8 *data,int pos)
{
	uint8 ret=*(data+pos);
	return ret;
}

void putByte(uint8 *data,int val)
{
	*data=val;
}

uint16 getWord(uint8 *data,int pos)
{
	uint16 dat1,dat2;
	if (IS_BIGENDIAN==1)
	{
		dat1=*(data+pos);
		dat2=*(data+pos+1);
	}
	else
	{
		dat1=*(data+pos+1);
		dat2=*(data+pos);
	}
	return (dat2<<8)+dat1;
}

void putWord(uint8 *data,int val)
{
	uint8 dat1,dat2;
	dat2=val&0xff;
	dat1=(val&0xff00)>>8;
	if (IS_BIGENDIAN==1)
	{
		*(data)=dat1;
		*(data+1)=dat2;
	}
	else
	{
		*(data)=dat2;
		*(data+1)=dat1;
	}
}

int getInt(uint8 *data,int pos)
{
	int dat1,dat2,dat3,dat4;
	if (IS_BIGENDIAN==1)
	{
		dat1=*(data+pos);
		dat2=*(data+pos+1);
		dat3=*(data+pos+2);
		dat4=*(data+pos+3);
	}
	else
	{
		dat1=*(data+pos+3);
		dat2=*(data+pos+2);
		dat3=*(data+pos+1);
		dat4=*(data+pos);
	}
	return (dat4<<24)+(dat3<<16)+(dat2<<8)+dat1;
}

void putInt(uint8 *data,int val)
{
	uint8 dat1,dat2,dat3,dat4;
	dat1=val&0xff;
	dat2=(val&0xff00)>>8;
	dat3=(val&0xff0000)>>16;
	dat4=(val&0xff000000)>>24;
	if (IS_BIGENDIAN==1)
	{
		*(data)=dat1;
		*(data+1)=dat2;
		*(data+2)=dat3;
		*(data+3)=dat4;
	}
	else
	{
		*(data)=dat4;
		*(data+1)=dat3;
		*(data+2)=dat2;
		*(data+3)=dat1;
	}
}

void outMaxLen(int pos,int len,int *ol)
{
	int l=pos+len;
	*ol=(*ol>l)?*ol:l;
}

