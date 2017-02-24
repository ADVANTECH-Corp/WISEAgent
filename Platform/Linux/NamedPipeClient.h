#ifndef _NAMEDPIPE_CLIENT_H_
#define _NAMEDPIPE_CLIENT_H_

#include <stdbool.h>

typedef int (* PPIPERECVCB)(char*, int);

typedef struct NamedPipeClient{
   int  hPipe;
   unsigned long commID;
   bool isConnected;
   void * pipeRecvThreadHandle;
   bool   pipeRecvThreadRun;
   PPIPERECVCB pPipeRecvCB;
}NAMEDPIPECLIENT, *PNAMEDPIPECLIENT;

typedef PNAMEDPIPECLIENT PIPECLINETHANDLE;

PIPECLINETHANDLE NamedPipeClientConnect(char * pName, unsigned long commID, PPIPERECVCB pPipeRecvCB);

void NamedPipeClientDisconnect(PIPECLINETHANDLE pipeClientHandle);

int NamedPipeClientSend(PIPECLINETHANDLE pipeClientHandle, char * sendData, unsigned long sendToLen);

#endif