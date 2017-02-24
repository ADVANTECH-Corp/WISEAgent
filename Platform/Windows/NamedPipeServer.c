#include "NamedPipeServer.h"
#include "IPCCommData.h"
#include <Windows.h>

PIPEINSTLIST  PipeInstList = NULL;

//----------------------------Watch object list function define--------------------------
static PIPEINSTLIST CreatePipeInstList();
static void DestroyPipeInstList(PIPEINSTLIST pipeInstList);
static PPIPEINSTNODE InsertPipeInstNode(PIPEINSTLIST pipeInstList, HANDLE hPipe);
static PPIPEINSTNODE FindPipeInstNodeWithCommID(PIPEINSTLIST pipeInstList, DWORD commID);
static PPIPEINSTNODE FindPipeInstNodeWithHandle(PIPEINSTLIST pipeInstList, HANDLE hPipe);
static BOOL DeletePipeInstNode(PIPEINSTLIST pipeInstList, DWORD commID);
static BOOL DeleteAllPipeInstNode(PIPEINSTLIST pipeInstList);
//---------------------------------------------------------------------------------------

//---------------------------Other function define---------------------------------------
static DWORD WINAPI PipeInstListenThreadStart(LPVOID * pThreadParam);
static DWORD WINAPI PipeInstRecvThreadStart(LPVOID * pThreadParam);
static BOOL LaunchPipeInst(PIPEINSTLIST pipeInstList);
static BOOL UnlaunchPipeInst(PIPEINSTLIST pipeInstList);
static BOOL NamedPipeServerGNSend(IPCMSGTYPE msgType, DWORD commID, char * sendData, DWORD sendToLen);
//---------------------------------------------------------------------------------------

static PIPEINSTLIST CreatePipeInstList()
{
   PPIPEINSTNODE head = NULL;
   head = (PPIPEINSTNODE)malloc(sizeof(PIPEINSTNODE));
   if(head) 
   {
      head->next = NULL;
      head->pipeInst.commID = 0;
      head->pipeInst.hPipe = NULL;
      head->pipeInst.isConnected = FALSE;
      head->pipeInst.pipeListenThreadHandle = NULL;
      head->pipeInst.pipeListenThreadRun = FALSE;
      head->pipeInst.pipeRecvThreadHandle = NULL;
      head->pipeInst.pipeRecvThreadRun = FALSE;
      head->pipeInst.pipeRecvCB = NULL;
   }
   return head;
}

static void DestroyPipeInstList(PIPEINSTLIST pipeInstList)
{
   PPIPEINSTNODE head = pipeInstList;
   if(NULL == head) return;
   DeleteAllPipeInstNode(head);
   free(head); 
   head = NULL;
}

static PPIPEINSTNODE InsertPipeInstNode(PIPEINSTLIST pipeInstList, HANDLE hPipe)
{
   PPIPEINSTNODE pRet = NULL;
   PPIPEINSTNODE newNode = NULL;
   PPIPEINSTNODE findNode = NULL;
   PPIPEINSTNODE head = pipeInstList;
   if(head == NULL || hPipe == NULL) return pRet;
   findNode = FindPipeInstNodeWithHandle(head, hPipe);
   if(findNode == NULL)
   {
      newNode = (PPIPEINSTNODE)malloc(sizeof(PIPEINSTNODE));
      newNode->pipeInst.commID = 0;
      newNode->pipeInst.hPipe = hPipe;
      newNode->pipeInst.isConnected = FALSE;
      newNode->pipeInst.pipeListenThreadHandle = NULL;
      newNode->pipeInst.pipeListenThreadRun = FALSE;
      newNode->pipeInst.pipeRecvThreadHandle = NULL;
      newNode->pipeInst.pipeRecvThreadRun = FALSE;
      newNode->pipeInst.pipeRecvCB = NULL;

      newNode->next = head->next;
      head->next = newNode;
      pRet = newNode;
   }
   return pRet;
}

static PPIPEINSTNODE FindPipeInstNodeWithCommID(PIPEINSTLIST pipeInstList, DWORD commID)
{
   PPIPEINSTNODE head = pipeInstList;
   PPIPEINSTNODE findNode = NULL;
   if(head == NULL) return findNode;
   findNode = head->next;
   while(findNode)
   {
      if(findNode->pipeInst.commID == commID) break;
      else
      {
         findNode = findNode->next;
      }
   }

   return findNode;
}

