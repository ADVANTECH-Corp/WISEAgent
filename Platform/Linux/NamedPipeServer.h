#ifndef  _NAMEDPIPE_SERVER_H_
#define  _NAMEDPIPE_SERVER_H_

#include <stdbool.h>

typedef bool (* PPIPERECVCB)(char*, int);

int NamedPipeServerInit(char * pName, unsigned long instCnt);

void NamedPipeServerUninit();

int IsChannelConnect(unsigned long commID);

int NamedPipeServerRegRecvCB(unsigned long commID, PPIPERECVCB pipeRecvCB);

int NamedPipeServerSend(unsigned long commID, char * sendData, unsigned long sendToLen);

#endif
