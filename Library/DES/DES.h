#ifndef _DES_CBC_PKCS7_H_
#define _DES_CBC_PKCS7_H_

#define IN
#define OUT

#ifdef __cplusplus
extern "C" {
#endif

int DESEncode(IN char * pKey, IN char * pIV, IN char * pPlaintext, OUT char * pCiphertext);
int DESDecode(IN char * pKey, IN char * pIV, IN char * pCiphertext, OUT char * pPlaintext);

int DESEncodeEx(IN char * pKey, IN char * pIV, IN char * pPlaintext, OUT char * pCiphertext, OUT int * cipherLen);
int DESDecodeEx(IN char * pKey, IN char * pIV, IN char * pCiphertext, IN int cipherLen, OUT char * pPlaintext);

#ifdef __cplusplus
}
#endif

#endif