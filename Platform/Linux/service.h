#ifndef _WIN_SERVICE_H_
#define _WIN_SERVICE_H_

#define DEF_SRVC_LOG_FILE_NAME       "SUSIAgentServiceLog.txt"
#define SERVICE_CONTROL_USER         128
#define DEF_SERVICE_NAME             "AgentService"
#define MAX_CMD_LEN                  32
#define DEF_SERVICE_VERSION          "1.0"
#define HELP_SERVICE_CMD             "-h"
#define DSVERSION_SERVICE_CMD        "-v"
#define INSTALL_SERVICE_CMD          "-i"
#define UNINSTALL_SERVICE_CMD        "-u"
#define QUIT_CMD                     "-q"
#define NSRV_RUN_CMD                 "-n"
#define STOP_NSER_RUN_CMD            "-s"

typedef int (*APP_START_CB) ();
typedef int (*APP_STOP_CB) ();

int ServiceInit(char * pSrvcName, char * pVersion, APP_START_CB pStart, APP_STOP_CB pStop, void * loghandle);
int ServiceUninit();
int LaunchService();
int ExecuteCmd(char* cmd);

#endif
