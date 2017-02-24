#ifndef _BASE_64_H_
#define _BASE_64_H_

#define IN
#define OUT

#ifdef __cplusplus
extern "C" {
#endif

int Base64Encode(IN const char * data, IN int dataLen, OUT char ** encData, OUT int * encLen);
int Base64Decode(IN const char * data, IN int dataLen, OUT char ** decData, OUT int * decLen);

#ifdef __cplusplus
}
#endif

#endif