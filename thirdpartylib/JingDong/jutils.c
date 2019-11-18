#include "debugPro.h"

#ifdef QCOM_4004B
#include "scw_common.h"
#else
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "hsf.h"
#include "cJSON.h"

#include "jutils.h"
#endif


int byte2hexstr(const uint8_t *pBytes, int srcLen, uint8_t *pDstStr, int dstLen)
{
	const char tab[] = "0123456789abcdef";
	int i = 0;

	memset(pDstStr, 0, dstLen);

	if (dstLen < srcLen * 2)
		srcLen = (dstLen - 1) / 2;

	for (i = 0; i < srcLen; i++)
	{
		*pDstStr++ = tab[*pBytes >> 4];
		*pDstStr++ = tab[*pBytes & 0x0f];
		pBytes++;
	}
	*pDstStr++ = 0;
	return srcLen * 2;
}

int utilGetIPString(struct sockaddr_in* pPeerAddr, char* str)
{
	struct sockaddr_in* pAddr = (struct sockaddr_in*)pPeerAddr;
#ifdef QCOM_4004B
              int n = sprintf(str, "%s:%d",
		_inet_ntoa((A_UINT32)pAddr->sin_addr.s_addr),
		pAddr->sin_port
		);

#else
	int n = sprintf(str, "%s:%d",
		(char *)inet_ntoa(pAddr->sin_addr),
		pAddr->sin_port
		);
#endif

	return n;
}


/* hex string to bytes*/
int hexStr2bytes(const char *hexStr, uint8_t *buf, int bufLen)
{
	int i;
	int len;

	if (NULL == hexStr)
	{
		len = 0;
	}
	else
	{
		len = (int)strlen(hexStr) / 2;

		if (bufLen < len)
			len = bufLen;
	}
	memset(buf, 0, bufLen);

	for (i = 0; i < len; i++)
	{
		char ch1, ch2;
		int val;

		ch1 = hexStr[i * 2];
		ch2 = hexStr[i * 2 + 1];
		if (ch1 >= '0' && ch1 <= '9')
			val = (ch1 - '0') * 16;
		else if (ch1 >= 'a' && ch1 <= 'f')
			val = ((ch1 - 'a') + 10) * 16;
		else if (ch1 >= 'A' && ch1 <= 'F')
			val = ((ch1 - 'A') + 10) * 16;
		else
			return -1;

		if (ch2 >= '0' && ch2 <= '9')
			val += ch2 - '0';
		else if (ch2 >= 'a' && ch2 <= 'f')
			val += (ch2 - 'a') + 10;
		else if (ch2 >= 'A' && ch2 <= 'F')
			val += (ch2 - 'A') + 10;
		else
			return -1;

		buf[i] = val & 0xff;
	}

	return 0;
}

#ifdef QCOM_4004B
int tolower(int ch)
{
	if ((unsigned int)(ch - 'A') < 26u)
		ch += 'a' - 'A';
	return ch;
}
#endif
static uint32_t unix_time_value;
static uint32_t getsysms(void)
{
#ifdef QCOM_4004B
	unsigned long ret;
#else
	uint64_t ret;
#endif

#ifdef __MRVL_MW300__
	ret = os_get_timestamp() / 1000UL;
#elif defined(__LINUX__)
	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &monotime);
	ret = 1000UL * monotime.tv_sec + (monotime.tv_nsec / 1000000UL);
#elif defined(_WIN32)
	ret = GetTickCount();
#elif defined(QCOM_4004B)
	ret = time_ms();
#else
     	ret = hfsys_get_time();
#endif
	return ret;
}
void timerReset(uint32_t *timestamp)
{
	unix_time_value = getsysms() & 0xFFFFFFFF;
	*timestamp = unix_time_value;
}

int isTimeOut(uint32_t timestamp, uint32_t timeout)  //???
{
	unix_time_value = getsysms() & 0xFFFFFFFF;
	uint32_t ret = (timestamp + timeout) - unix_time_value;
	return ret & 0x80000000;
}

int addressis_ip(const char * ipaddr)
{
	char ii, ipadd;
	int i, j;
	
	ii=0;
	for (j= 0; j< 4; j++)
	{
		ipadd=0;
		for (i=0; i< 4; i++, ii++)
		{
			if (*(ipaddr+ii)=='.')
				if (i== 0)
					return 0;		//the first shall not be '.'
				else
				{
					ii++;
					break;			//this feild finished
				}
			else if ((i==3)&&(j!=3))	//not the last feild, the number shall less than 4 bits
				return 0;
			else if ((*(ipaddr+ii) > '9')||(*(ipaddr+ii) < '0'))
			{
				if ((*(ipaddr+ii) == '\0')&&(j==3)&&(i!=0))
				{
					break;
				}
				else
					return 0;			//pls input number
			}
			else
				ipadd= ipadd*10+(*(ipaddr+ii)-'0');
			if (ipadd > 255)
				return 0;
		}
	}
	return 1;
}


int str2int(char* pStr)
{
  cJSON jValue = {0};
  parse_number(&jValue, pStr);
  return jValue.valueint;
}
/**
 * brief: 
 *
 * @Param: msg
 * @Param: is_fmt
 * @Param: num_line
 * @Param: buff
 * @Param: len
 */
void joylink_util_print_buffer(const char *msg, int is_fmt, int num_line, const uint8_t *buff, int len)
{
    if(NULL == msg || NULL == buff){
        return;
    }
    int cut_num = 4;
    int i = 0;
    u_printf("------:%s\n", msg);
    for(i =0; i < len; i++){
        u_printf("%02x ", (int)buff[i]);
        if(is_fmt && !(num_line%cut_num)){
            if(!((i + 1)%cut_num)){
                u_printf("| ");
            }
        }

        if(!((i + 1)%num_line)){
            u_printf("\n");
        }
    }
    u_printf("\n");
}

void joylink_util_free(void *p)
{
    if(p)
	{
        hfmem_free(p);
        p = NULL;
    }
}
void *joylink_util_malloc(size_t size)
{

    void *p = hfmem_malloc(size);
    if (!p){
        return p;
    }
    memset(p, 0, size);

    return p;
}
uint8_t joyTLVDataAdd(uint8_t *buf,uint8_t tag, uint8_t lc, uint8_t *value)
{
    uint8_t lenthtotal = 0;
    if(buf == NULL || value == NULL){
        return 0;
    }
    *buf        = tag;
    *(buf + 1)  = lc;
    memcpy(buf + 2, value, lc);
    lenthtotal = lc + 2;
    return lenthtotal;
}


