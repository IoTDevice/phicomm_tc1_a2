#ifndef _CUSTOM_UTIL_H_
#define _CUSTOM_UTIL_H_

// TODO: according to the UART definition
#define IS_BIGENDIAN		1
// TODO: the defined code of: SET,GET,MENU
#define ACT_CODE_GET		0x1001
#define ACT_CODE_SET		0x1002
#define ACT_CODE_MENU		0x1004

typedef unsigned char uint8;
typedef unsigned short uint16;

uint8 getBits(uint8 *data,int pos,uint8 bitmask,int shift);
void putBits(uint8 *data,uint8 val,uint8 bitmask,int shift);
uint8 getByte(uint8 *data,int pos);
void putByte(uint8 *data,int val);
uint16 getWord(uint8 *data,int pos);
void putWord(uint8 *data,int val);
void outMaxLen(int pos,int len,int *ol);

#endif


