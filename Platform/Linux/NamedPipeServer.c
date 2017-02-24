#include "NamedPipeServer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "IPCCommData.h"

typedef struct PipeInstance{
   int hPipe;
   unsigned long commID;
   bool isConnected;
   pthread_t pipeListenThreadHandle;
   bool   pipeListenThreadRun;
   pthread_t pipeRecvThreadHandle;
   bool   pipeRecvThreadRun;
   PPIPERECVCB pipeRecvCB;
}PIPEINST, *PPIPEINST;

typedef struct PipeInstNode * PPIPEINSTNODE;

typedef struct PipeInstNode{
   PIPEINST  pipeInst;
   PPIPEINSTNODE next;
}PIPEINSTNODE;

typedef PPIPEINSTNODE PIPEINSTLIST;

PIPEINSTLIST  PipeInstList = NULL;
char*		  FIFOName = NULL;
//----------------------------Watch object list function define--------------------------
static PIPEINSTLIST CreatePipeInstList();
static void DestroyPipeInstList(PIPEINSTLIST pipeInstList);
static PPIPEINSTNODE InsertPipeInstNode(PIPEINSTLIST pipeInstList, int hPipe);
static PPIPEINSTNODE FindPipeInstNodeWithCommID(PIPEINSTLIST pipeInstList, unsigned long commID);
static PPIPEINSTNODE FindPipeInstNodeWithHandle(PIPEINSTLIST pipeInstList, int hPipe);
static bool DeleteAllPipeInstNode(PIPEINSTLIST pipeInstList);
//---------------------------------------------------------------------------------------

//---------------------------Other function define---------------------------------------
static bool LaunchPipeInst(PIPEINSTLIST pipeInstList);
static bool UnlaunchPipeInst(PIPEINSTLIST pipeInstList);
//---------------------------------------------------------------------------------------

