#ifndef  _NAMEDPIPE_SERVER_H_
#define  _NAMEDPIPE_SERVER_H_

#include <stdbool.h>

typedef bool (* PPIPERECVCB)(char*, int);

typedef struct PipeInstance{
   void * hPipe;
   unsigned long commID;
   bool isConnected;
   void * pipeListenThreadHandle;
   bool   pipeListenThreadRun;
   void * pipeRecvThreadHandle;
   bool   pipeRecvThreadRun;
   PPIPERECVCB pipeRecvCB;
}PIPEINST, *PPIPEINST;

typedef struct PipeInstNode * PPIPEINSTNODE;

typedef struct PipeInstNode{
   PIPEINST  pipeInst;
   PPIPEINSTNODE next;
}PIPEINSTNODE;

typedef PPIPEINSTNODE PIPEINSTLIST;

int NamedPipeServerInit(char * pName, unsigned long instCnt);

void NamedPipeServerUninit();

int IsChannelConnect(unsigned long commID);

int NamedPipeServerRegRecvCB(unsigned long commID, PPIPERECVCB pipeRecvCB);

int  NamedPipeServerSend(unsigned long commID, char * sendData, unsigned long sendToLen);

#endif