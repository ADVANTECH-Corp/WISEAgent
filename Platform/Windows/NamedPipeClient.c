#include "NamedPipeClient.h"
#include "IPCCommData.h"
#include <Windows.h>

//-------------------------------Static function define---------------------------------------------------
static BOOL NamedPipeClientGNSend(PIPECLINETHANDLE pipeClientHandle, IPCMSGTYPE msgType,  char * sendData, DWORD sendToLen);
static DWORD WINAPI PipeClientRecvThreadStart(LPVOID * pThreadParam);
//--------------------------------------------------------------------------------------------------------

static BOOL NamedPipeClientGNSend(PIPECLINETHANDLE pipeClientHandle, IPCMSGTYPE msgType,  char * sendData, DWORD sendToLen)
{
   BOOL bRet = FALSE;
   if(pipeClientHandle ==NULL || sendData == NULL || sendToLen <= 0) return bRet;
   {
      DWORD sendLen = 0;

         if(pipeClientHandle->hPipe && pipeClientHandle->isConnected)
         {
            DWORD realSendToLen = sizeof(IPCMSG) + sendToLen;
            PIPCMSG pIPCMsg = (PIPCMSG)malloc(realSendToLen);
            memset(pIPCMsg, 0, realSendToLen);
            pIPCMsg->msgType = msgType;
            pIPCMsg->msgContentLen = sendToLen;
            memcpy(pIPCMsg->msgContent, sendData, sendToLen);
            bRet = WriteFile(pipeClientHandle->hPipe, (char *)pIPCMsg, realSendToLen, &sendLen, NULL);
            free(pIPCMsg);
         }

   }
   return bRet;
}

static DWORD WINAPI PipeClientRecvThreadStart(LPVOID * pThreadParam)
{
   PIPECLINETHANDLE  pipeClientHandle = (PIPECLINETHANDLE)pThreadParam;
   char recvBuf[DEF_IPC_OUTPUT_BUF_SIZE] = {0};
   DWORD recvCnt = 0;
   BOOL bRet = FALSE;
   while(pipeClientHandle->pipeRecvThreadRun)
   {
      if(pipeClientHandle->hPipe && pipeClientHandle->isConnected)
      {
         if(recvCnt > 0)
         {
            memset(recvBuf, 0, sizeof(recvBuf));
            recvCnt = 0;
         }
         bRet = ReadFile(pipeClientHandle->hPipe, recvBuf, sizeof(recvBuf), &recvCnt, NULL);
         if(bRet && recvCnt > 0)
         {
            PIPCMSG pIPCMsg = (PIPCMSG)recvBuf;
            switch(pIPCMsg->msgType)
            {
            case INTER_MSG:
               {
                  break;
               }
            case USER_MSG:
               {
                  char * userRecvBuf = pIPCMsg->msgContent;
                  DWORD userRecvCnt = pIPCMsg->msgContentLen;
                  if(pipeClientHandle->pPipeRecvCB)
                  {
                     pipeClientHandle->pPipeRecvCB(userRecvBuf, userRecvCnt);
                  }
                  break;
               }
            default: break;
            }
         }
      }
      Sleep(10);
   }
   return 0;
}


PIPECLINETHANDLE NamedPipeClientConnect(char * pName, unsigned long commID, PPIPERECVCB pPipeRecvCB)
{
   PIPECLINETHANDLE pipeClientHandle = NULL;
   BOOL bFlag = FALSE;
   if(pName == NULL || commID <= 0) return pipeClientHandle;
   if(!WaitNamedPipe(pName, NMPWAIT_WAIT_FOREVER)) return pipeClientHandle;
   {
      pipeClientHandle = (PIPECLINETHANDLE)malloc(sizeof(NAMEDPIPECLIENT));
      memset(pipeClientHandle, 0, sizeof(NAMEDPIPECLIENT));
      pipeClientHandle->pPipeRecvCB = pPipeRecvCB;
      pipeClientHandle->commID = commID;
      pipeClientHandle->pipeRecvThreadRun = TRUE;
      pipeClientHandle->pipeRecvThreadHandle = CreateThread(NULL, 0, PipeClientRecvThreadStart,pipeClientHandle, 0, NULL);
      if(!pipeClientHandle->pipeRecvThreadHandle)
      {
         pipeClientHandle->pipeRecvThreadRun = FALSE;
         goto done;
      }

      pipeClientHandle->hPipe = CreateFile(pName, GENERIC_READ |GENERIC_WRITE,
         0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if(INVALID_HANDLE_VALUE == pipeClientHandle->hPipe)
      {
         pipeClientHandle->isConnected = FALSE;
         goto done;
      }

      {
         DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
         SetNamedPipeHandleState(pipeClientHandle->hPipe, &dwMode, NULL, NULL);
      }

      pipeClientHandle->isConnected = TRUE;

      {
         IPCINTERMSG ipcInterMsg;
         ipcInterMsg.interCmdKey = IPC_CONNECT;
         ipcInterMsg.interParams.commID = commID;
         bFlag = NamedPipeClientGNSend(pipeClientHandle, INTER_MSG, (char *)&ipcInterMsg, sizeof(IPCINTERMSG));
         if(!bFlag) goto done;
      }

      bFlag =TRUE;
   }
   
done:
   if(!bFlag && pipeClientHandle)
   {
      NamedPipeClientDisconnect(pipeClientHandle);
     /* free(pipeClientHandle);*/
      pipeClientHandle = NULL;
   }
   return pipeClientHandle;
}

void NamedPipeClientDisconnect(PIPECLINETHANDLE pipeClientHandle)
{
   if(!pipeClientHandle) return;

   if(pipeClientHandle->pipeRecvThreadHandle)
   {
      pipeClientHandle->pipeRecvThreadRun = FALSE;
      WaitForSingleObject(pipeClientHandle->pipeRecvThreadHandle, INFINITE);
      CloseHandle(pipeClientHandle->pipeRecvThreadHandle);
      pipeClientHandle->pipeRecvThreadHandle = NULL;
   }

   if(pipeClientHandle->hPipe)
   {
      CloseHandle(pipeClientHandle->hPipe);
      pipeClientHandle->hPipe = NULL;
      pipeClientHandle->isConnected = FALSE;
   }

   free(pipeClientHandle);
   pipeClientHandle = NULL;
}

int NamedPipeClientSend(PIPECLINETHANDLE pipeClientHandle, char * sendData, unsigned long sendToLen)
{
   BOOL bRet = TRUE;
   bRet = NamedPipeClientGNSend(pipeClientHandle, USER_MSG, sendData, sendToLen);
   return bRet;
}