static PPIPEINSTNODE FindPipeInstNodeWithHandle(PIPEINSTLIST pipeInstList, HANDLE hPipe)
{
   PPIPEINSTNODE head = pipeInstList;
   PPIPEINSTNODE findNode = NULL;
   if(head == NULL) return findNode;
   findNode = head->next;
   while(findNode)
   {
      if(findNode->pipeInst.hPipe == hPipe) break;
      else
      {
         findNode = findNode->next;
      }
   }

   return findNode;
}

static BOOL DeletePipeInstNode(PIPEINSTLIST pipeInstList, DWORD commID)
{
   BOOL bRet = FALSE;
   PPIPEINSTNODE delNode = NULL;
   PPIPEINSTNODE p = NULL;
   PPIPEINSTNODE head = pipeInstList;
   if(head == NULL) return bRet;
   p = head;
   delNode = head->next;
   while(delNode)
   {
      if(delNode->pipeInst.commID == commID) 
      {
         p->next = delNode->next;

         if(delNode->pipeInst.pipeRecvThreadHandle)
         {
            delNode->pipeInst.pipeRecvThreadRun = FALSE;
            WaitForSingleObject(delNode->pipeInst.pipeRecvThreadHandle, INFINITE);
            CloseHandle(delNode->pipeInst.pipeRecvThreadHandle);
            delNode->pipeInst.pipeRecvThreadHandle = NULL;
         }

         if(delNode->pipeInst.pipeListenThreadHandle)
         {
            delNode->pipeInst.pipeListenThreadRun = FALSE;
            WaitForSingleObject(delNode->pipeInst.pipeListenThreadHandle, INFINITE);
            CloseHandle(delNode->pipeInst.pipeListenThreadHandle);
            delNode->pipeInst.pipeListenThreadHandle = NULL;
         }

         if(delNode->pipeInst.hPipe)
         {
            DisconnectNamedPipe(delNode->pipeInst.hPipe);
            CloseHandle(delNode->pipeInst.hPipe);
            delNode->pipeInst.hPipe = NULL;
         }
         
         free(delNode);
         delNode = NULL;
         bRet = TRUE;
         break;
      }
      else
      {
         p = delNode;
         delNode = delNode->next;
      }
   }
   return bRet;
}
static BOOL DeleteAllPipeInstNode(PIPEINSTLIST pipeInstList)
{
   BOOL bRet = FALSE;
   PPIPEINSTNODE delNode = NULL;
   PPIPEINSTNODE head = pipeInstList;
   if(head == NULL) return bRet;

   delNode = head->next;
   while(delNode)
   {
      head->next = delNode->next;

      if(delNode->pipeInst.pipeRecvThreadHandle)
      {
         delNode->pipeInst.pipeRecvThreadRun = FALSE;
         WaitForSingleObject(delNode->pipeInst.pipeRecvThreadHandle, INFINITE);
         CloseHandle(delNode->pipeInst.pipeRecvThreadHandle);
         delNode->pipeInst.pipeRecvThreadHandle = NULL;
      }

      if(delNode->pipeInst.pipeListenThreadHandle)
      {
         delNode->pipeInst.pipeListenThreadRun = FALSE;
         WaitForSingleObject(delNode->pipeInst.pipeListenThreadHandle, INFINITE);
         CloseHandle(delNode->pipeInst.pipeListenThreadHandle);
         delNode->pipeInst.pipeListenThreadHandle = NULL;
      }

      if(delNode->pipeInst.hPipe)
      {
         DisconnectNamedPipe(delNode->pipeInst.hPipe);
         CloseHandle(delNode->pipeInst.hPipe);
         delNode->pipeInst.hPipe = NULL;
      }

      free(delNode);
      delNode = head->next;
   }

   bRet = TRUE;
   return bRet;
}

static DWORD WINAPI PipeInstListenThreadStart(LPVOID * pThreadParam)
{
   PPIPEINST  pPipeInst = (PPIPEINST)pThreadParam;
   BOOL bRet = FALSE;
   DWORD dwErrorCode = 0;

   while(pPipeInst->pipeListenThreadRun)
   {   
      if(pPipeInst->hPipe)
      {
         bRet = ConnectNamedPipe(pPipeInst->hPipe, NULL);
         if(!bRet)
         {
            dwErrorCode = GetLastError();
            switch(dwErrorCode)
            {
            case ERROR_PIPE_LISTENING:
               {
                  pPipeInst->isConnected = FALSE;
                  break;
               }
            case ERROR_PIPE_CONNECTED:
               {
                  pPipeInst->isConnected = TRUE;
                  break;
               }
            case ERROR_NO_DATA:
               {
                  pPipeInst->isConnected = FALSE;
                  DisconnectNamedPipe(pPipeInst->hPipe);
                  pPipeInst->commID = 0;
                  break;
               }
            default:
               {
                  pPipeInst->isConnected = FALSE;
                  break;
               }
            }
         }
      }
      Sleep(10);
   }
   return 0;
}

