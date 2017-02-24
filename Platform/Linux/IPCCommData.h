#ifndef _IPC_COMM_DATA_H_
#define _IPC_COMM_DATA_H_

#define  DEF_IPC_OUTPUT_BUF_SIZE  (4096)
#define  DEF_IPC_INPUT_BUF_SIZE   (4096)

typedef enum IPCMsgType{
   USER_MSG,
   INTER_MSG,
}IPCMSGTYPE;

typedef struct IPCMessage{
   IPCMSGTYPE msgType;
   int  msgContentLen;
   char msgContent[];
}IPCMSG, *PIPCMSG;

typedef enum InterCmdKey{
   IPC_CONNECT,
}INTERCMDKEY;

typedef union InterParams{
   unsigned int commID;
}INTERPARAMS, *PINTERPARAMS;

typedef struct IPCInterMsg{
   INTERCMDKEY  interCmdKey;
   INTERPARAMS  interParams;
}IPCINTERMSG, *PIPCINTERMSG;

#endif