static PIPEINSTLIST CreatePipeInstList()
{
   PPIPEINSTNODE head = NULL;
   head = (PPIPEINSTNODE)malloc(sizeof(PIPEINSTNODE));
   if(head) 
   {
      head->next = NULL;
      head->pipeInst.commID = 0;
      head->pipeInst.hPipe = -1;
      head->pipeInst.isConnected = false;
      head->pipeInst.pipeListenThreadHandle = 0;
      head->pipeInst.pipeListenThreadRun = false;
      head->pipeInst.pipeRecvThreadHandle = 0;
      head->pipeInst.pipeRecvThreadRun = false;
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

static PPIPEINSTNODE InsertPipeInstNode(PIPEINSTLIST pipeInstList, int hPipe)
{
   PPIPEINSTNODE pRet = NULL;
   PPIPEINSTNODE newNode = NULL;
   PPIPEINSTNODE findNode = NULL;
   PPIPEINSTNODE head = pipeInstList;
   if(head == NULL || hPipe == -1) return pRet;
   findNode = FindPipeInstNodeWithHandle(head, hPipe);
   if(findNode == NULL)
   {
      newNode = (PPIPEINSTNODE)malloc(sizeof(PIPEINSTNODE));
      newNode->pipeInst.commID = 0;
      newNode->pipeInst.hPipe = hPipe;
      newNode->pipeInst.isConnected = true;
      newNode->pipeInst.pipeListenThreadHandle = 0;
      newNode->pipeInst.pipeListenThreadRun = false;
      newNode->pipeInst.pipeRecvThreadHandle = 0;
      newNode->pipeInst.pipeRecvThreadRun = false;
      newNode->pipeInst.pipeRecvCB = NULL;

      newNode->next = head->next;
      head->next = newNode;
      pRet = newNode;
   }
   return pRet;
}

static PPIPEINSTNODE FindPipeInstNodeWithCommID(PIPEINSTLIST pipeInstList, unsigned long commID)
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

static PPIPEINSTNODE FindPipeInstNodeWithHandle(PIPEINSTLIST pipeInstList, int hPipe)
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

static bool DeleteAllPipeInstNode(PIPEINSTLIST pipeInstList)
{
   bool bRet = false;
   PPIPEINSTNODE delNode = NULL;
   PPIPEINSTNODE head = pipeInstList;
   if(head == NULL) return bRet;

   delNode = head->next;
   while(delNode)
   {
      head->next = delNode->next;

      if(delNode->pipeInst.pipeRecvThreadHandle)
      {
         delNode->pipeInst.pipeRecvThreadRun = false;
         pthread_join(delNode->pipeInst.pipeRecvThreadHandle, NULL);
         delNode->pipeInst.pipeRecvThreadHandle = 0;
      }

      if(delNode->pipeInst.hPipe)
      {
         close(delNode->pipeInst.hPipe);
         delNode->pipeInst.hPipe = -1;
      }

      free(delNode);
      delNode = head->next;
   }

   bRet = true;
   return bRet;
}

static void* thread_pipe_recv(void *args)
{
   PPIPEINST  pPipeInst = (PPIPEINST)args;
   char recvBuf[DEF_IPC_OUTPUT_BUF_SIZE] = {0};
   long recvCnt = 0;
   while(pPipeInst->pipeRecvThreadRun)
   {
      if(pPipeInst->hPipe && pPipeInst->isConnected)
      {
         if(recvCnt > 0)
         {
            memset(recvBuf, 0, sizeof(recvBuf));
            recvCnt = 0;
         }
         recvCnt = read(pPipeInst->hPipe, recvBuf, sizeof(recvBuf));
         if(recvCnt > 0)
         {
            PIPCMSG pIPCMsg = (PIPCMSG)recvBuf;
            switch(pIPCMsg->msgType)
            {
            case INTER_MSG:
               {
                  IPCINTERMSG interMsg;
                  printf("read INTER_MSG: %d\r\n", recvCnt);
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
                  unsigned long userRecvCnt = pIPCMsg->msgContentLen;
		  printf("read USER_MSG: %d\r\n", recvCnt);
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
      usleep(10*1000);
   }
   return 0;
}

static bool LaunchPipeInst(PIPEINSTLIST pipeInstList)
{
   bool bRet = true;
   if(pipeInstList == NULL) return false;
   {
      PPIPEINSTNODE  pCurPipeInstNode = pipeInstList->next;
      while(pCurPipeInstNode)
      {
         pCurPipeInstNode->pipeInst.pipeRecvThreadRun = true;
	 if (pthread_create(&pCurPipeInstNode->pipeInst.pipeRecvThreadHandle, NULL, thread_pipe_recv, &pCurPipeInstNode->pipeInst) != 0)
         {
            pCurPipeInstNode->pipeInst.pipeRecvThreadRun = false;
            bRet = false;
            break;
         }
         pCurPipeInstNode = pCurPipeInstNode->next;
      }
   }
   return bRet;
}

static bool UnlaunchPipeInst(PIPEINSTLIST pipeInstList)
{
   bool bRet = false;
   if(pipeInstList == NULL) return bRet;
   {
      PPIPEINSTNODE  pCurPipeInstNode = pipeInstList->next;
      while(pCurPipeInstNode)
      {
         if(pCurPipeInstNode->pipeInst.pipeRecvThreadHandle)
         {
            pCurPipeInstNode->pipeInst.pipeRecvThreadRun = false;
	    pthread_join(pCurPipeInstNode->pipeInst.pipeRecvThreadHandle, NULL);
            pCurPipeInstNode->pipeInst.pipeRecvThreadHandle = 0;
         }
         pCurPipeInstNode = pCurPipeInstNode->next;
      }
      bRet = true;
   }
   return bRet;
}

int NamedPipeServerInit(char * pName, unsigned long instCnt)
{
   bool bRet = false;
   int hPipe = -1;
   if(pName == NULL || instCnt <= 0) return bRet;
   PipeInstList = CreatePipeInstList();
   if(PipeInstList == NULL) return bRet;
   FIFOName = strdup(pName);   
   mkfifo(pName, 0666);
   hPipe = open(pName, O_NONBLOCK|O_RDONLY);
   InsertPipeInstNode(PipeInstList, hPipe);
   bRet = LaunchPipeInst(PipeInstList);
   if(!bRet) NamedPipeServerUninit();
   return bRet;
}

void NamedPipeServerUninit()
{
   UnlaunchPipeInst(PipeInstList);
   DestroyPipeInstList(PipeInstList);
   unlink(FIFOName);
   free(FIFOName);
   FIFOName = NULL;
}

int IsChannelConnect(unsigned long commID)
{
   bool bRet = false;
   PPIPEINSTNODE findNode = NULL;
   if(PipeInstList == NULL) return bRet;
   findNode = FindPipeInstNodeWithCommID(PipeInstList, commID);
   if(findNode)
   {
      bRet = findNode->pipeInst.isConnected;
   }
   return bRet;
}

int NamedPipeServerRegRecvCB(unsigned long commID, PPIPERECVCB pipeRecvCB)
{
   bool bRet = false;
   if(pipeRecvCB == NULL || PipeInstList == NULL) return bRet;
   if(commID > 0)
   {
      PPIPEINSTNODE findNode = FindPipeInstNodeWithCommID(PipeInstList, commID);
      if(findNode)
      {
         findNode->pipeInst.pipeRecvCB = pipeRecvCB;
         bRet = true;
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
      bRet = true;
   }
   return bRet;
}
