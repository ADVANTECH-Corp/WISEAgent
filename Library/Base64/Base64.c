#include <string.h>
#include <malloc.h>
#include "Base64.h"

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="; 
static int FindPos(char ch);

static int FindPos(char ch)
{
	int iRet = -1;
	char *ptr = (char*)strrchr(base, ch);
	if(ptr != NULL) iRet = ptr - base;
	return iRet; 
}

int Base64Encode(IN const char * data, IN int dataLen, OUT char ** encData, OUT int * encLen)
{
	int iRet = -1;
	if(NULL == data || dataLen <=0 || NULL == encData || NULL == encLen) return iRet;
	{
		int bsStrLen = 0, bsBufLen = 0;
		int i = 0, j = 0, k = 0;
		int bsIndex = 0;
		int perEnBuf = 0;   //use 3 bytes
		char *enRetBuf = NULL;

		//bsStrLen = (dataLen%3 ? dataLen/3+1 : dataLen/3)*4;
		bsStrLen = dataLen/3;
		if(dataLen%3 > 0)
		{
			bsStrLen += 1;
		}
		bsStrLen = bsStrLen*4;
		bsBufLen = bsStrLen + 1;

		*encData = (char *)malloc(bsBufLen);
		if(*encData == NULL) return iRet = 3;
		enRetBuf = *encData;
		memset(enRetBuf, 0, bsBufLen);
		*encLen = bsStrLen;
		
      while(i < dataLen)
		{
			perEnBuf = 0;
			j = 0;
			while(j < 3) //per 3 bytes(24bits) to base64 encode
			{
				if(i >= dataLen) break;
            perEnBuf = ((perEnBuf << 8) | (data[i] & 0xFF));
				j++;
				i++;
			}
			perEnBuf = (perEnBuf << (3-j)*8);

			for(k = 0; k < 4; k++)
			{
				if(j < k) //add = (j=1 add tow =, j = 2 add one =)
				{
					bsIndex = 0x40;
				}
				else
				{
					bsIndex = (perEnBuf >> ((3-k)*6)) & 0x3F;
				}
				*enRetBuf = base[bsIndex];
				enRetBuf++;
			}
		}
		*enRetBuf = '\0';
	}
	return iRet = 0;
}

int Base64Decode(IN const char * data, IN int dataLen, OUT char ** decData, OUT int * decLen)
{
	int iRet = -1;
   if(NULL == data || dataLen <= 0 || NULL == decData || NULL == decLen) return iRet;
	if(dataLen%4 != 0) return iRet = 1;
	if(*(data + dataLen - 3) == '=') return iRet = 2;
	{
		int tempLen = 0;
		int nbsStrLen = 0, nbsBufLen = 0;
		int bsChPos = 0;
		int equalCnt = 0;
		int i = 0, j = 0, k = 0;
		int perDeBuf = 0;   //use 4 bytes
		char *deRetBuf = NULL;

		if(*(data + dataLen - 1) == '=') equalCnt++;
		if(*(data + dataLen - 2) == '=') equalCnt++;

		nbsStrLen = (dataLen - equalCnt)/4*3;
		if(equalCnt > 0)
		{
			nbsStrLen +=(3-equalCnt);
		}
		nbsBufLen = nbsStrLen + 1;

		*decData = (char *)malloc(nbsBufLen);
		if(*decData == NULL) return iRet = 3;
      deRetBuf = *decData;
		memset(deRetBuf, 0, nbsBufLen);
		*decLen = nbsStrLen;

		while(i < (dataLen - equalCnt))
		{
			perDeBuf = 0;
			j = 0;
			while(j < 4)
			{
				if(i >= (dataLen - equalCnt)) break;
            bsChPos = FindPos(data[i]);
				if(bsChPos == -1)
				{
					free(deRetBuf);
					return iRet = 4;
				}
				perDeBuf = (perDeBuf << 6) | bsChPos;
				j++;
				i++;
			}
         perDeBuf = perDeBuf<<((4-j)*6);

			for(k=0; k<3; k++)
			{
				if(j < 4 && k == (3 - equalCnt)) break;
				*deRetBuf = (char)((perDeBuf >> ((2-k)*8)) & 0xFF);
				deRetBuf++;
				tempLen++;
			}
		}
		*deRetBuf = '\0';
	}
	return iRet = 0;
}

