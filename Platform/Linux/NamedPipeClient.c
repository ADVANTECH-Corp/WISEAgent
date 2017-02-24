#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "NamedPipeClient.h"
#include "IPCCommData.h"

static bool NamedPipeClientGNSend(PIPECLINETHANDLE pipeClientHandle, IPCMSGTYPE msgType,  char * sendData, unsigned long sendToLen)
{
   bool bRet = false;
   if(pipeClientHandle ==NULL || sendData == NULL || sendToLen <= 0) return bRet;
   {
      unsigned long sendLen = -1;

         if(pipeClientHandle->hPipe && pipeClientHandle->isConnected)
         {
            unsigned long realSendToLen = sizeof(IPCMSG) + sendToLen;
            PIPCMSG pIPCMsg = (PIPCMSG)malloc(realSendToLen);
            memset(pIPCMsg, 0, realSendToLen);
            pIPCMsg->msgType = msgType;
            pIPCMsg->msgContentLen = sendToLen;
            memcpy(pIPCMsg->msgContent, sendData, sendToLen);
	    sendLen = write(pipeClientHandle->hPipe, (char *)pIPCMsg, realSendToLen);
            free(pIPCMsg);
         }
	  if(sendLen < 0)
		  bRet = false;
	  else
		  bRet = true;
   }
   return bRet;
}

PIPECLINETHANDLE NamedPipeClientConnect(char * pName, unsigned long commID, PPIPERECVCB pPipeRecvCB)
{
      PIPECLINETHANDLE pipeClientHandle = NULL;
      bool bFlag = false;
	  
      if(pName == NULL || commID <= 0 || pPipeRecvCB != NULL) return pipeClientHandle;
	
      if(access(pName, F_OK)) return pipeClientHandle;
         
      pipeClientHandle = (PIPECLINETHANDLE)malloc(sizeof(NAMEDPIPECLIENT));
      memset(pipeClientHandle, 0, sizeof(NAMEDPIPECLIENT));
      pipeClientHandle->pPipeRecvCB = pPipeRecvCB;
      pipeClientHandle->commID = commID;
      pipeClientHandle->pipeRecvThreadRun = true;
      pipeClientHandle->pipeRecvThreadHandle = NULL;

      pipeClientHandle->hPipe = open(pName, O_WRONLY);
	  
      if(-1 == pipeClientHandle->hPipe)
      {
         pipeClientHandle->isConnected = false;
         goto done;
      }

      pipeClientHandle->isConnected = true;

      {
         IPCINTERMSG ipcInterMsg;
         ipcInterMsg.interCmdKey = IPC_CONNECT;
         ipcInterMsg.interParams.commID = commID;
         bFlag = NamedPipeClientGNSend(pipeClientHandle, INTER_MSG, (char *)&ipcInterMsg, sizeof(IPCINTERMSG));
         if(!bFlag) goto done;
      }

      bFlag =true;
   
done:
   if(!bFlag)
   {
      NamedPipeClientDisconnect(pipeClientHandle);
      free(pipeClientHandle);
      pipeClientHandle = NULL;
   }
   return pipeClientHandle;
}

void NamedPipeClientDisconnect(PIPECLINETHANDLE pipeClientHandle)
{
   if(!pipeClientHandle) return;
   
   if(pipeClientHandle->hPipe)
   {
      close((int)pipeClientHandle->hPipe);
      pipeClientHandle->hPipe = 0;
      pipeClientHandle->isConnected = false;
   }
   free(pipeClientHandle);
   pipeClientHandle = NULL;
}

int NamedPipeClientSend(PIPECLINETHANDLE pipeClientHandle, char * sendData, unsigned long sendToLen)
{
   bool bRet = true;
   bRet = NamedPipeClientGNSend(pipeClientHandle, USER_MSG, sendData, sendToLen);
   return bRet;
}
