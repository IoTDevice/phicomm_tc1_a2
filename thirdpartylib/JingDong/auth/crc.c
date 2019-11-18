#include "crc.h"
uint16_t CRC16(const uint8_t * buffer, uint32_t size)
{
	uint16_t crc = 0xFFFF;  

	if (buffer && size)    
		while (size--)    
		{
			crc = (crc >> 8) | (crc << 8);
			crc ^= *buffer++;
			crc ^= ((unsigned char) crc) >> 4;
			crc ^= crc << 12;
			crc ^= (crc & 0xFF) << 5;
		}  
	return crc;
}

unsigned char crc8(unsigned char *A,unsigned char n)
{
	unsigned char i;
	unsigned char checksum = 0;

	while(n--)
	{
		for(i=1;i!=0;i*=2)
		{
			if( (checksum&1) != 0 )
			{
				checksum /= 2;
				checksum ^= 0X8C;
			}
			else
			{
				checksum /= 2;
			}

			if( (*A & i) != 0 )
			{
				checksum ^= 0X8C;
			}
		}
		A++;
	}
	return(checksum);
}