static DWORD WINAPI PipeInstRecvThreadStart(LPVOID * pThreadParam)
{
   PPIPEINST  pPipeInst = (PPIPEINST)pThreadParam;
   char recvBuf[DEF_IPC_OUTPUT_BUF_SIZE] = {0};
   DWORD recvCnt = 0;
   BOOL bRet = FALSE;
   while(pPipeInst->pipeRecvThreadRun)
   {
      if(pPipeInst->hPipe && pPipeInst->isConnected)
      {
         if(recvCnt > 0)
         {
            memset(recvBuf, 0, sizeof(recvBuf));
            recvCnt = 0;
         }
         bRet = ReadFile(pPipeInst->hPipe, recvBuf, sizeof(recvBuf), &recvCnt, NULL);
         if(bRet && recvCnt > 0)
         {
            PIPCMSG pIPCMsg = (PIPCMSG)recvBuf;
            switch(pIPCMsg->msgType)
            {
            case INTER_MSG:
               {
                  IPCINTERMSG interMsg;
                  if(pIPCMsg->msgContentLen == sizeof(IPCINTERMSG))
                  {
                     memcpy(&interMsg, pIPCMsg->msgContent, sizeof(IPCINTERMSG));
                     if(interMsg.interCmdKey == IPC_CONNECT)
                     {
                        pPipeInst->commID = interMsg.interParams.commID;
                     }
                  }
                  break;
               }
            case USER_MSG:
               {
                  char * userRecvBuf = pIPCMsg->msgContent;
                  DWORD userRecvCnt = pIPCMsg->msgContentLen;
                  if(pPipeInst->pipeRecvCB)
                  {
                     pPipeInst->pipeRecvCB(userRecvBuf, userRecvCnt);
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

static BOOL LaunchPipeInst(PIPEINSTLIST pipeInstList)
{
   BOOL bRet = TRUE;
   if(pipeInstList == NULL) return FALSE;
   {
      PPIPEINSTNODE  pCurPipeInstNode = pipeInstList->next;
      while(pCurPipeInstNode)
      {
         pCurPipeInstNode->pipeInst.pipeListenThreadRun = TRUE;
         pCurPipeInstNode->pipeInst.pipeListenThreadHandle = CreateThread(NULL, 0, PipeInstListenThreadStart, (LPVOID)&(pCurPipeInstNode->pipeInst), 0, NULL);
         if(!pCurPipeInstNode->pipeInst.pipeListenThreadHandle)
         {
            pCurPipeInstNode->pipeInst.pipeListenThreadRun = FALSE;
            bRet = FALSE;
            break;
         }

         pCurPipeInstNode->pipeInst.pipeRecvThreadRun = TRUE;
         pCurPipeInstNode->pipeInst.pipeRecvThreadHandle = CreateThread(NULL, 0, PipeInstRecvThreadStart, (LPVOID)&(pCurPipeInstNode->pipeInst), 0, NULL);
         if(!pCurPipeInstNode->pipeInst.pipeRecvThreadHandle)
         {
            pCurPipeInstNode->pipeInst.pipeRecvThreadRun = FALSE;
            bRet = FALSE;
            break;
         }
         pCurPipeInstNode = pCurPipeInstNode->next;
      }
   }
   return bRet;
}

static BOOL UnlaunchPipeInst(PIPEINSTLIST pipeInstList)
{
   BOOL bRet = FALSE;
   if(pipeInstList == NULL) return bRet;
   {
      PPIPEINSTNODE  pCurPipeInstNode = pipeInstList->next;
      while(pCurPipeInstNode)
      {
         if(pCurPipeInstNode->pipeInst.pipeListenThreadHandle)
         {
            pCurPipeInstNode->pipeInst.pipeListenThreadRun = FALSE;
            WaitForSingleObject(pCurPipeInstNode->pipeInst.pipeListenThreadHandle, INFINITE);
            pCurPipeInstNode->pipeInst.pipeListenThreadHandle = NULL;
         }

         if(pCurPipeInstNode->pipeInst.pipeRecvThreadHandle)
         {
            pCurPipeInstNode->pipeInst.pipeRecvThreadRun = FALSE;
            WaitForSingleObject(pCurPipeInstNode->pipeInst.pipeRecvThreadHandle, INFINITE);
            pCurPipeInstNode->pipeInst.pipeRecvThreadHandle = NULL;
         }
         pCurPipeInstNode = pCurPipeInstNode->next;
      }
      bRet = TRUE;
   }
   return bRet;
}

static BOOL NamedPipeServerGNSend(IPCMSGTYPE msgType, DWORD commID, char * sendData, DWORD sendToLen)
{
   BOOL bRet = FALSE;
   if(PipeInstList == NULL || commID <= 0 || sendData == NULL || sendToLen <= 0) return bRet;
   {
      DWORD sendLen = 0;
      PPIPEINSTNODE findNode = FindPipeInstNodeWithCommID(PipeInstList, commID);
      if(findNode)
      {
         if(findNode->pipeInst.hPipe && findNode->pipeInst.isConnected)
         {
            DWORD realSendToLen = sizeof(IPCMSG) + sendToLen;
            PIPCMSG pIPCMsg = (PIPCMSG)malloc(realSendToLen);
            memset(pIPCMsg, 0, realSendToLen);
            pIPCMsg->msgType = msgType;
            pIPCMsg->msgContentLen = sendToLen;
            memcpy(pIPCMsg->msgContent, sendData, sendToLen);
            bRet = WriteFile(findNode->pipeInst.hPipe, (char *)pIPCMsg, realSendToLen, &sendLen, NULL);
            free(pIPCMsg);
         }
      }
   }
   return bRet;
}
int NamedPipeServerInit(char * pName, DWORD instCnt)
{
   BOOL bRet = FALSE;
   DWORD i = 0;
   if(pName == NULL || instCnt <= 0) return bRet;
   PipeInstList = CreatePipeInstList();
   if(PipeInstList == NULL) return bRet;

   for(i = 0; i<instCnt; i++)
   {
      HANDLE hPipe = NULL;
      hPipe = CreateNamedPipe(pName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT, 
         PIPE_UNLIMITED_INSTANCES, DEF_IPC_OUTPUT_BUF_SIZE, DEF_IPC_INPUT_BUF_SIZE, 0, NULL);
      if(hPipe == INVALID_HANDLE_VALUE) goto done;
      InsertPipeInstNode(PipeInstList, hPipe);
   }
   bRet = LaunchPipeInst(PipeInstList);
done:
   if(!bRet) NamedPipeServerUninit();
   return bRet;
}

void NamedPipeServerUninit()
{
   UnlaunchPipeInst(PipeInstList);
   DestroyPipeInstList(PipeInstList);
}

int IsChannelConnect(DWORD commID)
{
   BOOL bRet = FALSE;
   PPIPEINSTNODE findNode = NULL;
   if(PipeInstList == NULL) return bRet;
   findNode = FindPipeInstNodeWithCommID(PipeInstList, commID);
   if(findNode)
   {
      bRet = findNode->pipeInst.isConnected;
   }
   return bRet;
}

int NamedPipeServerRegRecvCB(DWORD commID, PPIPERECVCB pipeRecvCB)
{
   BOOL bRet = FALSE;
   if(pipeRecvCB == NULL || PipeInstList == NULL) return bRet;
   if(commID > 0)
   {
      PPIPEINSTNODE findNode = FindPipeInstNodeWithCommID(PipeInstList, commID);
      if(findNode)
      {
         findNode->pipeInst.pipeRecvCB = pipeRecvCB;
         bRet = TRUE;
      }
   }
   else
   {
      PPIPEINSTNODE curNode = PipeInstList->next;
      while(curNode)
      {
         curNode->pipeInst.pipeRecvCB = pipeRecvCB;
         curNode = curNode->next;
      }
      bRet = TRUE;
   }
   return bRet;
}

int NamedPipeServerSend(DWORD commID, char * sendData, DWORD sendToLen)
{
   BOOL bRet = FALSE;
   bRet = NamedPipeServerGNSend(USER_MSG, commID, sendData, sendToLen);
   return bRet;
}