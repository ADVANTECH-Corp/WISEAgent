/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.														 */
/* Create Date  : 2015 by Zach Chih															     */
/* Modified Date: 2016/3/9 by Zach Chih															 */
/* Abstract     : Modbus Handler                                   													*/
/* Reference    : None																									 */
/****************************************************************************/
#include "Modbus_Handler.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include "wrapper.h"
#include "pthread.h"
#include "util_path.h"
#include "unistd.h"
#include "susiaccess_handler_api.h"
#include "IoTMessageGenerate.h"
#include "Modbus_HandlerLog.h"
#include "Modbus_Parser.h"
#include "ReadINI.h"


//-----------------------------------------------------------------------------
//#############################################################################
//symbol correspondence
//Code	:	Modbus Spec	in INI File
//DI	:	Discrete Inputs		-->IB
//DO	:	Coils				-->B
//AI	:	Input Registers		-->IR
//AO	:	Holding Registers	-->R
//#############################################################################
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------------------

//#define DEF_HANDLER_NAME	"Modbus_Handler"
#define DEF_MODBUS_TCP		"Modbus_TCP"
#define DEF_MODBUS_RTU		"Modbus_RTU"
#define DEF_SIMULATOR_NAME	"Simulator"

//#define INI_PATH "\Modbus.ini"

#define cagent_request_custom 2002
#define cagent_custom_action 30002
#define SET_STR_LENGTH 200

/*
//----------------------------------------------Customize
#define BARCODE_NUM 15
#define BARCODE_REG_NUM 7
//----------------------------------------------Customize_end
*/

//-----------------------------------------------------------------------------
//----------------------sensor info item list function define------------------
//-----------------------------------------------------------------------------
static sensor_info_list CreateSensorInfoList();
static void DestroySensorInfoList(sensor_info_list sensorInfoList);
static int InsertSensorInfoNode(sensor_info_list sensorInfoList, sensor_info_t * pSensorInfo);
static sensor_info_node_t * FindSensorInfoNodeWithID(sensor_info_list sensorInfoList, int id);
static int DeleteSensorInfoNodeWithID(sensor_info_list sensorInfoList, int id);
static int DeleteAllSensorInfoNode(sensor_info_list sensorInfoList);
static bool IsSensorInfoListEmpty(sensor_info_list sensorInfoList);
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Internal Prototypes:
//-----------------------------------------------------------------------------
//

typedef struct{
   pthread_t threadHandler;
   bool isThreadRunning;
}handler_context_t;



typedef struct report_data_params_t{
	unsigned int intervalTimeMs;
	unsigned int continueTimeMs;
	char repFilter[4096];
}report_data_params_t;

static report_data_params_t AutoUploadParams;
static report_data_params_t AutoReportParams;


/*
//----------------------------------------------Customize
char ascii_array[43] = {'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
int barflag=0; // 0-> no barcode; 1-> AI barcode; 2->AO barcode;
//----------------------------------------------Customize_end
*/
//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
static void* g_loghandle = NULL;

static Handler_info  g_PluginInfo;
static handler_context_t g_RetrieveContex;
static handler_context_t g_AutoReportContex;
static handler_context_t g_AutoUploadContex;
//clock_t AutoUpload_start;
//clock_t AutoUpload_continue;
static HANDLER_THREAD_STATUS g_status = handler_status_no_init;
static bool g_bRetrieve = false;
static bool g_bAutoReport = false;
static bool g_bAutoUpload = false;
static time_t g_monitortime;
static HandlerSendCbf  g_sendcbf = NULL;						// Client Sends information (in JSON format) to Cloud Server	
static HandlerSendCustCbf  g_sendcustcbf = NULL;			    // Client Sends information (in JSON format) to Cloud Server with custom topic	
static HandlerSubscribeCustCbf g_subscribecustcbf = NULL;
static HandlerAutoReportCbf g_sendreportcbf = NULL;				// Client Sends report (in JSON format) to Cloud Server with AutoReport topic
static HandlerSendCapabilityCbf g_sendcapabilitycbf = NULL;		
static HandlerSendEventCbf g_sendeventcbf = NULL;

static char * CurIotDataJsonStr = NULL;
//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
//const char strPluginName[MAX_TOPIC_LEN] = {"Modbus_Handler"};
char *strPluginName = NULL;										//be used to set the customized handler name by module_config.xml
const int iRequestID = cagent_request_custom;
const int iActionID = cagent_custom_action;
MSG_CLASSIFY_T *g_Capability = NULL;
bool bAllDataAlloc=false;
int Rev_Fail_Num=0;
bool g_bRev_Fail=false;

bool bConnectionFlag=false;
bool bIsSimtor=false;
bool bFind=false; //Find INI file
int  iTCP_RTU=0;
char Modbus_Protocol[20]="";
char Device_Name[20]="";
bool Modbus_Log=false; //Log
char *MRLog_path;
char *MSLog_path;
FILE *pMRLog=NULL;
FILE *pMSLog=NULL;
//--------Modbus_TCP
char Modbus_Clent_IP[16]=""; 
int Modbus_Client_Port=502;
int Modbus_UnitID=1;
//--------Modbus_RTU
char Modbus_Slave_Port[6]=""; 
int Modbus_Baud=19200;
char Modbus_Parity[5];
int Modbus_DataBits=8;
int Modbus_StopBits=1;
int Modbus_SlaveID=1;
//--------
modbus_t *ctx=NULL;
int numberOfDI=0;
int numberOfAI=0;
int numberOfDO=0;
int numberOfAO=0;
//--------Floating real number of registers
uint16_t *AI_Regs_temp_Ary;
uint16_t *AO_Regs_temp_Ary;
uint16_t *AO_Set_Regs_temp_Ary;
int numberOfAI_Regs=0;
int numberOfAO_Regs=0;

WISE_Data *DI;
WISE_Data *AI;
WISE_Data *DO;
WISE_Data *AO;
uint8_t *DI_Bits;
uint8_t  *DO_Bits;
uint16_t *AI_Regs;
uint16_t *AO_Regs;
time_t Start_Upload;
time_t Continue_Upload;
//-----------------------------------------------------------------------------
// AutoReport:
//-----------------------------------------------------------------------------
cJSON *Report_root=NULL;
cJSON *Stop_Report_root=NULL;
int Report_array_size=0, Report_interval=0;
bool Report_Reply_All=false;
char **Report_Data_paths=NULL;

cJSON *Report_item, *Report_it, *Report_js_name, *Report_first,*Report_second_interval,*Report_second,*Report_third,*Report_js_list;

//-----------------------------------------------------------------------------
// Function:
//-----------------------------------------------------------------------------
char* itobs(uint32_t n, char *ps) {
   int size = 8*sizeof(n);
   int i = size -1;
   
   while(i+1) {
     ps[i--] = (1 & n) + '0';
     n >>= 1;
   }
   ps[size] = '\0';
   return ps;
 }

//fix modbus_get_float (CDAB) to custom_get_float(ABCD) work! -->sw_mode = 1
static float custom_get_float(uint16_t *src)
{

    uint16_t fix_bytes_order[] = { src[1], src[0] };

    return modbus_get_float(fix_bytes_order);

}
//fix modbus_set_float (CDAB) to custom_set_float(ABCD)	work! -->sw_mode = 1
static void custom_set_float(float fv, uint16_t *src)
{
	uint16_t temp;

    modbus_set_float(fv,src);
	temp=src[1];
	src[1]=src[0];
	src[0]=temp;

}
//fix modbus_get_float_dcba to custom_get_float_dcba(DCBA) work! -->sw_mode = 2
static float custom_get_float_dcba(uint16_t *src)
{

	uint16_t fix_bytes_order[] = { src[1], src[0] };
	//printf("%d %d\n",src[1],src[0]);
    return modbus_get_float_dcba(fix_bytes_order);

}
//fix modbus_set_float_dcba to custom_set_float_dcba(DCBA) work! -->sw_mode = 2
static void custom_set_float_dcba(float fv, uint16_t *src)
{
	uint16_t temp;

    modbus_set_float_dcba(fv,src);
	temp=src[1];
	src[1]=src[0];
	src[0]=temp;


}


//custom_get_float_badc(BADC) work!	-->sw_mode = 3
static float custom_get_float_badc(uint16_t *src)
{
    float f;
    uint32_t i;
	//char s[8 * sizeof(i) + 1];
   
	i = bswap_32((((uint32_t)src[1]) << 16) + src[0]);
	memcpy(&f, &i, sizeof(float));
	//printf("%d = %s\n", i, itobs(i,s));

    return f;

}
//custom_set_float_badc(BADC) work! -->sw_mode = 3
static void custom_set_float_badc(float fv, uint16_t *src)
{
    uint32_t i;

    memcpy(&i, &fv, sizeof(uint32_t));
    i = bswap_32(i);
    src[0] = (uint16_t)i;
    src[1] = (uint16_t)(i >> 16);
	//printf("%d\n",src[0]);
	//printf("%d\n",src[1]);

}

//custom_get_float_cdab(CDAB) work! -->sw_mode = 4
static float custom_get_float_cdab(uint16_t *src) 
{
    float f;
    uint32_t i;
	char s[8 * sizeof(i) + 1];

    i = (((uint32_t)src[1]) << 16) + src[0];
    memcpy(&f, &i, sizeof(float));
	//printf("%d = %s\n", i, itobs(i,s));
    return f;
}

//custom_set_float_cdab(CDAB) work! -->sw_mode = 4
static void custom_set_float_cdab(float fv, uint16_t *src)
{
    uint32_t i;

    memcpy(&i, &fv, sizeof(uint32_t));
    src[0] = (uint16_t)i;
    src[1] = (uint16_t)(i >> 16);
	//printf("%d\n",src[0]);
	//printf("%d\n",src[1]);

}


//custom_get_unsigned_int(ABCD) work! -->sw_mode = 5
static uint32_t custom_get_unsigned_int(uint16_t *src)
{

    uint16_t fix_bytes_order[] = { src[1], src[0] };
	uint32_t i;
	i = (((uint32_t)src[0]) << 16) + src[1];

    return i;

}

//custom_set_unsigned_int(ABCD) work! -->sw_mode = 5
static void custom_set_unsigned_int(uint32_t uiv, uint16_t *src)
{
    uint32_t i;

    memcpy(&i, &uiv, sizeof(uint32_t));
    src[1] = (uint16_t)i;
    src[0] = (uint16_t)(i >> 16);

}

//custom_get_unsigned_int_cdab(CDAB) work! -->sw_mode = 6
static uint32_t custom_get_unsigned_int_cdab(uint16_t *src)
{

    uint16_t fix_bytes_order[] = { src[1], src[0] };
	uint32_t i;
	i = (((uint32_t)src[1]) << 16) + src[0];

    return i;

}
//custom_set_unsigned_int_cdab(CDAB) work! -->sw_mode = 6
static void custom_set_unsigned_int_cdab(uint32_t uiv, uint16_t *src)
{
    uint32_t i;

    memcpy(&i, &uiv, sizeof(uint32_t));
    src[0] = (uint16_t)i;
    src[1] = (uint16_t)(i >> 16);

}
//custom_get_int(ABCD) work! -->sw_mode = 7
static int custom_get_int(uint16_t *src)
{

    uint16_t fix_bytes_order[] = { src[1], src[0] };
	int i;
	i = (((uint32_t)src[0]) << 16) + src[1];

    return i;

}
//custom_set_int(ABCD) work! -->sw_mode = 7
static void custom_set_int(int iv, uint16_t *src)
{
    uint32_t i;

    memcpy(&i, &iv, sizeof(uint32_t));
    src[1] = (uint16_t)i;
    src[0] = (uint16_t)(i >> 16);

}
//custom_get_int_cdab(CDAB) work! -->sw_mode = 8
static int custom_get_int_cdab(uint16_t *src)
{

    uint16_t fix_bytes_order[] = { src[1], src[0] };
	int i;
	i = (((uint32_t)src[1]) << 16) + src[0];

    return i;

}
//custom_set_int_cdab(CDAB) work! -->sw_mode = 8
static void custom_set_int_cdab(int iv, uint16_t *src)
{
    uint32_t i;

    memcpy(&i, &iv, sizeof(uint32_t));
    src[0] = (uint16_t)i;
    src[1] = (uint16_t)(i >> 16);

}
void Handler_Uninitialize();
bool Modbus_Rev();
#ifdef _MSC_VER
BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
	if (reason_for_call == DLL_PROCESS_ATTACH) // Self-explanatory
	{
		printf("DllInitializer\r\n");
		DisableThreadLibraryCalls(module_handle); // Disable DllMain calls for DLL_THREAD_*
		if (reserved == NULL) // Dynamic load
		{
			// Initialize your stuff or whatever
			// Return FALSE if you don't want your module to be dynamically loaded
		}
		else // Static load
		{
			// Return FALSE if you don't want your module to be statically loaded
			return FALSE;
		}
	}

	if (reason_for_call == DLL_PROCESS_DETACH) // Self-explanatory
	{
		printf("DllFinalizer\r\n");
		if (reserved == NULL) // Either loading the DLL has failed or FreeLibrary was called
		{
			// Cleanup
			Handler_Uninitialize();
		}
		else // Process is terminating
		{
			// Cleanup
			Handler_Uninitialize();
		}
	}
	return TRUE;
}
#else
__attribute__((constructor))
/**
 * initializer of the shared lib.
 */
static void Initializer(int argc, char** argv, char** envp)
{
    printf("DllInitializer\r\n");
}

__attribute__((destructor))
/** 
 * It is called when shared lib is being unloaded.
 * 
 */
static void Finalizer()
{
    printf("DllFinalizer\r\n");
	Handler_Uninitialize();
}
#endif

static sensor_info_list CreateSensorInfoList()
{
	sensor_info_node_t * head = NULL;
	head = (sensor_info_node_t *)malloc(sizeof(sensor_info_node_t));
	if(head)
	{
		memset(head, 0, sizeof(sensor_info_node_t));
		head->next = NULL;
	}
	return head;
}

static void DestroySensorInfoList(sensor_info_list sensorInfoList)
{
	if(NULL == sensorInfoList) return;
	DeleteAllSensorInfoNode(sensorInfoList);
	free(sensorInfoList); 
	sensorInfoList = NULL;
}

static int DeleteAllSensorInfoNode(sensor_info_list sensorInfoList)
{
	int iRet = -1;
	sensor_info_node_t * delNode = NULL, *head = NULL;
	int i = 0;
	if(sensorInfoList == NULL) return iRet;
	head = sensorInfoList;
	delNode = head->next;
	while(delNode)
	{
		head->next = delNode->next;
		if(delNode->sensorInfo.jsonStr != NULL)
		{
			free(delNode->sensorInfo.jsonStr);
			delNode->sensorInfo.jsonStr = NULL;
		}
		if(delNode->sensorInfo.pathStr != NULL)
		{
			free(delNode->sensorInfo.pathStr);
			delNode->sensorInfo.pathStr = NULL;
		}
		free(delNode);
		delNode = head->next;
	}

	iRet = 0;
	return iRet;
}

//==========================================================
//read platform section in INI file
bool read_INI_Platform(char *modulePath,char *iniPath)
{

	FILE *fPtr;
	char *temp_INI_name=NULL;
	
	temp_INI_name=(char *)calloc(strlen(strPluginName)+1+4,sizeof(char));	//+4 for ".ini"
	strcpy(temp_INI_name,strPluginName);
	strcat(temp_INI_name,".ini");
	// Load ini file
	util_module_path_get(modulePath);
	util_path_combine(iniPath,modulePath,temp_INI_name);

    fPtr = fopen(iniPath, "r");
    if (fPtr) {
        printf("INI Opened Successful...\n");

		strcpy(Modbus_Protocol,GetIniKeyString("Platform","Protocol",iniPath));
		strcpy(Device_Name,GetIniKeyString("Platform","Name",iniPath));

		if(strcmp(Modbus_Protocol,DEF_MODBUS_TCP)==0)
		{	
			printf("Protocol : Modbus_TCP\n");
			iTCP_RTU=0;
			
			strcpy(Modbus_Clent_IP,GetIniKeyString("Platform","ClientIP",iniPath));
			Modbus_Client_Port=GetIniKeyInt("Platform","ClientPort",iniPath);
			Modbus_UnitID=GetIniKeyInt("Platform","UnitID",iniPath);
			if(GetIniKeyInt("Platform","Log",iniPath))
				Modbus_Log=true;
			else
				Modbus_Log=false;
		}
		else if(strcmp(Modbus_Protocol,DEF_MODBUS_RTU)==0)
		{
			printf("Protocol : Modbus_RTU\n");	
			iTCP_RTU=1;

			strcpy(Modbus_Slave_Port,GetIniKeyString("Platform","SlavePort",iniPath));
			Modbus_Baud=GetIniKeyInt("Platform","Baud",iniPath);
			strcpy(Modbus_Parity,GetIniKeyString("Platform","Parity",iniPath));
			Modbus_DataBits=GetIniKeyInt("Platform","DataBits",iniPath);
			Modbus_StopBits=GetIniKeyInt("Platform","StopBits",iniPath);
			Modbus_SlaveID=GetIniKeyInt("Platform","SlaveID",iniPath);
			if(GetIniKeyInt("Platform","Log",iniPath))
				Modbus_Log=true;
			else
				Modbus_Log=false;
		}
		else
		{
			printf("Protocol error!!\n");
			iTCP_RTU=-1;	
		}

		if(strcmp(Device_Name,DEF_SIMULATOR_NAME)==0)
		{
			printf("bIsSimtor=true;\n");		
			bIsSimtor=true;
		}
		else
		{
			printf("bIsSimtor=false;\n");	
			bIsSimtor=false;
		}

        fclose(fPtr);
		free(temp_INI_name);
		return true;
    }
    else {
        printf("INI Opened Failed...\n");
		free(temp_INI_name);
		return false;
    }


}

//read other sections except platform in INI file
bool read_INI()
{
	
	char modulePath[200]={0};
	char iniPath[200]={0};
	char str[100]={0};
	char *pstr;

	FILE *fPtr;

	char *temp_INI_name=NULL;
	
	temp_INI_name=(char *)calloc(strlen(strPluginName)+1+4,sizeof(char));	//+4 for ".ini"
	strcpy(temp_INI_name,strPluginName);
	strcat(temp_INI_name,".ini");
	// Load ini file
	util_module_path_get(modulePath);
	util_path_combine(iniPath,modulePath,temp_INI_name);

    fPtr = fopen(iniPath, "r");

	if (fPtr) {				
									printf("INI Opened Successful...\n");
									#pragma region Find_INI
									//--------------------DI
									numberOfDI=GetIniKeyInt("Discrete Inputs","numberOfIB",iniPath);
									if(numberOfDI!=0)
									{	
										DI_Bits=(uint8_t *)calloc(numberOfDI,sizeof(uint8_t));
										DI=(WISE_Data *)calloc(numberOfDI,sizeof(WISE_Data));
									}
									for(int i=0;i<numberOfDI;i++){
										char strNumberOfDI[10];
										DI[i].Regs=0;
										DI[i].Bits=0;
										DI[i].fv=0;
										DI[i].pre_fv=0;
										DI[i].uiv=0;
										DI[i].pre_uiv=0;
										DI[i].iv=0;
										DI[i].pre_iv=0;
										DI[i].bRevFin=false;		

										sprintf(strNumberOfDI, "IB%d", i);
		
										strcpy(str,GetIniKeyString("Discrete Inputs",strNumberOfDI,iniPath));

										pstr=strtok(str, ",");
										if (pstr!=NULL)
											DI[i].address = atoi(pstr);//get the address
										else
											DI[i].address=0;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											strcpy(DI[i].name,pstr); // get the name
										else
											strcpy(DI[i].name,""); 

										printf("%d. DI address: %d name: %s\n",i,DI[i].address,DI[i].name);
									}

									//--------------------DO

									numberOfDO=GetIniKeyInt("Coils","numberOfB",iniPath);
									if(numberOfDO!=0)
									{
										DO_Bits=(uint8_t *)calloc(numberOfDO,sizeof(uint8_t));
										DO=(WISE_Data *)calloc(numberOfDO,sizeof(WISE_Data));	
									}
									for(int i=0;i<numberOfDO;i++){
										char strNumberOfDO[10];
										DO[i].Regs=0;
										DO[i].Bits=0;
										DO[i].fv=0;
										DO[i].pre_fv=0;
										DO[i].uiv=0;
										DO[i].pre_uiv=0;
										DO[i].iv=0;
										DO[i].pre_iv=0;
										DO[i].bRevFin=false;

										sprintf(strNumberOfDO, "B%d", i);
	
										strcpy(str,GetIniKeyString("Coils",strNumberOfDO,iniPath));

										pstr=strtok(str, ",");
										if (pstr!=NULL)
											DO[i].address = atoi(pstr);//get the address
										else
											DO[i].address=0;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											strcpy(DO[i].name,pstr); // get the name
										else
											strcpy(DO[i].name,""); 

										printf("%d. DO address: %d name: %s\n",i,DO[i].address,DO[i].name);
									}

									//--------------------AI
									pstr=NULL;

									numberOfAI=GetIniKeyInt("Input Registers","numberOfIR",iniPath);
									numberOfAI_Regs=numberOfAI;
									if(numberOfAI!=0)
									{	
										AI=(WISE_Data *)calloc(numberOfAI,sizeof(WISE_Data));
									}
									for(int i=0;i<numberOfAI;i++){
										char strNumberOfAI[10];
										double tmp;
										AI[i].Regs=0;
										AI[i].Bits=0;
										AI[i].fv=0;
										AI[i].pre_fv=0;
										AI[i].uiv=0;
										AI[i].pre_uiv=0;
										AI[i].iv=0;
										AI[i].pre_iv=0;
										AI[i].bRevFin=false;

										sprintf(strNumberOfAI, "IR%d", i);
	
										strcpy(str,GetIniKeyString("Input Registers",strNumberOfAI,iniPath));

										pstr=strtok(str, ",");
										if (pstr!=NULL)
											AI[i].address = atoi(pstr);//get the address
										else
											AI[i].address=0;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											strcpy(AI[i].name,pstr); // get the name
										else
											strcpy(AI[i].name,""); 

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AI[i].min=atof(pstr); // get the min
										else
											AI[i].min=0;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AI[i].max=atof(pstr); // get the max
										else
											AI[i].max=0; 

										if(AI[i].min>AI[i].max)
										{
											tmp=AI[i].min;
											AI[i].min=AI[i].max;
											AI[i].max=tmp;
										}

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AI[i].precision=atof(pstr); // get the precision
										else
											AI[i].precision=1; 

										if(AI[i].precision==0)
											AI[i].precision=1;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											strcpy(AI[i].unit,pstr); // get the unit
										else
											strcpy(AI[i].unit,""); 

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AI[i].sw_mode=atoi(pstr); // get the sw_mode
										else
											AI[i].sw_mode=0; 

										if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4||AI[i].sw_mode==5||AI[i].sw_mode==6||AI[i].sw_mode==7||AI[i].sw_mode==8)
											numberOfAI_Regs++;

										printf("%d. AI address: %d name: %s min: %lf max: %lf pre: %lf unit: %s sw_mode: %d\n",i,AI[i].address,AI[i].name,AI[i].min,AI[i].max,AI[i].precision,AI[i].unit,AI[i].sw_mode);
									}
									if(numberOfAI_Regs!=0)
										AI_Regs=(uint16_t *)calloc(numberOfAI_Regs,sizeof(uint16_t));

									//--------------------AO
									pstr=NULL;

									numberOfAO=GetIniKeyInt("Holding Registers","numberOfR",iniPath);
									numberOfAO_Regs=numberOfAO;
									if(numberOfAO!=0)
									{	
										AO=(WISE_Data *)calloc(numberOfAO,sizeof(WISE_Data));
									}
									for(int i=0;i<numberOfAO;i++){
										char strNumberOfAO[10];
										double tmp;
										AO[i].Regs=0;
										AO[i].Bits=0;
										AO[i].fv=0;
										AO[i].pre_fv=0;
										AO[i].uiv=0;
										AO[i].pre_uiv=0;
										AO[i].iv=0;
										AO[i].pre_iv=0;
										AO[i].bRevFin=false;

										sprintf(strNumberOfAO, "R%d", i);

										strcpy(str,GetIniKeyString("Holding Registers",strNumberOfAO,iniPath));

										pstr=strtok(str, ",");
										if (pstr!=NULL)
											AO[i].address = atoi(pstr);//get the address
										else
											AO[i].address=0;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											strcpy(AO[i].name,pstr); // get the name
										else
											strcpy(AO[i].name,""); 

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AO[i].min=atof(pstr); // get the min
										else
											AO[i].min=0;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AO[i].max=atof(pstr); // get the max
										else
											AO[i].max=0; 

										if(AO[i].min>AO[i].max)
										{
											tmp=AO[i].min;
											AO[i].min=AO[i].max;
											AO[i].max=tmp;
										}

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AO[i].precision=atof(pstr); // get the precision
										else
											AO[i].precision=1; 

										if(AO[i].precision==0)
											AO[i].precision=1;

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											strcpy(AO[i].unit,pstr); // get the unit
										else
											strcpy(AO[i].unit,""); 

										pstr=strtok(NULL, ",");
										if (pstr!=NULL)
											AO[i].sw_mode=atoi(pstr); // get the sw_mode
										else
											AO[i].sw_mode=0; 

										if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4||AO[i].sw_mode==5||AO[i].sw_mode==6||AO[i].sw_mode==7||AO[i].sw_mode==8)
											numberOfAO_Regs++;
										printf("%d. AO address: %d name: %s min: %lf max: %lf pre: %lf unit: %s sw_mode: %d\n",i,AO[i].address,AO[i].name,AO[i].min,AO[i].max,AO[i].precision,AO[i].unit,AO[i].sw_mode);
									}
									if(numberOfAO_Regs!=0)
										AO_Regs=(uint16_t *)calloc(numberOfAO_Regs,sizeof(uint16_t));	
									#pragma endregion Find_INI
									fclose(fPtr);
									free(temp_INI_name);
									return true;
								

	}
    else {
        printf("INI Opened Failed...\n");
		free(temp_INI_name);
		return false;
    }


}


//Prepare to Upload Data
void UpDataPrepare(WISE_Data *Data, bool bUAI_AO, int *AIORcur, bool ret)	//DI,DO already keep the latest data,so dont have to take care here.
{
	int i=0;

	if(bUAI_AO)
	{
			switch(Data->sw_mode)
			{
				case 1:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->fv=custom_get_float(AI_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 2:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->fv=custom_get_float_dcba(AI_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 3:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->fv=custom_get_float_badc(AI_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 4:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->fv=custom_get_float_cdab(AI_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 5:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->uiv=custom_get_unsigned_int(AI_Regs_temp_Ary);
						Data->pre_uiv=Data->uiv;
					}
					else
						Data->uiv=Data->pre_uiv;
					(*AIORcur)+=2;
					break;
				case 6:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->uiv=custom_get_unsigned_int_cdab(AI_Regs_temp_Ary);
						Data->pre_uiv=Data->uiv;
					}
					else
						Data->uiv=Data->pre_uiv;
					(*AIORcur)+=2;
					break;
				case 7:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->iv=custom_get_int(AI_Regs_temp_Ary);
						Data->pre_iv=Data->iv;
					}
					else
						Data->iv=Data->pre_iv;
					(*AIORcur)+=2;
					break;
				case 8:
					if(Data->bRevFin)
					{
						AI_Regs_temp_Ary[0]=AI_Regs[*AIORcur];
						AI_Regs_temp_Ary[1]=AI_Regs[*AIORcur+1];
						Data->iv=custom_get_int_cdab(AI_Regs_temp_Ary);
						Data->pre_iv=Data->iv;
					}
					else
						Data->iv=Data->pre_iv;
					(*AIORcur)+=2;
					break;
				default:
					Data->Regs=AI_Regs[*AIORcur];
					(*AIORcur)++;

			}

	}
	else
	{
			switch(Data->sw_mode)
			{
				case 1:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->fv=custom_get_float(AO_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 2:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->fv=custom_get_float_dcba(AO_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 3:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->fv=custom_get_float_badc(AO_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 4:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->fv=custom_get_float_cdab(AO_Regs_temp_Ary);
						Data->pre_fv=Data->fv;
					}
					else
						Data->fv=Data->pre_fv;
					(*AIORcur)+=2;
					break;
				case 5:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->uiv=custom_get_unsigned_int(AO_Regs_temp_Ary);
						Data->pre_uiv=Data->uiv;
					}
					else
						Data->uiv=Data->pre_uiv;
					(*AIORcur)+=2;
					break;
				case 6:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->uiv=custom_get_unsigned_int_cdab(AO_Regs_temp_Ary);
						Data->pre_uiv=Data->uiv;
					}
					else
						Data->uiv=Data->pre_uiv;
					(*AIORcur)+=2;
					break;
				case 7:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->iv=custom_get_int(AO_Regs_temp_Ary);
						Data->pre_iv=Data->iv;
					}
					else
						Data->iv=Data->pre_iv;
					(*AIORcur)+=2;
					break;
				case 8:
					if(Data->bRevFin)
					{
						AO_Regs_temp_Ary[0]=AO_Regs[*AIORcur];
						AO_Regs_temp_Ary[1]=AO_Regs[*AIORcur+1];
						Data->iv=custom_get_int_cdab(AO_Regs_temp_Ary);
						Data->pre_iv=Data->iv;
					}
					else
						Data->iv=Data->pre_iv;
					(*AIORcur)+=2;
					break;
				default:
					Data->Regs=AO_Regs[*AIORcur];
					(*AIORcur)++;

			}

	}

}
//Prepare to Download Data
int DownDataPrepare(WISE_Data *Data,float fv,uint32_t uiv,int iv, bool ret)
{
	int rc;
	if(!ret)
	{
		Data->fv=0;
		Data->Regs=0;
		return -1;
	}
	else
	{
		switch(Data->sw_mode)
		{
			case 1:
				custom_set_float(fv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 2:
				custom_set_float_dcba(fv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 3:
				custom_set_float_badc(fv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 4:
				custom_set_float_cdab(fv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 5:
				custom_set_unsigned_int(uiv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 6:
				custom_set_unsigned_int_cdab(uiv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 7:
				custom_set_int(iv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			case 8:
				custom_set_int_cdab(iv,AO_Set_Regs_temp_Ary);
				rc=modbus_write_register(ctx,Data->address,AO_Set_Regs_temp_Ary[0]);
				rc=modbus_write_register(ctx,Data->address+1,AO_Set_Regs_temp_Ary[1]);
				return rc;
				break;
			default:
				return modbus_write_register(ctx,Data->address,fv/Data->precision);

		}

	}


}
/*
//-------------------------------------Customize
void CustomFunc_Str(char *barcode)
{
	int i=0;
	int flag = false;
	char *delim = "_";
	char * pch;
	char tmp_name[20];
	char tmp[4]={""};
	char str[BARCODE_REG_NUM][3]={{""}}; 
	int index1,index2;
	int pos;

	strcpy(barcode,"");
	if(barflag==1)
	{
			for(i=0;i<numberOfAI;i++)
			{
				if(strncmp(AI[i].name,"barcode",6)==0)
				{	strcpy(tmp_name,AI[i].name);
					pch = strtok(tmp_name,delim);
					while (pch != NULL)
					{
						if(flag)
							strcpy(tmp,pch);
						//printf ("%s\n",pch);
						flag=true;
						pch = strtok(NULL, delim);
					}
					flag=false;
					pos=atoi(tmp);

					index1=(AI_Regs[i]>>8)-48;
					if(index1>42 || index1 <0)
					{
						if(index1 == 51)
							str[pos][1]='m';
						else
							str[pos][1]=' ';
					}
					else
						str[pos][1] = ascii_array[index1];
					
					index2 = (AI_Regs[i]&0xff)-48;
					if(index2>42 || index2 <0)
					{
						if(index2 == 51)
							str[pos][0]='m';
						else
							str[pos][0]=' ';
					}
					else
						str[pos][0] = ascii_array[index2];
				}
			}
	}
	else if (barflag==2)
	{
			for(i=0;i<numberOfAO;i++)
			{
				if(strncmp(AO[i].name,"barcode",6)==0)
				{	strcpy(tmp_name,AO[i].name);
					pch = strtok(tmp_name,delim);
					while (pch != NULL)
					{
						if(flag)
							strcpy(tmp,pch);
						//printf ("%s\n",pch);
						flag=true;
						pch = strtok(NULL, delim);
					}
					flag=false;
					pos=atoi(tmp);

					index1=(AO_Regs[i]>>8)-48;
					if(index1>42 || index1 <0)
					{
						if(index1 == 51)
							str[pos][1]='m';
						else
							str[pos][1]=' ';
					}
					else
						str[pos][1] = ascii_array[index1];
					
					index2 = (AO_Regs[i]&0xff)-48;
					if(index2>42 || index2 <0)
					{
						if(index2 == 51)
							str[pos][0]='m';
						else
							str[pos][0]=' ';
					}
					else
						str[pos][0] = ascii_array[index2];
				}
			}
	}


	for(i=0;i<BARCODE_REG_NUM;i++)
		strcat(barcode,str[i]);
	barcode[BARCODE_NUM-2]='\0';

	printf("barcode : %s\n",barcode);

}
//-------------------------------------Customize_end
*/
MSG_CLASSIFY_T * CreateCapability()
{
	MSG_CLASSIFY_T *myCapability = IoT_CreateRoot((char*) strPluginName);
	MSG_CLASSIFY_T *myGroup;
	MSG_ATTRIBUTE_T* attr;
	IoT_READWRITE_MODE mode=IoT_READONLY;

    char Client_Port[6];
	char Client_UnitID[6];
	char SlaveID[6];
	bool bUAI_AO;

	/*
	//-------------------------------------Customize
	char barcode[BARCODE_NUM]={""};
	//-------------------------------------Customize_end
	*/

	if(bFind)
	{			
				myGroup = IoT_AddGroup(myCapability,"Platform");
				
				if(myGroup)
				{	mode=IoT_READONLY;
					attr = IoT_AddSensorNode(myGroup, "Protocol");
					if(attr)
						IoT_SetStringValue(attr,Modbus_Protocol,mode);

					attr = IoT_AddSensorNode(myGroup, "Name");
					if(attr)
						IoT_SetStringValue(attr,Device_Name,mode);
					if(iTCP_RTU==0)
					{	
						sprintf(Client_Port,"%d",Modbus_Client_Port);
						sprintf(Client_UnitID,"%d",Modbus_UnitID);

						attr = IoT_AddSensorNode(myGroup, "ClientIP");
						if(attr)
							IoT_SetStringValue(attr,Modbus_Clent_IP,mode);

						attr = IoT_AddSensorNode(myGroup, "ClientPort");
						if(attr)
							IoT_SetStringValue(attr,Client_Port,mode);

						attr = IoT_AddSensorNode(myGroup, "UnitID");
						if(attr)
							IoT_SetStringValue(attr,Client_UnitID,mode);
					}
					else if(iTCP_RTU==1)
					{
						sprintf(SlaveID,"%d",Modbus_SlaveID);

						attr = IoT_AddSensorNode(myGroup, "SlavePort");
						if(attr)
							IoT_SetStringValue(attr,Modbus_Slave_Port,mode);

						attr = IoT_AddSensorNode(myGroup, "Baud");
						if(attr)
							IoT_SetDoubleValue(attr,Modbus_Baud,mode,"bps");

						attr = IoT_AddSensorNode(myGroup, "Parity");
						if(attr)
							IoT_SetStringValue(attr,Modbus_Parity,mode);

						attr = IoT_AddSensorNode(myGroup, "DataBits");
						if(attr)
							IoT_SetDoubleValue(attr,Modbus_DataBits,mode,"bits");

						attr = IoT_AddSensorNode(myGroup, "StopBits");
						if(attr)
							IoT_SetDoubleValue(attr,Modbus_StopBits,mode,"bits");

						attr = IoT_AddSensorNode(myGroup, "SlaveID");
						if(attr)
							IoT_SetStringValue(attr,SlaveID,mode);
					
					}
					else
					{
						;//Not TCP or RTU
					}


					attr = IoT_AddSensorNode(myGroup, "Connection");
					if(attr)
					{
						if(bIsSimtor)
						{
							IoT_SetBoolValue(attr,true,mode);
						}
						else
						{
							if(!g_bRetrieve)
								IoT_SetBoolValue(attr,false,mode);
							else
								IoT_SetBoolValue(attr,true,mode);
						}
								
					}	
				}

				
				if(numberOfDI!=0)
				{	
					myGroup = IoT_AddGroup(myCapability, "Discrete Inputs");
					if(myGroup)
					{
							mode=IoT_READONLY;
							for(int i=0;i<numberOfDI;i++){
								if(bIsSimtor)
								{
									DI_Bits[i]=rand()%2;
								}
								else
								{
									if(!g_bRetrieve)
										DI_Bits[i]=false;
								}
								attr = IoT_AddSensorNode(myGroup, DI[i].name);
								if(attr)
								{
									DI[i].Bits=DI_Bits[i];
									IoT_SetBoolValue(attr,DI[i].Bits,mode);	
								}
							}
					}

				}
				
				if(numberOfDO!=0)
				{
					myGroup = IoT_AddGroup(myCapability, "Coils");
					if(myGroup)
					{
							mode=IoT_READWRITE;
							for(int i=0;i<numberOfDO;i++){
								if(bIsSimtor)
								{
									DO_Bits[i]=rand()%2;
								}
								else
								{
									if(!g_bRetrieve)
										DO_Bits[i]=false;
								}
								attr = IoT_AddSensorNode(myGroup, DO[i].name);
								if(attr)
								{
									DO[i].Bits=DO_Bits[i];
									IoT_SetBoolValue(attr,DO[i].Bits,mode);	
								}
							}
					}

				}
				/*
				//-----------------------------------------Customize
				if(numberOfAI!=0)
				{
					bool flag=false;
					for(int i=0;i<numberOfAI;i++)
						if(strncmp(AI[i].name,"barcode",6)==0)
						{
							flag=true;
							break;
						}
					if(flag)
					{
						myGroup = IoT_AddGroup(myCapability, "Barcode");
						if(myGroup)
						{
								mode=IoT_READONLY;
								barflag=1;
								attr = IoT_AddSensorNode(myGroup, "Barcode");
								if(attr)
									CustomFunc_Str(barcode);

								IoT_SetStringValue(attr,barcode,mode);
								
						}
					}
				}
				if(numberOfAO!=0)
				{
					bool flag=false;
					for(int i=0;i<numberOfAO;i++)
						if(strncmp(AO[i].name,"barcode",6)==0)
						{
							flag=true;
							break;
						}
					if(flag)
					{
						myGroup = IoT_AddGroup(myCapability, "Barcode");
						if(myGroup)
						{
								mode=IoT_READONLY;
								barflag=2;
								attr = IoT_AddSensorNode(myGroup, "Barcode");
								if(attr)
									CustomFunc_Str(barcode);

								IoT_SetStringValue(attr,barcode,mode);
								
						}
					}
				}
				//-----------------------------------------Customize_end
				*/
				if(numberOfAI!=0)
				{	int AIRcur=0;
					bUAI_AO=true;
					myGroup = IoT_AddGroup(myCapability, "Input Registers");
					if(myGroup)
					{
							mode=IoT_READONLY;
							for(int i=0;i<numberOfAI;i++){
								if(bIsSimtor)
								{
									while(true)
									{
										if(AI[i].max==0)
										{
											AI_Regs[i]=0;
											break;
										}
										AI_Regs[i]=rand()%(int)AI[i].max/AI[i].precision;
										if(AI_Regs[i]>=AI[i].min)
											break;
										else
											continue;
									}								
								}
								else
								{
									//if(!ret)
									//	AI_Regs[i]=0;
								}

								attr = IoT_AddSensorNode(myGroup, AI[i].name);

								if(attr)
								{		
									UpDataPrepare(&AI[i],bUAI_AO,&AIRcur,g_bRetrieve);
									if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4)
									{	
										IoT_SetDoubleValueWithMaxMin(attr,AI[i].fv,mode,AI[i].max,AI[i].min,AI[i].unit);
									}
									else
									{
										IoT_SetDoubleValueWithMaxMin(attr,AI[i].Regs*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
									}

								}
							}

					}

				}

				if(numberOfAO!=0)
				{	int AORcur=0;
					bUAI_AO=false;
					myGroup = IoT_AddGroup(myCapability, "Holding Registers");
					if(myGroup)
					{
							mode=IoT_READWRITE;
							for(int i=0;i<numberOfAO;i++){
								if(bIsSimtor)
								{
									while(true)
									{
										if(AO[i].max==0)
										{
											AO_Regs[i]=0;
											break;
										}
										AO_Regs[i]=rand()%(int)AO[i].max/AO[i].precision;
										if(AO_Regs[i]>=AO[i].min)
											break;
										else
											continue;
									}								
								}
								else
								{
									//if(!ret)
									//	AO_Regs[i]=0;
								}

								attr = IoT_AddSensorNode(myGroup, AO[i].name);

								if(attr)
								{		
									UpDataPrepare(&AO[i],bUAI_AO,&AORcur,g_bRetrieve);
									if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4)
									{	
										IoT_SetDoubleValueWithMaxMin(attr,AO[i].fv,mode,AO[i].max,AO[i].min,AO[i].unit);
									}
									else
									{
										IoT_SetDoubleValueWithMaxMin(attr,AO[i].Regs*AO[i].precision,mode,AO[i].max,AO[i].min,AO[i].unit);
									}

								}
							}
					}

				}
				
	}

	return myCapability;
}


//--------------------------------------------------------------------------------------------------------------
//------------------------------------------------Modbus--------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//Establish a Modbus Connection
bool Modbus_Connect()
{
	if (modbus_connect(ctx) == -1) {
		ModbusLog(g_loghandle, Error, "Modbus Connection failed!! - Connect ERROR : %s",modbus_strerror(errno));

		bConnectionFlag=false;
		return false;
	}
	else
	{	
		ModbusLog(g_loghandle, Normal, "Modbus Connection established!!");

		bConnectionFlag=true;
		if(iTCP_RTU==0)
			modbus_set_slave(ctx, Modbus_UnitID);
		else if(iTCP_RTU==1)
			modbus_set_slave(ctx, Modbus_SlaveID);
		else 
			;	//Not TCP or RTU
		return true;
	}

}

//Disconnect a Modbus Connection
bool Modbus_Disconnect()
{
	modbus_close(ctx);
	ModbusLog(g_loghandle, Warning, "Modbus Disconnection!!");
	return false;
}

//Receive Data From Modbus
bool Modbus_Rev()
{

	int ret=-1;
	int i=0;
	bool bFCon=false;
	char str[655350]=""; 

	strcat(str,"{");

	if(g_bRev_Fail==true)
		bFCon=true;

	if(bFCon && g_bRev_Fail)
		Rev_Fail_Num++;
	
	//------------------floatint test case   10.565(abcd) 0.033731(dcba) refer to float_sample.txt
	/*uint16_t a=16681;		
	uint16_t b=2621;	
	uint16_t *t=(uint16_t *)calloc(2,sizeof(uint16_t));
	t[0]=a;
	t[1]=b;
	printf("%f\n",custom_get_float(t));
	printf("%f\n",custom_get_float_dcba(t));*/
	//------------------floatint test case end

	g_bRev_Fail=false;

	printf("Rev_Fail_Num : %d\n",Rev_Fail_Num);
	
	if(Rev_Fail_Num>5) //reconnect --> continual 6 times lost connection  
	{		
		Modbus_Disconnect();
		sleep(5);			//sleep 5 secs for avoiding lots of sockets created and not released yet
		if(!Modbus_Connect())
			return false;	
		else
			Rev_Fail_Num=0;
	}

	if(numberOfDI!=0)
	{	
		for(i=0;i<numberOfDI;i++)
		{	
			char tmp[50];
			DI[i].bRevFin=false;	//not in use actually
			if(bConnectionFlag)
				ret = modbus_read_input_bits(ctx, DI[i].address, 1, (DI_Bits+i));
			else
				ret = -1;
			if (ret == -1) {
				ModbusLog(g_loghandle, Error, "modbus_read_input_bits failed - %d - IB[%d] Rev ERROR : %s",Rev_Fail_Num,i,modbus_strerror(errno));
				sprintf(tmp,"\"IB[%d]\":\"%s\"",i,"FAIL");
				strcat(str,tmp);
				if((i!=(numberOfDI-1))||numberOfDO!=0||numberOfAI!=0||numberOfAO!=0)
					strcat(str,",");

				g_bRev_Fail=true;
			}
			else
			{
				sprintf(tmp,"\"IB[%d]\":%d",i,*(DI_Bits+i));
				strcat(str,tmp);
				if((i!=(numberOfDI-1))||numberOfDO!=0||numberOfAI!=0||numberOfAO!=0)
					strcat(str,",");

				DI[i].bRevFin=true;	//not in use actually
				//printf("DI_Bits[%d] : %d\n",i,DI_Bits[i]);
			}
		}

	}

	if(numberOfDO!=0)
	{	
		for(i=0;i<numberOfDO;i++)
		{
			char tmp[50];
			DO[i].bRevFin=false;	//not in use actually
			if(bConnectionFlag)
				ret = modbus_read_bits(ctx, DO[i].address, 1, (DO_Bits+i));
			else
				ret = -1;
			if (ret == -1) {
				ModbusLog(g_loghandle, Error, "modbus_read_bits failed - %d - B[%d] Rev ERROR : %s",Rev_Fail_Num,i,modbus_strerror(errno));
				sprintf(tmp,"\"B[%d]\":\"%s\"",i,"FAIL");
				strcat(str,tmp);
				if((i!=(numberOfDI-1))||numberOfDO!=0||numberOfAI!=0||numberOfAO!=0)
					strcat(str,",");

				g_bRev_Fail=true;
			}
			else
			{
				sprintf(tmp,"\"B[%d]\":%d",i,*(DO_Bits+i));
				strcat(str,tmp);
				if((i!=(numberOfDO-1))||numberOfAI!=0||numberOfAO!=0)
					strcat(str,",");

				DO[i].bRevFin=true;	//not in use actually
				//printf("DO_Bits[%d] : %d\n",i,DO_Bits[i]);
			}
		}

	}

	if(numberOfAI!=0)
	{	
		int AIRRevcur=0;
		for(i=0;i<numberOfAI;i++)
		{	
			char tmp[50];
			AI[i].bRevFin=false;
			if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4||AI[i].sw_mode==5||AI[i].sw_mode==6||AI[i].sw_mode==7||AI[i].sw_mode==8)
			{				

				if(bConnectionFlag)
					ret = modbus_read_input_registers(ctx, AI[i].address, 2, (AI_Regs+AIRRevcur));
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_input_registers failed - %d - IR[%d] Rev ERROR : %s",Rev_Fail_Num,i,modbus_strerror(errno));
					sprintf(tmp,"\"IR[%d]\":\"%s\"",i,"FAIL");
					strcat(str,tmp);
					if((i!=(numberOfAI-1))||numberOfAO!=0)
						strcat(str,",");

					g_bRev_Fail=true;
				}
				else{
					sprintf(tmp,"\"IR[%d]\":[%d,%d]",i,*(AI_Regs+AIRRevcur),*(AI_Regs+AIRRevcur+1));
					strcat(str,tmp);
					if((i!=(numberOfAI-1))||numberOfAO!=0)
						strcat(str,",");
				}
/*
				if(bConnectionFlag)
					ret = modbus_read_input_registers(ctx, AI[i].address+1, 1, (AI_Regs+AIRRevcur+1));
				else
					ret = -1;
				if (ret == -1) {
					printf("modbus_read_input_registers failed\n");
					fprintf(stderr, "Rev ERROR : %s\n", modbus_strerror(errno));
					Modbus_Disconnect();
					Modbus_Connect();
					return false;
				}
*/
				AIRRevcur+=2;
				AI[i].bRevFin=true;
			}
			else
			{
				if(bConnectionFlag)
					ret = modbus_read_input_registers(ctx, AI[i].address, 1, (AI_Regs+AIRRevcur));
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_input_registers failed - %d - IR[%d] Rev ERROR : %s",Rev_Fail_Num,i,modbus_strerror(errno));
					sprintf(tmp,"\"IR[%d]\":\"%s\"",i,"FAIL");
					strcat(str,tmp);
					if((i!=(numberOfAI-1))||numberOfAO!=0)
						strcat(str,",");

					g_bRev_Fail=true;
				}
				else{
					sprintf(tmp,"\"IR[%d]\":%d",i,*(AI_Regs+AIRRevcur));
					strcat(str,tmp);
					if((i!=(numberOfAI-1))||numberOfAO!=0)
						strcat(str,",");
				}

				//printf("AI_Regs[%d] : %d\n",i,*(AI_Regs+AIRRevcur));
				AIRRevcur++;
				AI[i].bRevFin=true;
			}

		}

	}

	if(numberOfAO!=0)
	{	
		int AORRevcur=0;
		for(i=0;i<numberOfAO;i++)
		{	
			char tmp[50];
			AO[i].bRevFin=false;
			if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4||AO[i].sw_mode==5||AO[i].sw_mode==6||AO[i].sw_mode==7||AO[i].sw_mode==8)
			{			
				if(bConnectionFlag)
					ret = modbus_read_registers(ctx, AO[i].address, 2, (AO_Regs+AORRevcur));
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_registers failed - %d - R[%d] Rev ERROR : %s",Rev_Fail_Num,i,modbus_strerror(errno));
					sprintf(tmp,"\"R[%d]\":\"%s\"",i,"FAIL");
					strcat(str,tmp);
					if((i!=(numberOfAO-1)))
						strcat(str,",");

					g_bRev_Fail=true;
				}
				else{
					sprintf(tmp,"\"R[%d]\":[%d,%d]",i,*(AO_Regs+AORRevcur),*(AO_Regs+AORRevcur+1));
					strcat(str,tmp);
					if((i!=(numberOfAO-1)))
						strcat(str,",");
				}
				AORRevcur+=2;
				AO[i].bRevFin=true;
			}
			else
			{
				
				if(bConnectionFlag)
					ret = modbus_read_registers(ctx, AO[i].address, 1, (AO_Regs+AORRevcur));
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_registers failed - %d - R[%d] Rev ERROR : %s",Rev_Fail_Num,i,modbus_strerror(errno));
					sprintf(tmp,"\"R[%d]\":\"%s\"",i,"FAIL");
					strcat(str,tmp);
					if((i!=(numberOfAO-1)))
						strcat(str,",");

					g_bRev_Fail=true;
				}
				else{
					sprintf(tmp,"\"R[%d]\":%d",i,*(AO_Regs+AORRevcur));
					strcat(str,tmp);
					if((i!=(numberOfAO-1)))
						strcat(str,",");
				}

				//printf("AO_Regs[%d] : %d\n",i,*(AO_Regs+AORRevcur));
				AORRevcur++;
				AO[i].bRevFin=true;
			}

		}
	}

	strcat(str,"}");
		if(Modbus_Log)
		{
			pMRLog=fopen(MRLog_path,"a+");
			if(pMRLog==NULL)
				ModbusLog(g_loghandle, Error, "Fail to Open MR.txt!!");
			else{
				time_t current_time=time(NULL);
				char* c_time_string;

				if (current_time == ((time_t)-1))
					fprintf(stderr, "Failure to obtain the current time.\n");

				c_time_string=ctime(&current_time);

				 if (c_time_string == NULL)
					fprintf(stderr, "Failure to convert the current time.\n");

				fprintf(pMRLog, "%d, %lld, %s, %s\n",Rev_Fail_Num,current_time,c_time_string, str);
				fclose(pMRLog);
		}
	}

	return true;

}

//--------------------------------------------------------------------------------------------------------------
//------------------------------------------------SensorInfo----------------------------------------------------
//--------------------------------------------------------------------------------------------------------------

static bool IsSensorInfoListEmpty(sensor_info_list sensorInfoList)
{
	bool bRet = TRUE;
	sensor_info_node_t * curNode = NULL, *head = NULL;
	if(sensorInfoList == NULL) return bRet;
	head = sensorInfoList;
	curNode = head->next;
	if(curNode != NULL) bRet = FALSE;
	return bRet;
}

//---------------------------------------------------------------------------------------------------------------------
//------------------------------------------------CAPABILITY_GET_SET_UPLOAD-------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

//Get Capability
static void GetCapability()
{
	char* result = NULL;
	MSG_CLASSIFY_T *t_Capability = NULL;
	int len = 0;
		
	t_Capability=CreateCapability();

	if(t_Capability)
	{
		result = IoT_PrintCapability(t_Capability);
		len=strlen(result);
		if(len!=0)
			g_sendcbf(&g_PluginInfo, modbus_get_capability_rep, result, strlen(result)+1, NULL, NULL);

		IoT_ReleaseAll(t_Capability);
		t_Capability = NULL;
	}
	else
	{
		char * errorRepJsonStr = NULL;
		char errorStr[128];
		sprintf(errorStr, "Command(%d), Get capability error!", modbus_get_capability_req);
		int jsonStrlen = Parser_PackModbusError(errorStr, &errorRepJsonStr);
		if(jsonStrlen > 0 && errorRepJsonStr != NULL)
		{
			g_sendcbf(&g_PluginInfo, modbus_error_rep, errorRepJsonStr, strlen(errorRepJsonStr)+1, NULL, NULL);
		}
		if(errorRepJsonStr)free(errorRepJsonStr);
	}

	if(result)
		free(result);



}
//Get Sensors' Data
static void GetSensorsDataEx(sensor_info_list sensorInfoList, char * pSessionID)
{
    int Num_Sensor=1000; // to avoid exceeding 
	WISE_Sensor *sensors=(WISE_Sensor *)calloc(Num_Sensor,sizeof(WISE_Sensor));
	MSG_CLASSIFY_T  *pSensorInofList=NULL,*pEArray=NULL, *pSensor=NULL;
	MSG_CLASSIFY_T  *pRoot=NULL;
	MSG_ATTRIBUTE_T	*attr;

	char * repJsonStr = NULL;
	int count=0;
	char* result = NULL;
	bool bUAI_AO;
	/*
	//-------------------------------------Customize
	char barcode[BARCODE_NUM]={""};
	//-------------------------------------Customize_end
	*/
	#pragma region IsSensorInfoListEmpty
	if(!IsSensorInfoListEmpty(sensorInfoList))
	{
		sensor_info_node_t *curNode = NULL;
		curNode = sensorInfoList->next;
	
				#pragma region pRoot
				pRoot = MSG_CreateRoot();
				if(pRoot)
				{
						attr = MSG_AddJSONAttribute(pRoot, "sessionID");
						if(attr)
							MSG_SetStringValue( attr, pSessionID, NULL); 

						pSensorInofList = MSG_AddJSONClassify(pRoot, "sensorInfoList", NULL, false);
						if(pSensorInofList)
							pEArray=MSG_AddJSONClassify(pSensorInofList,"e",NULL,true);
						while(curNode)
						{

								#pragma region pSensorInofList
								if(pSensorInofList)
								{		
										#pragma region pEArray		
										if(pEArray)
										{
													#pragma region pSensor
													pSensor = MSG_AddJSONClassify(pEArray, "sensor", NULL, false);	
													if(pSensor)
													{		attr = MSG_AddJSONAttribute(pSensor, "n");
															if(attr)
																MSG_SetStringValue(attr, curNode->sensorInfo.pathStr, NULL);
															if(count<Num_Sensor)
																Modbus_General_Node_Parser(curNode,sensors,count);

															#pragma region MSG_Find_Sensor
															if(IoT_IsSensorExist(g_Capability,curNode->sensorInfo.pathStr))
															{		
																	if(attr)
																	{	
																			#pragma region Simulator
																			if(bIsSimtor)
																			{
																									#pragma region Platform-DI-DO-AI-AO
																									int i=0;
																									if(strcmp(sensors[count].type,"Platform")==0)
																									{
																										if(strcmp(sensors[count].name,"Protocol")==0)
																										{
																											attr = MSG_AddJSONAttribute(pSensor, "sv");
																											if(attr)
																												if(MSG_SetStringValue(attr, Modbus_Protocol, "r"))
																												{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																												}
																										}
																										if(strcmp(sensors[count].name,"Name")==0)
																										{
																											attr = MSG_AddJSONAttribute(pSensor, "sv");
																											if(attr)
																												if(MSG_SetStringValue(attr, Device_Name, "r"))
																												{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																												}
																										}
																										if(iTCP_RTU==0)
																										{
																													if(strcmp(sensors[count].name,"ClientIP")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																															if(MSG_SetStringValue(attr, Modbus_Clent_IP, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													}
																													if(strcmp(sensors[count].name,"ClientPort")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																														{
																															char temp[6];
																															sprintf(temp,"%d",Modbus_Client_Port);
																															if(MSG_SetStringValue(attr, temp, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}
																													if(strcmp(sensors[count].name,"UnitID")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																														{
																															char temp[6];
																															sprintf(temp,"%d",Modbus_UnitID);
																															if(MSG_SetStringValue(attr, temp, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}
																										}
																										else if(iTCP_RTU==1)
																										{
																													if(strcmp(sensors[count].name,"SlavePort")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																															if(MSG_SetStringValue(attr, Modbus_Slave_Port, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													}
																													if(strcmp(sensors[count].name,"Baud")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "v");
																														if(attr)
																														{	
																															if(MSG_SetDoubleValue(attr, Modbus_Baud, "r", "bps"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}
																													if(strcmp(sensors[count].name,"Parity")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																														{
																															if(MSG_SetStringValue(attr, Modbus_Parity, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}
																													if(strcmp(sensors[count].name,"DataBits")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "v");
																														if(attr)
																														{	
																															if(MSG_SetDoubleValue(attr, Modbus_DataBits, "r", "bits"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}
																													if(strcmp(sensors[count].name,"StopBits")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "v");
																														if(attr)
																														{	
																															if(MSG_SetDoubleValue(attr, Modbus_StopBits, "r", "bits"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}
																													if(strcmp(sensors[count].name,"SlaveID")==0)
																													{
																														
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																														{	char temp[6];
																															sprintf(temp,"%d",Modbus_SlaveID);
																															if(MSG_SetStringValue(attr, temp, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													
																														}	
																													}

																										}

																										if(strcmp(sensors[count].name,"Connection")==0)
																										{
																											attr = MSG_AddJSONAttribute(pSensor, "sv");
																											if(attr)
																												if(MSG_SetBoolValue(attr, true, "r"))
																												{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																												}
																										}

																									}
																									if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																									{
																										for(i=0;i<numberOfDI;i++)
																										{
																											if(strcmp(DI[i].name,sensors[count].name)==0)
																											{	
																												DI[i].Bits=rand()%2;
																												attr = MSG_AddJSONAttribute(pSensor, "bv");
																												if(attr)
																													if(MSG_SetBoolValue(attr, DI[i].Bits, "r"))
																													{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																														if(attr)
																															MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																													}	
																											}
																											if(i==numberOfDI)
																											{	
																												attr = MSG_AddJSONAttribute(pSensor, "sv");
																												if(attr)
																													MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																												attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																												if(attr)
																													MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																											}
																										}
																									}
																									if(strcmp(sensors[count].type,"Coils")==0)
																									{
																										for(i=0;i<numberOfDO;i++)
																										{
																											if(strcmp(DO[i].name,sensors[count].name)==0)
																											{	
																												DO[i].Bits=rand()%2;									
																												attr = MSG_AddJSONAttribute(pSensor, "bv");
																												if(attr)
																													if(MSG_SetBoolValue(attr, DO[i].Bits, "rw"))
																													{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																														if(attr)
																															MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																													}	
																											}
																											if(i==numberOfDO)
																											{	
																												attr = MSG_AddJSONAttribute(pSensor, "sv");
																												if(attr)
																													MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																												attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																												if(attr)
																													MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																											}
																										}
																									}
																									if(strcmp(sensors[count].type,"Input Registers")==0)
																									{
																										for(i=0;i<numberOfAI;i++)
																										{
																											if(strcmp(AI[i].name,sensors[count].name)==0)
																											{	
																												while(true)
																												{
																													if(AI[i].max==0)
																													{
																														AI_Regs[i]=0;
																														break;
																													}
																													AI[i].Regs=rand()%(int)AI[i].max;
																													if(AI[i].Regs>=AI[i].min)
																														break;
																													else
																														continue;
																												}
																												attr = MSG_AddJSONAttribute(pSensor, "v");
																												if(attr)
																													if(MSG_SetDoubleValue(attr, AI[i].Regs,"r", AI[i].unit))
																													{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																														if(attr)
																															MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																													}	
																											}
																											if(i==numberOfAI)
																											{	
																												attr = MSG_AddJSONAttribute(pSensor, "sv");
																												if(attr)
																													MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																												attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																												if(attr)
																													MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																											}
																										}
																									}
																									if(strcmp(sensors[count].type,"Holding Registers")==0)
																									{
																										for(i=0;i<numberOfAO;i++)
																										{
																											if(strcmp(AO[i].name,sensors[count].name)==0)
																											{	
																												while(true)
																												{
																													if(AO[i].max==0)
																													{
																														AO_Regs[i]=0;
																														break;
																													}
																													AO[i].Regs=rand()%(int)AO[i].max;
																													if(AO[i].Regs>=AO[i].min)
																														break;
																													else
																														continue;
																												}									
																												attr = MSG_AddJSONAttribute(pSensor, "v");
																												if(attr)
																													if(MSG_SetDoubleValue(attr, AO[i].Regs,"rw", AO[i].unit))
																													{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																														if(attr)
																															MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																													}	
																											}
																											if(i==numberOfAO)
																											{	
																												attr = MSG_AddJSONAttribute(pSensor, "sv");
																												if(attr)
																													MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																												attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																												if(attr)
																													MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																											}
																										}
																									}
																									#pragma endregion Platform-DI-DO-AI-AO
																			}	
																			else
																			{
																						#pragma region g_bRetrieve
																						if(g_bRetrieve)
																						{	
																												#pragma region Platform-DI-DO-AI-AO
																												int i=0;
																												if(strcmp(sensors[count].type,"Platform")==0)
																												{
																													if(strcmp(sensors[count].name,"Protocol")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																															if(MSG_SetStringValue(attr, Modbus_Protocol, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													}
																													if(strcmp(sensors[count].name,"Name")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																															if(MSG_SetStringValue(attr, Device_Name, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													}
																													if(iTCP_RTU==0)
																													{
																															if(strcmp(sensors[count].name,"ClientIP")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "sv");
																																if(attr)
																																	if(MSG_SetStringValue(attr, Modbus_Clent_IP, "r"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															}
																															if(strcmp(sensors[count].name,"ClientPort")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "sv");
																																if(attr)
																																{
																																	char temp[6];
																																	sprintf(temp,"%d",Modbus_Client_Port);
																																	if(MSG_SetStringValue(attr, temp, "r"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}
																															if(strcmp(sensors[count].name,"UnitID")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "sv");
																																if(attr)
																																{
																																	char temp[6];
																																	sprintf(temp,"%d",Modbus_UnitID);
																																	if(MSG_SetStringValue(attr, temp, "r"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}
																													}
																													else if(iTCP_RTU==1)
																													{
																															if(strcmp(sensors[count].name,"SlavePort")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "sv");
																																if(attr)
																																	if(MSG_SetStringValue(attr, Modbus_Slave_Port, "r"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															}
																															if(strcmp(sensors[count].name,"Baud")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "v");
																																if(attr)
																																{	
																																	if(MSG_SetDoubleValue(attr, Modbus_Baud, "r", "bps"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}
																															if(strcmp(sensors[count].name,"Parity")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "sv");
																																if(attr)
																																{
																																	if(MSG_SetStringValue(attr, Modbus_Parity, "r"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}
																															if(strcmp(sensors[count].name,"DataBits")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "v");
																																if(attr)
																																{	
																																	if(MSG_SetDoubleValue(attr, Modbus_DataBits, "r", "bits"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}
																															if(strcmp(sensors[count].name,"StopBits")==0)
																															{
																																attr = MSG_AddJSONAttribute(pSensor, "v");
																																if(attr)
																																{	
																																	if(MSG_SetDoubleValue(attr, Modbus_StopBits, "r", "bits"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}
																															if(strcmp(sensors[count].name,"SlaveID")==0)
																															{
																																
																																attr = MSG_AddJSONAttribute(pSensor, "sv");
																																if(attr)
																																{	char temp[6];
																																	sprintf(temp,"%d",Modbus_SlaveID);
																																	if(MSG_SetStringValue(attr, temp, "r"))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																															
																																}	
																															}

																													}
																													else
																														;//Not TCP or RTU

																													if(strcmp(sensors[count].name,"Connection")==0)
																													{
																														attr = MSG_AddJSONAttribute(pSensor, "sv");
																														if(attr)
																															if(MSG_SetBoolValue(attr, true, "r"))
																															{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																if(attr)
																																	MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																															}
																													}

																												}
																												if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																												{
																													for(i=0;i<numberOfDI;i++)
																													{
																														if(strcmp(DI[i].name,sensors[count].name)==0)
																														{	
																															DI[i].Bits=DI_Bits[i];									
																															attr = MSG_AddJSONAttribute(pSensor, "bv");
																															if(attr)
																																if(MSG_SetBoolValue(attr, DI[i].Bits, "r"))
																																{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																}	
																														}
																														if(i==numberOfDI)
																														{	
																															attr = MSG_AddJSONAttribute(pSensor, "sv");
																															if(attr)
																																MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																															attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																															if(attr)
																																MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																														}
																													}
																												}
																												if(strcmp(sensors[count].type,"Coils")==0)
																												{
																													for(i=0;i<numberOfDO;i++)
																													{
																														if(strcmp(DO[i].name,sensors[count].name)==0)
																														{	
																															DO[i].Bits=DO_Bits[i];									
																															attr = MSG_AddJSONAttribute(pSensor, "bv");
																															if(attr)
																																if(MSG_SetBoolValue(attr, DO[i].Bits, "rw"))
																																{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																																}	
																														}
																														if(i==numberOfDO)
																														{	
																															attr = MSG_AddJSONAttribute(pSensor, "sv");
																															if(attr)
																																MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																															attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																															if(attr)
																																MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																														}
																													}
																												}
																												if(strcmp(sensors[count].type,"Input Registers")==0)
																												{	int AIRcur=0;
																													bUAI_AO=true;
																													for(i=0;i<numberOfAI;i++)
																													{	
																														UpDataPrepare(&AI[i],bUAI_AO,&AIRcur,g_bRetrieve);
																														if(strcmp(AI[i].name,sensors[count].name)==0)
																														{	
																															//AI[i].Regs=AI_Regs[i];		
																															attr = MSG_AddJSONAttribute(pSensor, "v");
																															if(attr)
																															{
																																if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4)
																																{
																																	if(MSG_SetDoubleValue(attr, AI[i].fv,"r", AI[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																																}
																																else if(AI[i].sw_mode==5||AI[i].sw_mode==6)
																																{
																																	if(MSG_SetDoubleValue(attr, AI[i].uiv,"r", AI[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																																}
																																else if(AI[i].sw_mode==7||AI[i].sw_mode==8)
																																{
																																	if(MSG_SetDoubleValue(attr, AI[i].iv,"r", AI[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}
																																}
																																else
																																{
																																	if(MSG_SetDoubleValue(attr, AI[i].Regs*AI[i].precision,"r", AI[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"r", NULL);																			
																																	}	
																																}
																															}
																														}
																														if(i==numberOfAI)
																														{	
																															attr = MSG_AddJSONAttribute(pSensor, "sv");
																															if(attr)
																																MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																															attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																															if(attr)
																																MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																														}
																													}
																												}
																												if(strcmp(sensors[count].type,"Holding Registers")==0)
																												{	int AORcur=0;
																													bUAI_AO=false;
																													for(i=0;i<numberOfAO;i++)
																													{	
																														UpDataPrepare(&AO[i],bUAI_AO,&AORcur,g_bRetrieve);
																														if(strcmp(AO[i].name,sensors[count].name)==0)
																														{	
																															//AO[i].Regs=AO_Regs[i];									
																															attr = MSG_AddJSONAttribute(pSensor, "v");
																															if(attr)
																															{
																																if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4)
																																{
																																	if(MSG_SetDoubleValue(attr, AO[i].fv,"rw", AO[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																																	}
																																}
																																else if(AO[i].sw_mode==5||AO[i].sw_mode==6)
																																{
																																	if(MSG_SetDoubleValue(attr, AO[i].uiv,"rw", AO[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																																	}
																																}
																																else if(AO[i].sw_mode==7||AO[i].sw_mode==8)
																																{
																																	if(MSG_SetDoubleValue(attr, AO[i].iv,"rw", AO[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																																	}
																																}
																																else
																																{
																																	if(MSG_SetDoubleValue(attr, AO[i].Regs*AO[i].precision,"rw", AO[i].unit))
																																	{	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																		if(attr)
																																			MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);																			
																																	}	
																																}		
																															}
																														}
																														if(i==numberOfAO)
																														{	
																															attr = MSG_AddJSONAttribute(pSensor, "sv");
																															if(attr)
																																MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																															attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																															if(attr)
																																MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																														}
																													}
																												}
																												#pragma endregion Platform-DI-DO-AI-AO


																						}
																						else
																						{
																												#pragma region FAIL	
																												if(strcmp(sensors[count].type,"Platform")==0)
																												{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																													if(attr)
																														MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"r");
																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"r", NULL);	
																												}
																												if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																												{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																													if(attr)
																														MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"r");
																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"r", NULL);	
																												}
																												if(strcmp(sensors[count].type,"Coils")==0)
																												{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																													if(attr)
																														MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"rw");
																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"rw", NULL);	
																												}
																												if(strcmp(sensors[count].type,"Input Registers")==0)
																												{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																													if(attr)
																														MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"r");
																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"r", NULL);	
																												}
																												if(strcmp(sensors[count].type,"Holding Registers")==0)
																												{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																													if(attr)
																														MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"rw");
																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																													if(attr)
																														MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"rw", NULL);	
																												}
																												#pragma endregion FAIL
																						}	
																						#pragma endregion g_bRetrieve
																			}
																			#pragma endregion Simulator
																																				
																	
																	}
															}
															else
															{
																									#pragma region NOT FOUND	
																									if(strcmp(sensors[count].type,"Platform")==0)
																									{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																										if(attr)
																											MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																										if(attr)
																											MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																									}
																									else if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																									{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																										if(attr)
																											MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																										if(attr)
																											MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																									}
																									else if(strcmp(sensors[count].type,"Coils")==0)
																									{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																										if(attr)
																											MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																										if(attr)
																											MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																									}
																									else if(strcmp(sensors[count].type,"Input Registers")==0)
																									{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																										if(attr)
																											MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																										if(attr)
																											MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																									}
																									else if(strcmp(sensors[count].type,"Holding Registers")==0)
																									{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																										if(attr)
																											MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																										if(attr)
																											MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																									}
																									else
																									{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																										if(attr)
																											MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,NULL);
																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																										if(attr)
																											MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,NULL, NULL);	
																									}
																									#pragma endregion NOT FOUND
															}
															#pragma endregion MSG_Find_Sensor

													}
													#pragma endregion pSensor
										}
										#pragma endregion pEArray
								}
								#pragma endregion pSensorInofList
							curNode = curNode->next;
							count++;
						}
						{

							repJsonStr = IoT_PrintData(pRoot);
							printf("\nGET Handler_Data = %s\n",repJsonStr);
							printf("---------------------\n");
							if(repJsonStr != NULL)
							{
								g_sendcbf(&g_PluginInfo, modbus_get_sensors_data_rep, repJsonStr, strlen(repJsonStr)+1, NULL, NULL);
							}
							if(repJsonStr)free(repJsonStr);
						}
				}
				#pragma endregion pRoot
	}
	#pragma endregion IsSensorInfoListEmpty

	if(pRoot)
		MSG_ReleaseRoot(pRoot);
	if(sensors)		
		free(sensors);


}


//Set Sensors' Data
static void SetSensorsDataEx(sensor_info_list sensorInfoList, char * pSessionID)
{
    int Num_Sensor=1000; // to avoid exceeding 
	WISE_Sensor *sensors=(WISE_Sensor *)calloc(Num_Sensor,sizeof(WISE_Sensor));
	MSG_CLASSIFY_T  *pSensorInofList=NULL,*pEArray=NULL, *pSensor=NULL;
	MSG_CLASSIFY_T  *pRoot=NULL;
	MSG_ATTRIBUTE_T	*attr;

	char * repJsonStr = NULL;
	int count=0;

	bool bVal=false;
	double dVal=0;
	char *sVal=(char *)calloc(SET_STR_LENGTH,sizeof(char));
	bool bFormat=false;


	#pragma region IsSensorInfoListEmpty
	if(!IsSensorInfoListEmpty(sensorInfoList))
	{
		sensor_info_node_t *curNode = NULL;
		curNode = sensorInfoList->next;
				#pragma region pRoot
				pRoot = MSG_CreateRoot();
				if(pRoot)
				{
						attr = MSG_AddJSONAttribute(pRoot, "sessionID");
						if(attr)
							MSG_SetStringValue( attr, pSessionID, NULL); 

						pSensorInofList = MSG_AddJSONClassify(pRoot, "sensorInfoList", NULL, false);
						if(pSensorInofList)
							pEArray=MSG_AddJSONClassify(pSensorInofList,"e",NULL,true);
						
						while(curNode)
						{

								bFormat=Modbus_Parser_Set_FormatCheck(curNode,&bVal,&dVal,sVal);
								if(bFormat)
									;
								else
									printf("\nFormat Error!!\n");

								#pragma region pSensorInofList
								if(pSensorInofList)
								{		
										#pragma region pEArray		
										if(pEArray)
										{
													#pragma region pSensor
													pSensor = MSG_AddJSONClassify(pEArray, "sensor", NULL, false);	
													if(pSensor)
													{		attr = MSG_AddJSONAttribute(pSensor, "n");
															if(attr)
																MSG_SetStringValue(attr, curNode->sensorInfo.pathStr, NULL);
															if(count<Num_Sensor)
																Modbus_General_Node_Parser(curNode,sensors,count);

																			#pragma region bFormat
																			if(!bFormat)
																			{
																						if(strcmp(sensors[count].type,"Platform")==0)
																						{
																							attr = MSG_AddJSONAttribute(pSensor, "sv");
																							if(attr)
																								MSG_SetStringValue(attr,IOT_SGRC_STR_FORMAT_ERROR,"r");
																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																							if(attr)
																								MSG_SetDoubleValue(attr, IOT_SGRC_FORMAT_ERROR,"r", NULL);	
																						}

																						if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																						{
																							attr = MSG_AddJSONAttribute(pSensor, "sv");
																							if(attr)
																								MSG_SetStringValue(attr,IOT_SGRC_STR_FORMAT_ERROR,"r");
																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																							if(attr)
																								MSG_SetDoubleValue(attr, IOT_SGRC_FORMAT_ERROR,"r", NULL);	
																						}

																						if(strcmp(sensors[count].type,"Coils")==0)
																						{
																							attr = MSG_AddJSONAttribute(pSensor, "sv");
																							if(attr)
																								MSG_SetStringValue(attr,IOT_SGRC_STR_FORMAT_ERROR,"rw");
																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																							if(attr)
																								MSG_SetDoubleValue(attr, IOT_SGRC_FORMAT_ERROR,"rw", NULL);	
																						}

																						if(strcmp(sensors[count].type,"Input Registers")==0)
																						{
																							attr = MSG_AddJSONAttribute(pSensor, "sv");
																							if(attr)
																								MSG_SetStringValue(attr,IOT_SGRC_STR_FORMAT_ERROR,"r");
																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																							if(attr)
																								MSG_SetDoubleValue(attr, IOT_SGRC_FORMAT_ERROR,"r", NULL);	
																						}

																						if(strcmp(sensors[count].type,"Holding Registers")==0)
																						{
																							attr = MSG_AddJSONAttribute(pSensor, "sv");
																							if(attr)
																								MSG_SetStringValue(attr,IOT_SGRC_STR_FORMAT_ERROR,"rw");
																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																							if(attr)
																								MSG_SetDoubleValue(attr, IOT_SGRC_FORMAT_ERROR,"rw", NULL);	
																						}

																			}	
																			else
																			{
																						#pragma region MSG_Find_Sensor
																						if(IoT_IsSensorExist(g_Capability,curNode->sensorInfo.pathStr))
																						{		
																								if(attr)
																								{	
																														#pragma region bIsSimtor
																														if(bIsSimtor)
																														{
																																#pragma region Platform-DI-DO-AI-AO
																																int i=0;
																																if(strcmp(sensors[count].type,"Platform")==0)
																																{
																																	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_READ_ONLY,"r");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_READ_ONLY,"r", NULL);	
																																}
																																if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																																{
							
																																	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_READ_ONLY,"r");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_READ_ONLY,"r", NULL);	
																																		
																																}
																																if(strcmp(sensors[count].type,"Coils")==0)
																																{
																																	for(i=0;i<numberOfDO;i++)
																																	{
																																		if(strcmp(DO[i].name,sensors[count].name)==0)
																																		{	
																																			attr = MSG_AddJSONAttribute(pSensor, "sv");
																																			if(attr)
																																				MSG_SetStringValue(attr,IOT_SGRC_STR_SUCCESS,"rw");
																																			attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																			if(attr)
																																				MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);
																																		}
																																		if(i==numberOfDO)
																																		{	
																																			attr = MSG_AddJSONAttribute(pSensor, "sv");
																																			if(attr)
																																				MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																																			attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																			if(attr)
																																				MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																																		}
																																	}
																																}
																																if(strcmp(sensors[count].type,"Input Registers")==0)
																																{
																														
																																	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_READ_ONLY,"r");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_READ_ONLY,"r", NULL);	
																																		
																																}
																																if(strcmp(sensors[count].type,"Holding Registers")==0)
																																{
																																	for(i=0;i<numberOfAO;i++)
																																	{
																																		if(strcmp(AO[i].name,sensors[count].name)==0)
																																		{		
																																				if(dVal>AO[i].max || dVal<AO[i].min)
																																				{
																																					attr = MSG_AddJSONAttribute(pSensor, "sv");
																																					if(attr)
																																						MSG_SetStringValue(attr,IOT_SGRC_STR_OUT_RANGE,"rw");
																																					attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																					if(attr)
																																						MSG_SetDoubleValue(attr,IOT_SGRC_OUT_RANGE,"rw", NULL);	
																																				}
																																				else
																																				{
																																					attr = MSG_AddJSONAttribute(pSensor, "sv");
																																					if(attr)
																																						MSG_SetStringValue(attr,IOT_SGRC_STR_SUCCESS,"rw");
																																					attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																					if(attr)
																																						MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);	

																																				}

																																		}
																																		if(i==numberOfAO)
																																		{	
																																			attr = MSG_AddJSONAttribute(pSensor, "sv");
																																			if(attr)
																																				MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																																			attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																			if(attr)
																																				MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																																		}
																																	}
																																}

																																#pragma endregion Platform-DI-DO-AI-AO
																													}
																													else
																													{
																																#pragma region g_bRetrieve
																																if(g_bRetrieve)
																																{	
																																						#pragma region Platform-DI-DO-AI-AO
																																						int i=0;
																																						if(strcmp(sensors[count].type,"Platform")==0)
																																						{
																																									attr = MSG_AddJSONAttribute(pSensor, "sv");
																																									if(attr)
																																										MSG_SetStringValue(attr,IOT_SGRC_STR_READ_ONLY,"r");
																																									attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																									if(attr)
																																										MSG_SetDoubleValue(attr, IOT_SGRC_READ_ONLY,"r", NULL);	
																																						}
																																						if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																																						{
													
																																									attr = MSG_AddJSONAttribute(pSensor, "sv");
																																									if(attr)
																																										MSG_SetStringValue(attr,IOT_SGRC_STR_READ_ONLY,"r");
																																									attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																									if(attr)
																																										MSG_SetDoubleValue(attr, IOT_SGRC_READ_ONLY,"r", NULL);	
																																								
																																						}
																																						if(strcmp(sensors[count].type,"Coils")==0)
																																						{
																																							for(i=0;i<numberOfDO;i++)
																																							{
																																								if(strcmp(DO[i].name,sensors[count].name)==0)
																																								{	

																																									int ret=modbus_write_bit(ctx,DO[i].address,bVal);

																																									if(ret!=-1)
																																									{
																																										attr = MSG_AddJSONAttribute(pSensor, "sv");
																																										if(attr)
																																											MSG_SetStringValue(attr,IOT_SGRC_STR_SUCCESS,"rw");
																																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																										if(attr)
																																											MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);
																																									}
																																									else
																																									{
																																										attr = MSG_AddJSONAttribute(pSensor, "sv");
																																										if(attr)
																																											MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"rw");
																																										attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																										if(attr)
																																											MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"rw", NULL);	

																																									}

																																								}
																																								if(i==numberOfDO)
																																								{	
																																									attr = MSG_AddJSONAttribute(pSensor, "sv");
																																									if(attr)
																																										MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																																									attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																									if(attr)
																																										MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																																								}
																																							}
																																						}
																																						if(strcmp(sensors[count].type,"Input Registers")==0)
																																						{
																																				
																																									attr = MSG_AddJSONAttribute(pSensor, "sv");
																																									if(attr)
																																										MSG_SetStringValue(attr,IOT_SGRC_STR_READ_ONLY,"r");
																																									attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																									if(attr)
																																										MSG_SetDoubleValue(attr, IOT_SGRC_READ_ONLY,"r", NULL);	
																																								
																																						}
																																						if(strcmp(sensors[count].type,"Holding Registers")==0)
																																						{	
																																							for(i=0;i<numberOfAO;i++)
																																							{	

																																								if(strcmp(AO[i].name,sensors[count].name)==0)
																																								{
																																										if(dVal>AO[i].max || dVal<AO[i].min)
																																										{
																																												attr = MSG_AddJSONAttribute(pSensor, "sv");
																																												if(attr)
																																													MSG_SetStringValue(attr,IOT_SGRC_STR_OUT_RANGE,"rw");
																																												attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																												if(attr)
																																													MSG_SetDoubleValue(attr,IOT_SGRC_OUT_RANGE,"rw", NULL);	
																																										}
																																										else
																																										{		int ret=DownDataPrepare(&AO[i],(float)dVal,(uint32_t)dVal,(int)dVal,g_bRetrieve);
																																												if(ret!=-1)
																																												{
																																													attr = MSG_AddJSONAttribute(pSensor, "sv");
																																													if(attr)
																																														MSG_SetStringValue(attr,IOT_SGRC_STR_SUCCESS,"rw");
																																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																													if(attr)
																																														MSG_SetDoubleValue(attr, IOT_SGRC_SUCCESS,"rw", NULL);	
																																												}
																																												else
																																												{
																																													attr = MSG_AddJSONAttribute(pSensor, "sv");
																																													if(attr)
																																														MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"rw");
																																													attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																													if(attr)
																																														MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"rw", NULL);	

																																												}
																																										}

																																								}
																																								if(i==numberOfAO)
																																								{	
																																									attr = MSG_AddJSONAttribute(pSensor, "sv");
																																									if(attr)
																																										MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																																									attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																									if(attr)
																																										MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																																								}
																																							}
																																						}

																																						#pragma endregion Platform-DI-DO-AI-AO


																																}
																																else
																																{
																																						#pragma region FAIL	
																																						if(strcmp(sensors[count].type,"Platform")==0)
																																						{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																							if(attr)
																																								MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"r");
																																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																							if(attr)
																																								MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"r", NULL);	
																																						}

																																						if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																																						{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																							if(attr)
																																								MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"r");
																																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																							if(attr)
																																								MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"r", NULL);	
																																						}
																																						if(strcmp(sensors[count].type,"Coils")==0)
																																						{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																							if(attr)
																																								MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"rw");
																																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																							if(attr)
																																								MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"rw", NULL);	
																																						}
																																						if(strcmp(sensors[count].type,"Input Registers")==0)
																																						{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																							if(attr)
																																								MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"r");
																																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																							if(attr)
																																								MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"r", NULL);	
																																						}
																																						if(strcmp(sensors[count].type,"Holding Registers")==0)
																																						{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																							if(attr)
																																								MSG_SetStringValue(attr,IOT_SGRC_STR_FAIL,"rw");
																																							attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																							if(attr)
																																								MSG_SetDoubleValue(attr, IOT_SGRC_FAIL,"rw", NULL);	
																																						}
																																						#pragma endregion FAIL
																																}	
																																#pragma endregion g_bRetrieve
																													}
																													#pragma endregion bIsSimtor
																										
																									
																								}
																						}
																						else
																						{
																																#pragma region NOT FOUND		
																																if(strcmp(sensors[count].type,"Platform")==0)
																																{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																																}
																																else if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																																{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																																}
																																else if(strcmp(sensors[count].type,"Coils")==0)
																																{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																																}
																																else if(strcmp(sensors[count].type,"Input Registers")==0)
																																{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"r");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"r", NULL);	
																																}
																																else if(strcmp(sensors[count].type,"Holding Registers")==0)
																																{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,"rw");
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,"rw", NULL);	
																																}
																																else
																																{	attr = MSG_AddJSONAttribute(pSensor, "sv");
																																	if(attr)
																																		MSG_SetStringValue(attr,IOT_SGRC_STR_NOT_FOUND,NULL);
																																	attr = MSG_AddJSONAttribute(pSensor, "StatusCode");
																																	if(attr)
																																		MSG_SetDoubleValue(attr, IOT_SGRC_NOT_FOUND,NULL, NULL);	
																																}
																																#pragma endregion NOT FOUND
																						}
																						#pragma endregion MSG_Find_Sensor
																			}
																			#pragma endregion bFormat
															 

													}
													#pragma endregion pSensor
										}
										#pragma endregion pEArray
								}
								#pragma endregion pSensorInofList								
							curNode = curNode->next;
							count++;
							
						}
						{

							repJsonStr = IoT_PrintData(pRoot);
							printf("\nSET Handler_Data = %s\n",repJsonStr);
							printf("---------------------\n");
							if(repJsonStr != NULL)
							{
								g_sendcbf(&g_PluginInfo, modbus_set_sensors_data_rep, repJsonStr, strlen(repJsonStr)+1, NULL, NULL);
							}
							if(repJsonStr)
								free(repJsonStr);
						}
				}
				#pragma endregion pRoot
	}
	#pragma endregion IsSensorInfoListEmpty

	if(pRoot)
		MSG_ReleaseRoot(pRoot);
	if(sensors)
		free(sensors);
	if(sVal)
		free(sVal);

}

//Upload All Sensors' Data
static void UploadAllSensorsData(bool bReport_Upload,void *args)
{	
	handler_context_t *pHandlerContex = (handler_context_t *)args;

	char *repJsonStr = NULL;

	MSG_CLASSIFY_T  *classify=NULL;
	MSG_ATTRIBUTE_T *attr=NULL;

	IoT_READWRITE_MODE mode;

	int i;
	bool bUAI_AO;
	/*
	//-------------------------------------Customize
	char barcode[BARCODE_NUM]={""};
	//-------------------------------------Customize_end
	*/
	#pragma region UploadAllSensorsData
	printf("\nUploadAllSensorsData..........\n");

	if(!g_Capability)
		g_Capability =  CreateCapability();

	if(g_Capability)
	{		
			classify=IoT_FindGroup(g_Capability,"Platform");
			if(classify)
				attr=IoT_FindSensorNode(classify,"Connection");
			if(attr)
			{	mode=IoT_READONLY;

				if(bIsSimtor)
				{
					IoT_SetBoolValue(attr,true,mode);
				}
				else
				{
					if(attr)
						if(!g_bRetrieve)
							IoT_SetBoolValue(attr,false,mode);
						else
							IoT_SetBoolValue(attr,true,mode);
				}	
			}
			classify=IoT_FindGroup(g_Capability,"Discrete Inputs");		
			if(classify)
			{  
				if(numberOfDI!=0)
				{	mode=IoT_READONLY;
					for(i=0;i<numberOfDI;i++)
					{	attr=IoT_FindSensorNode(classify,DI[i].name);
						if(bIsSimtor)
						{
							DI_Bits[i]=rand()%2;
						}
						else
						{
							//if(!g_bRetrieve)
							//	DI_Bits[i]=false;
						}
						if(attr)
						{	
                            DI[i].Bits=DI_Bits[i];
							IoT_SetBoolValue(attr,(bool)DI[i].Bits,mode);
						}
					}
					attr=NULL;
				}
			}
			classify=NULL;
	

			classify=IoT_FindGroup(g_Capability,"Coils");		
			if(classify)
			{  
				if(numberOfDO!=0)
				{	mode=IoT_READWRITE;
					for(i=0;i<numberOfDO;i++)
					{	attr=IoT_FindSensorNode(classify,DO[i].name);
						if(bIsSimtor)
						{
							DO_Bits[i]=rand()%2;
						}
						else
						{
							//if(!g_bRetrieve)
							//	DO_Bits[i]=false;
						}
						if(attr)
						{
                            DO[i].Bits=DO_Bits[i];
							IoT_SetBoolValue(attr,(bool)DO[i].Bits,mode);
						}
					}
					attr=NULL;
				}
			}
			classify=NULL;
			/*
			//-------------------------------------Customize
			classify=IoT_FindGroup(g_Capability,"Barcode");
			if(classify)
			{	mode=IoT_READONLY;
				if(barflag!=0)
				{
					attr=IoT_FindSensorNode(classify,"Barcode");
					if(attr)
					{
						CustomFunc_Str(barcode);
						IoT_SetStringValue(attr,barcode,mode);
					}
				}
			}
			//-------------------------------------Customize_end
			*/

			classify=IoT_FindGroup(g_Capability,"Input Registers");		
			if(classify)
			{  
				if(numberOfAI!=0)
				{	int AIRcur=0;
					bUAI_AO=true;
					mode=IoT_READONLY;
					for(i=0;i<numberOfAI;i++)
					{	attr=IoT_FindSensorNode(classify,AI[i].name);
						if(bIsSimtor)
						{
							while(true)
							{
								if(AI[i].max==0)
								{
									AI_Regs[i]=0;
									break;
								}
								AI_Regs[i]=rand()%(int)AI[i].max/AI[i].precision;
								if(AI_Regs[i]>=AI[i].min)
									break;
								else
									continue;
							}
							if(attr)
								IoT_SetDoubleValueWithMaxMin(attr,AI_Regs[i]*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
						}
						else
						{
							//if(!g_bRetrieve)
							//	AI_Regs[i]=0;
							if(attr)
							{	
								UpDataPrepare(&AI[i],bUAI_AO,&AIRcur,g_bRetrieve);
								if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4)
								{	
									IoT_SetDoubleValueWithMaxMin(attr,AI[i].fv,mode,AI[i].max,AI[i].min,AI[i].unit);
								}
								else if(AI[i].sw_mode==5||AI[i].sw_mode==6)
								{
									IoT_SetDoubleValueWithMaxMin(attr,AI[i].uiv,mode,AI[i].max,AI[i].min,AI[i].unit);
								}
								else if(AI[i].sw_mode==7||AI[i].sw_mode==8)
								{
									IoT_SetDoubleValueWithMaxMin(attr,AI[i].iv,mode,AI[i].max,AI[i].min,AI[i].unit);
								}
								else
								{
									IoT_SetDoubleValueWithMaxMin(attr,AI[i].Regs*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
								}
							}	
						}
						attr=NULL;

					}
			
				}
			}
			classify=NULL;

			classify=IoT_FindGroup(g_Capability,"Holding Registers");		
			if(classify)
			{  
				if(numberOfAO!=0)
				{	int AORcur=0;
					bUAI_AO=false;
					mode=IoT_READWRITE;
					for(i=0;i<numberOfAO;i++)
					{	attr=IoT_FindSensorNode(classify,AO[i].name);
						if(bIsSimtor)
						{
							while(true)
							{
								if(AO[i].max==0)
								{
									AO_Regs[i]=0;
									break;
								}
								AO_Regs[i]=rand()%(int)AO[i].max/AO[i].precision;
								if(AO_Regs[i]>=AO[i].min)
									break;
								else
									continue;
							}
							if(attr)
								IoT_SetDoubleValueWithMaxMin(attr,AO_Regs[i]*AO[i].precision,mode,AO[i].max,AO[i].min,AO[i].unit);
						}
						else
						{
							//if(!g_bRetrieve)
							//	AO_Regs[i]=0;
							if(attr)
							{	
								UpDataPrepare(&AO[i],bUAI_AO,&AORcur,g_bRetrieve);
								if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4)
								{	
									IoT_SetDoubleValueWithMaxMin(attr,AO[i].fv,mode,AO[i].max,AO[i].min,AO[i].unit);
								}
								else if(AO[i].sw_mode==5||AO[i].sw_mode==6)
								{
									IoT_SetDoubleValueWithMaxMin(attr,AO[i].uiv,mode,AO[i].max,AO[i].min,AO[i].unit);
								}
								else if(AO[i].sw_mode==7||AO[i].sw_mode==8)
								{
									IoT_SetDoubleValueWithMaxMin(attr,AO[i].iv,mode,AO[i].max,AO[i].min,AO[i].unit);
								}
								else
								{
									IoT_SetDoubleValueWithMaxMin(attr,AO[i].Regs*AO[i].precision,mode,AO[i].max,AO[i].min,AO[i].unit);
								}
							}	
						}
						attr=NULL;
					}
				}
			}
			classify=NULL;


			repJsonStr = IoT_PrintData(g_Capability);
			printf("\nALL Handler_Data = %s\n",repJsonStr);
			printf("---------------------\n");
			if(bReport_Upload)
			{	
				if(g_sendreportcbf)
				{
						if(Modbus_Log)
						{
							pMSLog=fopen(MSLog_path,"a+");
							if(pMSLog==NULL)
								ModbusLog(g_loghandle, Error, "Fail to Open MS.txt!!");
							else{
								time_t current_time=time(NULL);
								char* c_time_string;

								if (current_time == ((time_t)-1))
									fprintf(stderr, "Failure to obtain the current time.\n");

								c_time_string=ctime(&current_time);

								 if (c_time_string == NULL)
									fprintf(stderr, "Failure to convert the current time.\n");

								fprintf(pMSLog, "%lld, %s, %s\n",current_time,c_time_string, repJsonStr);
								fclose(pMSLog);
						}
					}
					g_sendreportcbf(&g_PluginInfo, repJsonStr, strlen(repJsonStr), NULL, NULL);
			
				}
			}
			else
			{	if(g_sendcbf)
					g_sendcbf(&g_PluginInfo, modbus_auto_upload_rep, repJsonStr, strlen(repJsonStr)+1, NULL, NULL);
			}
			
			free(repJsonStr);
			
	}
	#pragma endregion UploadAllSensorsData




}
//Distinguish Uploading All Data or Partial Data
static void UploadSensorsDataEx(char **Data_paths, int Data_num, bool Reply_All,bool bReport_Upload, void *args)
{
	int Num_Sensor=1000; // to avoid exceeding 
	WISE_Sensor *sensors=(WISE_Sensor *)calloc(Num_Sensor,sizeof(WISE_Sensor));
	MSG_CLASSIFY_T *myUpload = IoT_CreateRoot((char*) strPluginName);

	MSG_CLASSIFY_T  *classify_Find=NULL;
	MSG_ATTRIBUTE_T *attr_Find=NULL;

	MSG_CLASSIFY_T *myGroup=NULL;
	MSG_ATTRIBUTE_T* attr=NULL;
	IoT_READWRITE_MODE mode=IoT_READONLY;

	char * repJsonStr = NULL;
	int Check_Level=0;
	bool Have_name=false;
	int count=0;
	bool bUAI_AO;

	/*
	//-------------------------------------Customize
	char barcode[BARCODE_NUM]={""};
	//-------------------------------------Customize_end
	*/

	for(int i=0;i<Num_Sensor;i++)
	{	strcpy(sensors[i].handler,"");
		strcpy(sensors[i].type,"");
		strcpy(sensors[i].name,"");		
	}
	if(Reply_All)
		UploadAllSensorsData(bReport_Upload,args);
	else
	{			printf("UploadSensorsDataEx.........\n");
				for(count=0;count<Data_num;count++)
					#pragma region Data_paths
					if(Data_paths[count])
					{
							Modbus_General_Paths_Parser(Data_paths[count],sensors,count);
							if(strcmp(sensors[count].name,"")==0)
								Have_name=false;
							else
								Have_name=true;

							#pragma region g_Capability
							if(!g_Capability)
								g_Capability =  CreateCapability();

							if(g_Capability)
							{		
									#pragma region DEF_HANDLER_NAME
									Check_Level=0;
									if(strcmp(sensors[count].handler,strPluginName)==0)
									{		
												Check_Level++;
												classify_Find=IoT_FindGroup(g_Capability,sensors[count].type);
												#pragma region classify_Find
												if(classify_Find)
												{		
														Check_Level++;	
														attr_Find=IoT_FindSensorNode(classify_Find,sensors[count].name);
														if(attr_Find)
															Check_Level++;														
												
												}
												#pragma endregion classify_Find

												#pragma region Check_Level
												switch(Check_Level)
												{
													case 1:	
															#pragma region Check_Level=1
															break;
															#pragma endregion Check_Level=1
													case 2:
															#pragma region Check_Level=2
																	#pragma region Have_name
																	if(!Have_name)
																	{

																					if(strcmp(sensors[count].type,"Platform")==0)
																					{			char Client_Port[6];
																								char Client_UnitID[6];
																								char SlaveID[6];
	

																								myGroup = IoT_AddGroup(myUpload,"Platform");
																								if(myGroup)
																								{			
																											mode=IoT_READONLY;
																											attr = IoT_AddSensorNode(myGroup, "Protocol");
																											if(attr)
																												IoT_SetStringValue(attr,Modbus_Protocol,mode);

																											attr = IoT_AddSensorNode(myGroup, "Name");
																											if(attr)
																												IoT_SetStringValue(attr,Device_Name,mode);

																											if(iTCP_RTU==0)
																											{
																													sprintf(Client_Port,"%d",Modbus_Client_Port);
																													sprintf(Client_UnitID,"%d",Modbus_UnitID);

																													attr = IoT_AddSensorNode(myGroup, "ClientIP");
																													if(attr)
																														IoT_SetStringValue(attr,Modbus_Clent_IP,mode);

																													attr = IoT_AddSensorNode(myGroup, "ClientPort");
																													if(attr)
																														IoT_SetStringValue(attr,Client_Port,mode);

																													attr = IoT_AddSensorNode(myGroup, "UnitID");
																													if(attr)
																														IoT_SetStringValue(attr,Client_UnitID,mode);
																											}
																											else if(iTCP_RTU==1)
																											{
																													sprintf(SlaveID,"%d",Modbus_SlaveID);

																													attr = IoT_AddSensorNode(myGroup, "SlavePort");
																													if(attr)
																														IoT_SetStringValue(attr,Modbus_Slave_Port,mode);

																													attr = IoT_AddSensorNode(myGroup, "Baud");
																													if(attr)
																														IoT_SetDoubleValue(attr,Modbus_Baud,mode,"bps");

																													attr = IoT_AddSensorNode(myGroup, "Parity");
																													if(attr)
																														IoT_SetStringValue(attr,Modbus_Parity,mode);

																													attr = IoT_AddSensorNode(myGroup, "DataBits");
																													if(attr)
																														IoT_SetDoubleValue(attr,Modbus_DataBits,mode,"bits");

																													attr = IoT_AddSensorNode(myGroup, "StopBits");
																													if(attr)
																														IoT_SetDoubleValue(attr,Modbus_StopBits,mode,"bits");

																													attr = IoT_AddSensorNode(myGroup, "SlaveID");
																													if(attr)
																														IoT_SetStringValue(attr,SlaveID,mode);
																											}
																											else
																												;//Not TCP or RTU


																											attr = IoT_AddSensorNode(myGroup, "Connection");

																											if(bIsSimtor)
																											{
																												IoT_SetBoolValue(attr,true,mode);
																											}
																											else
																											{
																												if(attr)
																													if(!g_bRetrieve)
																														IoT_SetBoolValue(attr,false,mode);
																													else
																														IoT_SetBoolValue(attr,true,mode);
																											}
	
																								}

																					}
																					else if(strcmp(sensors[count].type,"Discrete Inputs")==0)
																					{
																								myGroup = IoT_AddGroup(myUpload, "Discrete Inputs");
																								if(myGroup)
																								{	
																											mode=IoT_READONLY;
																											for(int i=0;i<numberOfDI;i++){
																												if(sensors[count].name,DI[i].name)
																												{
																													if(bIsSimtor)
																													{
																														DI_Bits[i]=rand()%2;
																													}
																													else
																													{
																														if(!g_bRetrieve)
																															DI_Bits[i]=false;
																													}

																													attr = IoT_AddSensorNode(myGroup, DI[i].name);
																													if(attr)
																													{
																														DI[i].Bits=DI_Bits[i];
																														IoT_SetBoolValue(attr,DI[i].Bits,mode);	
																													}
																												}

																											}
																								}

																					}
																					else if(strcmp(sensors[count].type,"Coils")==0)
																					{
																								myGroup = IoT_AddGroup(myUpload, "Coils");
																								if(myGroup)
																								{
																											mode=IoT_READWRITE;
																											for(int i=0;i<numberOfDO;i++){
																												if(sensors[count].name,DO[i].name)
																												{
																													if(bIsSimtor)
																													{
																														DO_Bits[i]=rand()%2;
																													}
																													else
																													{
																														if(!g_bRetrieve)
																															DO_Bits[i]=false;
																													}
																													attr = IoT_AddSensorNode(myGroup, DO[i].name);
																													if(attr)
																													{
																														DO[i].Bits=DO_Bits[i];
																														IoT_SetBoolValue(attr,DO[i].Bits,mode);	
																													}
																												}
																											}
																								}

																					}
																					else if(strcmp(sensors[count].type,"Input Registers")==0)
																					{
																								int AIRcur=0;
																								bUAI_AO=true;
																								myGroup = IoT_AddGroup(myUpload, "Input Registers");
																								if(myGroup)
																								{
																											mode=IoT_READONLY;
																											for(int i=0;i<numberOfAI;i++){
																												if(sensors[count].name,AI[i].name)
																												{
																													if(bIsSimtor)
																													{
																														while(true)
																														{
																															if(AI[i].max==0)
																															{
																																AI_Regs[i]=0;
																																break;
																															}
																															AI_Regs[i]=rand()%(int)AI[i].max/AI[i].precision;
																															if(AI_Regs[i]>=AI[i].min)
																																break;
																															else
																																continue;
																														}
																														attr = IoT_AddSensorNode(myGroup, AI[i].name);
																														if(attr)
																															IoT_SetDoubleValueWithMaxMin(attr,AI_Regs[i]*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
																													}
																													else
																													{
																														//if(!g_bRetrieve)
																														//	AI_Regs[i]=0;
																														attr = IoT_AddSensorNode(myGroup, AI[i].name);
																														if(attr)
																														{	
																															
																															UpDataPrepare(&AI[i],bUAI_AO,&AIRcur,g_bRetrieve);
																															if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4)
																															{	
																																IoT_SetDoubleValueWithMaxMin(attr,AI[i].fv,mode,AI[i].max,AI[i].min,AI[i].unit);
																															}
																															else if(AI[i].sw_mode==5||AI[i].sw_mode==6)
																															{	
																																IoT_SetDoubleValueWithMaxMin(attr,AI[i].uiv,mode,AI[i].max,AI[i].min,AI[i].unit);
																															}
																															else if(AI[i].sw_mode==7||AI[i].sw_mode==8)
																															{	
																																IoT_SetDoubleValueWithMaxMin(attr,AI[i].iv,mode,AI[i].max,AI[i].min,AI[i].unit);
																															}
																															else
																															{
																																IoT_SetDoubleValueWithMaxMin(attr,AI[i].Regs*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
																															}
																														}
																													}
																												}

																											}
																								}
																					}
																					else if(strcmp(sensors[count].type,"Holding Registers")==0)
																					{
																								int AORcur=0;
																								bUAI_AO=false;
																								myGroup = IoT_AddGroup(myUpload, "Holding Registers");
																								if(myGroup)
																								{
																											mode=IoT_READWRITE;
																											for(int i=0;i<numberOfAO;i++){
																												if(bIsSimtor)
																												{
																													while(true)
																													{
																														if(AO[i].max==0)
																														{
																															AO_Regs[i]=0;
																															break;
																														}
																														AO_Regs[i]=rand()%(int)AO[i].max/AO[i].precision;
																														if(AO_Regs[i]>=AO[i].min)
																															break;
																														else
																															continue;
																													}
																													attr = IoT_AddSensorNode(myGroup, AO[i].name);
																													if(attr)
																														IoT_SetDoubleValueWithMaxMin(attr,AO_Regs[i]*AO[i].precision,mode,AO[i].max,AO[i].min,AO[i].unit);

																												}
																												else
																												{
																													//if(!g_bRetrieve)
																													//	AO_Regs[i]=0;
																													attr = IoT_AddSensorNode(myGroup, AO[i].name);
																													if(attr)
																													{	
																														UpDataPrepare(&AO[i],bUAI_AO,&AORcur,g_bRetrieve);
																														if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4)
																														{	
																															IoT_SetDoubleValueWithMaxMin(attr,AO[i].fv,mode,AO[i].max,AO[i].min,AO[i].unit);
																														}
																														else if(AO[i].sw_mode==5||AO[i].sw_mode==6)
																														{	
																															IoT_SetDoubleValueWithMaxMin(attr,AO[i].uiv,mode,AO[i].max,AO[i].min,AO[i].unit);
																														}
																														else if(AO[i].sw_mode==7||AO[i].sw_mode==8)
																														{	
																															IoT_SetDoubleValueWithMaxMin(attr,AO[i].iv,mode,AO[i].max,AO[i].min,AO[i].unit);
																														}
																														else
																														{
																															IoT_SetDoubleValueWithMaxMin(attr,AO[i].Regs*AO[i].precision,mode,AO[i].max,AO[i].min,AO[i].unit);
																														}
																													}
																												}
																											}
																								}
																					}
																					/*
																					//----------------------------------------Customize
																					else if(strcmp(sensors[count].type,"Barcode")==0)
																					{
																								myGroup = IoT_AddGroup(myUpload, "Barcode");
																								if(myGroup)
																								{			
																											mode=IoT_READWRITE;
																											attr = IoT_AddSensorNode(myGroup, "Barcode");
																											CustomFunc_Str(barcode);
																											IoT_SetStringValue(attr,barcode,mode);				
																								}
																					}
																					//----------------------------------------Customize_end
																					*/
																	}
																	#pragma endregion Have_name
															

															break;
															#pragma endregion Check_Level=2
													case 3:
															#pragma region Check_Level=3
															if(strcmp(sensors[count].type,"Platform")==0)
															{			char Client_Port[6];
																		char Client_UnitID[6];
																		char SlaveID[6];

																		myGroup = IoT_AddGroup(myUpload,"Platform");
																		if(myGroup)
																		{
																						mode=IoT_READONLY;
																						if(strcmp(sensors[count].name,"Protocol")==0)
																						{
																									attr = IoT_AddSensorNode(myGroup, "Protocol");
																									if(attr)
																										IoT_SetStringValue(attr,Modbus_Protocol,mode);
																						}
																						else if(strcmp(sensors[count].name,"Name")==0)
																						{
																									attr = IoT_AddSensorNode(myGroup, "Name");
																									if(attr)
																										IoT_SetStringValue(attr,Device_Name,mode);
																						}
																						if(iTCP_RTU==0)
																						{
																									sprintf(Client_Port,"%d",Modbus_Client_Port);
																									sprintf(Client_UnitID,"%d",Modbus_UnitID);
																									if(strcmp(sensors[count].name,"ClientIP")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "ClientIP");
																												if(attr)
																													IoT_SetStringValue(attr,Modbus_Clent_IP,mode);
																									}
																									else if(strcmp(sensors[count].name,"ClientPort")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "ClientPort");
																												if(attr)
																													IoT_SetStringValue(attr,Client_Port,mode);
																									}
																									else if(strcmp(sensors[count].name,"UnitID")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "UnitID");
																												if(attr)
																													IoT_SetStringValue(attr,Client_UnitID,mode);
																									}
																						}
																						else if(iTCP_RTU==1)
																						{
																									sprintf(SlaveID,"%d",Modbus_SlaveID);
																									if(strcmp(sensors[count].name,"SlavePort")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "SlavePort");
																												if(attr)
																													IoT_SetStringValue(attr,Modbus_Slave_Port,mode);
																									}
																									else if(strcmp(sensors[count].name,"Baud")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "Baud");
																												if(attr)
																													IoT_SetDoubleValue(attr,Modbus_Baud,mode,"bps");
																									}
																									else if(strcmp(sensors[count].name,"Parity")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "Parity");
																												if(attr)
																													IoT_SetStringValue(attr,Modbus_Parity,mode);
																									}
																									else if(strcmp(sensors[count].name,"DataBits")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "DataBits");
																												if(attr)
																													IoT_SetDoubleValue(attr,Modbus_DataBits,mode,"bits");
																									}
																									else if(strcmp(sensors[count].name,"StopBits")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "StopBits");
																												if(attr)
																													IoT_SetDoubleValue(attr,Modbus_StopBits,mode,"bits");;
																									}
																									else if(strcmp(sensors[count].name,"SlaveID")==0)
																									{
																												attr = IoT_AddSensorNode(myGroup, "SlaveID");
																												if(attr)
																													IoT_SetStringValue(attr,SlaveID,mode);
																									}
																						}
																						else
																							;//Not TCP or RTU
																						
																						if(strcmp(sensors[count].name,"Connection")==0)
																						{
																									attr = IoT_AddSensorNode(myGroup, "Connection");
																											
																									if(bIsSimtor)
																									{
																										IoT_SetBoolValue(attr,true,mode);
																									}
																									else
																									{
																										if(attr)
																											if(!g_bRetrieve)
																												IoT_SetBoolValue(attr,false,mode);
																											else
																												IoT_SetBoolValue(attr,true,mode);
																									}
																						}
																		
																		}

															}
															else if(strcmp(sensors[count].type,"Discrete Inputs")==0)
															{
																		myGroup = IoT_AddGroup(myUpload, "Discrete Inputs");
																		if(myGroup)
																		{
																					mode=IoT_READONLY;
																					for(int i=0;i<numberOfDI;i++){
																						if(strcmp(sensors[count].name,DI[i].name)==0)
																						{
																								if(bIsSimtor)
																								{
																									DI_Bits[i]=rand()%2;
																								}
																								else
																								{
																									//if(!g_bRetrieve)
																									//	DI_Bits[i]=false;
																								}
																								attr = IoT_AddSensorNode(myGroup, DI[i].name);
																								if(attr)
																								{
																									DI[i].Bits=DI_Bits[i];
																									IoT_SetBoolValue(attr,DI[i].Bits,mode);	
																								}
																						}

																					}
																		}

															}
															else if(strcmp(sensors[count].type,"Coils")==0)
															{
																		myGroup = IoT_AddGroup(myUpload, "Coils");
																		if(myGroup)
																		{
																					mode=IoT_READWRITE;
																					for(int i=0;i<numberOfDO;i++){
																						if(strcmp(sensors[count].name,DO[i].name)==0)
																						{
																								if(bIsSimtor)
																								{
																									DO_Bits[i]=rand()%2;
																								}
																								else
																								{
																									//if(!g_bRetrieve)
																									//	DO_Bits[i]=false;
																								}
																								attr = IoT_AddSensorNode(myGroup, DO[i].name);
																								if(attr)
																								{
																									DO[i].Bits=DO_Bits[i];
																									IoT_SetBoolValue(attr,DO[i].Bits,mode);	
																								}
																						}
																					}
																		}

															}
															else if(strcmp(sensors[count].type,"Input Registers")==0)
															{			int AIRcur=0;
																		bUAI_AO=true;
																		myGroup = IoT_AddGroup(myUpload, "Input Registers");
																		if(myGroup)
																		{
																					mode=IoT_READONLY;
																					for(int i=0;i<numberOfAI;i++){
																						if(strcmp(sensors[count].name,AI[i].name)==0)
																						{
																								if(bIsSimtor)
																								{
																									while(true)
																									{
																										if(AI[i].max==0)
																										{
																											AI_Regs[i]=0;
																											break;
																										}
																										AI_Regs[i]=rand()%(int)AI[i].max/AI[i].precision;
																										if(AI_Regs[i]>=AI[i].min)
																											break;
																										else
																											continue;
																									}
																								}
																								else
																								{
																									//if(!g_bRetrieve)
																									//	AI_Regs[i]=0;
																								}
																								attr = IoT_AddSensorNode(myGroup, AI[i].name);
																								if(attr)
																								{	
																									UpDataPrepare(&AI[i],bUAI_AO,&AIRcur,g_bRetrieve);
																									if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4)
																									{	
																										IoT_SetDoubleValueWithMaxMin(attr,AI[i].fv,mode,AI[i].max,AI[i].min,AI[i].unit);
																									}
																									else if(AI[i].sw_mode==5||AI[i].sw_mode==6)
																									{	
																										IoT_SetDoubleValueWithMaxMin(attr,AI[i].uiv,mode,AI[i].max,AI[i].min,AI[i].unit);
																									}
																									else if(AI[i].sw_mode==7||AI[i].sw_mode==8)
																									{	
																										IoT_SetDoubleValueWithMaxMin(attr,AI[i].iv,mode,AI[i].max,AI[i].min,AI[i].unit);
																									}
																									else
																									{
																										IoT_SetDoubleValueWithMaxMin(attr,AI[i].Regs*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
																									}
																								}
																						}

																					}
																		}
															}
															else if(strcmp(sensors[count].type,"Holding Registers")==0)
															{			int AORcur=0;
																		bUAI_AO=false;
																		myGroup = IoT_AddGroup(myUpload, "Holding Registers");
																		if(myGroup)
																		{
																					mode=IoT_READWRITE;
																					for(int i=0;i<numberOfAO;i++){
																						if(strcmp(sensors[count].name,AO[i].name)==0)
																						{
																								if(bIsSimtor)
																								{
																									while(true)
																									{
																										if(AO[i].max==0)
																										{
																											AO_Regs[i]=0;
																											break;
																										}
																										AO_Regs[i]=rand()%(int)AO[i].max/AO[i].precision;
																										if(AO_Regs[i]>=AO[i].min)
																											break;
																										else
																											continue;
																									}
																								}
																								else
																								{
																									//if(!g_bRetrieve)
																									//	AO_Regs[i]=0;
																								}
																								attr = IoT_AddSensorNode(myGroup, AO[i].name);
																								if(attr)
																								{		
																									UpDataPrepare(&AO[i],bUAI_AO,&AORcur,g_bRetrieve);
																									if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4)
																									{	
																										IoT_SetDoubleValueWithMaxMin(attr,AO[i].fv,mode,AO[i].max,AO[i].min,AO[i].unit);
																									}
																									else if(AO[i].sw_mode==5||AO[i].sw_mode==6)
																									{	
																										IoT_SetDoubleValueWithMaxMin(attr,AO[i].uiv,mode,AO[i].max,AO[i].min,AO[i].unit);
																									}
																									else if(AO[i].sw_mode==7||AO[i].sw_mode==8)
																									{	
																										IoT_SetDoubleValueWithMaxMin(attr,AO[i].iv,mode,AO[i].max,AO[i].min,AO[i].unit);
																									}																									
																									else
																									{
																										IoT_SetDoubleValueWithMaxMin(attr,AO[i].Regs*AO[i].precision,mode,AO[i].max,AO[i].min,AO[i].unit);
																									}
																								}
																						}

																					}
																		}

															}
															/*
															//-----------------------------------------Customize
															else if(strcmp(sensors[count].type,"Barcode")==0)
															{			
																		myGroup = IoT_AddGroup(myUpload, "Barcode");
																		if(myGroup)
																		{			
																					
																					mode=IoT_READWRITE;
																					attr = IoT_AddSensorNode(myGroup, "Barcode");	
																					CustomFunc_Str(barcode);
																					IoT_SetStringValue(attr,barcode,mode);
																					
																		}
															}
															//-----------------------------------------Customize_end
															*/
															break;
															#pragma endregion Check_Level=3
												}
												#pragma endregion Check_Level

												

									}
									#pragma endregion DEF_HANDLER_NAME
							}			
							#pragma endregion g_Capability

					
					}
					#pragma endregion Data_paths

					repJsonStr = IoT_PrintData(myUpload);
					printf("\nNOT ALL Handler_Data = %s\n",repJsonStr);
					printf("---------------------\n");
					if(bReport_Upload)
					{	
						if(g_sendreportcbf)
						{
								if(Modbus_Log)
								{
									pMSLog=fopen(MSLog_path,"a+");
									if(pMSLog==NULL)
										ModbusLog(g_loghandle, Error, "Fail to Open MS.txt!!");
									else{
										time_t current_time=time(NULL);
										char* c_time_string;

										if (current_time == ((time_t)-1))
											fprintf(stderr, "Failure to obtain the current time.\n");

										c_time_string=ctime(&current_time);

										 if (c_time_string == NULL)
											fprintf(stderr, "Failure to convert the current time.\n");

										fprintf(pMSLog, "%lld, %s, %s\n",current_time,c_time_string, repJsonStr);
										fclose(pMSLog);
								}
							}
							g_sendreportcbf(&g_PluginInfo, repJsonStr, strlen(repJsonStr), NULL, NULL);
						}
					}					
					else
					{	if(g_sendcbf)
							g_sendcbf(&g_PluginInfo, modbus_auto_upload_rep, repJsonStr, strlen(repJsonStr)+1, NULL, NULL);
					}
					
					free(repJsonStr);
	}


	if(sensors)
		free(sensors);
	if(myUpload)
		IoT_ReleaseAll(myUpload);

}
//--------------------------------------------------------------------------------------------------------------
//------------------------------------------------Threads-------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//Modbus Retrieve Thread
static void* RetrieveThreadStart(void *args)
{

	handler_context_t *pHandlerContex = (handler_context_t *)args;

	#pragma region Modbus_Retrieve
	while (g_PluginInfo.agentInfo->status == 0)
	{
		if(!pHandlerContex->isThreadRunning && bAllDataAlloc)
			return 0;
		sleep(1);
	}
	
	while(pHandlerContex->isThreadRunning)
	{ 
		if(bAllDataAlloc)
			if(!bIsSimtor && ctx!=NULL)
				g_bRetrieve=Modbus_Rev();
		usleep(500*1000);	//To fix yocto
	}
	#pragma endregion Modbus_Retrieve

	return 0;

}

//Auto Report Thread
static void* AutoReportThreadStart(void *args)
{	

	handler_context_t *pHandlerContex = (handler_context_t *)args;
	int i = 0;
	bool bReport_Upload;
	time_t lock_time;
	time_t current_time;

	while (g_PluginInfo.agentInfo->status == 0 && bAllDataAlloc)
	{
		if(!pHandlerContex->isThreadRunning)
			return 0;
		sleep(1);
	}

	while(g_AutoReportContex.isThreadRunning)
	{
		#pragma region g_bAutoReport
		if(bAllDataAlloc)
		{
			if(g_bAutoReport)
			{	
				bReport_Upload=true;
				printf("\nAutoReport...........\n");
				UploadSensorsDataEx(Report_Data_paths,Report_array_size,Report_Reply_All,bReport_Upload,args);
				//sleep(Report_interval);
				lock_time=time(NULL);
				while(true)
				{
					current_time=time(NULL);
					if(current_time-lock_time>=Report_interval)
						break;
					usleep(100*1000);
				}
			}
			else
				sleep(1);
		}
		else
			sleep(1);
		#pragma endregion g_bAutoReport


	}
	
	return 0;

}




//Auto Upload Thread
static void* AutoUploadThreadStart(void *args)
{
	bool bReport_Upload;
	char tmpRepFilter[4096] = {0};
	cJSON *root=NULL;
	unsigned int diff=0;

	handler_context_t *pHandlerContex = (handler_context_t *)args;
	int i = 0;

	while (g_PluginInfo.agentInfo->status == 0 && bAllDataAlloc)
	{
		if(!pHandlerContex->isThreadRunning)
			return 0;
		sleep(1);
	}

	while(g_AutoUploadContex.isThreadRunning)
	{
		if(bAllDataAlloc)
		{
				#pragma region g_bAutoUpload
				if(g_bAutoUpload)
				{
							bool Reply_All=false;
							char **Data_paths=NULL;
							int array_size=0;

							cJSON *item, *it, *js_name,*js_list;

							#pragma region Reply_All 
							if(!Reply_All)
							{
									#pragma region root 
									root = cJSON_Parse(AutoUploadParams.repFilter);
									if(!root) 
									{
										printf("get root faild !\n");
										
									}
									else
									{
										js_list = cJSON_GetObjectItem(root, "e");
										#pragma region js_list
										if(!js_list)
										{
											printf("no list!\n");
										}
										else
										{	
											array_size = cJSON_GetArraySize(js_list);
											Data_paths = (char **)calloc(array_size, sizeof(char *));

											char *p  = NULL;
						
											for(i=0; i< array_size; i++)
											{

													item = cJSON_GetArrayItem(js_list, i);
													if(Data_paths)
														Data_paths[i] = (char *)calloc(200, sizeof(char));   //set Data_path size as 200*char
													if(!item)    
													{
														printf("no item!\n");
													}
													else
													{
														p = cJSON_PrintUnformatted(item);
														it = cJSON_Parse(p);
														
														if(!it)
															continue;
														else
														{
															js_name = cJSON_GetObjectItem(it, "n");
															if(Data_paths)
																if(Data_paths[i])
																	strcpy(Data_paths[i],js_name->valuestring);
														}

														if(p)
															free(p);
														if(it)
															cJSON_Delete(it);

														if(strcmp(Data_paths[i],strPluginName)==0)
														{	Reply_All=true;
															printf("Reply_All=true;\n");
															break;
														}
																		
													}
					
											}
															
										}
										#pragma endregion js_list
												
									}
									
									if(root)
										cJSON_Delete(root);
									#pragma endregion root
							}
							#pragma endregion Reply_All

							Continue_Upload=time(NULL);

							diff=(unsigned int)difftime(Continue_Upload,Start_Upload);
							printf("timeout: %d\n",AutoUploadParams.continueTimeMs/1000);
							printf("diff: %d\n",diff);

							if(diff<AutoUploadParams.continueTimeMs/1000)
							{	
								bReport_Upload=false;
								UploadSensorsDataEx(Data_paths,array_size,Reply_All,bReport_Upload,args);
								sleep(AutoUploadParams.intervalTimeMs/1000);
							}
							else
							{
								g_bAutoUpload=false;
							}
							for(i=0;i<array_size; i++)
								if(Data_paths)
									if(Data_paths[i])
										free(Data_paths[i]);
							if(Data_paths)
								free(Data_paths);

				}
				else
					sleep(1);
				#pragma endregion g_bAutoUpload
		}
		else
			sleep(1);

	}
	
	return 0;
}

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------Handler Functions-------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
/* **************************************************************************************
 *  Function Name: Handler_Initialize
 *  Description: Init any objects or variables of this handler
 *  Input :  PLUGIN_INFO *pluginfo
 *  Output: None
 *  Return:  0  : Success Init Handler
 *              -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLER_API Handler_Initialize( HANDLER_INFO *pluginfo )
{
	if( pluginfo == NULL )
		return handler_fail;

	g_loghandle = pluginfo->loghandle;

	// 1. Topic of this handler
	strPluginName=(char *)calloc(strlen(pluginfo->Name)+1,sizeof(char));	
	strcpy(strPluginName,pluginfo->Name);

	pluginfo->RequestID = iRequestID;
	pluginfo->ActionID = iActionID;
	printf(" >Name: %s\r\n", strPluginName);
	// 2. Copy agent info 
	memcpy(&g_PluginInfo, pluginfo, sizeof(HANDLER_INFO));
	g_PluginInfo.agentInfo = pluginfo->agentInfo;

	// 3. Callback function -> Send JSON Data by this callback function
	g_sendcbf = g_PluginInfo.sendcbf = pluginfo->sendcbf;
	g_sendcustcbf = g_PluginInfo.sendcustcbf = pluginfo->sendcustcbf;
	g_subscribecustcbf = g_PluginInfo.subscribecustcbf = pluginfo->subscribecustcbf;
	g_sendreportcbf = g_PluginInfo.sendreportcbf = pluginfo->sendreportcbf;
	g_sendcapabilitycbf =g_PluginInfo.sendcapabilitycbf = pluginfo->sendcapabilitycbf;
	g_sendeventcbf = g_PluginInfo.sendeventcbf = pluginfo->sendeventcbf;

	g_RetrieveContex.threadHandler = NULL;
	g_RetrieveContex.isThreadRunning = false;
	g_AutoReportContex.threadHandler = NULL;
	g_AutoReportContex.isThreadRunning = false;
	g_AutoUploadContex.threadHandler = NULL;
	g_AutoUploadContex.isThreadRunning = false;

	
	g_status = handler_status_init;
	

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Uninitialize
 *  Description: Release the objects or variables used in this handler
 *  Input :  None
 *  Output: None
 *  Return:  void
 * ***************************************************************************************/
void Handler_Uninitialize()
{
	g_sendcbf = NULL;
	g_sendcustcbf = NULL;
	g_sendreportcbf = NULL;
	g_sendcapabilitycbf = NULL;
	g_subscribecustcbf = NULL;
	if(strPluginName)
		free(strPluginName);

	if(MRLog_path)
		free(MRLog_path);
	if(MSLog_path)
		free(MSLog_path);

	if(pMRLog)
		fclose(pMRLog);
	if(pMSLog)
		fclose(pMSLog);

	if(g_Capability)
	{
		IoT_ReleaseAll(g_Capability);
		g_Capability = NULL;
	}

}

/* **************************************************************************************
 *  Function Name: Handler_Get_Status
 *  Description: Get Handler Threads Status. CAgent will restart current Handler or restart CAgent self if busy.
 *  Input :  None
 *  Output: char * : pOutStatus       // cagent handler status
 *  Return:  handler_success  : Success Init Handler
 *			 handler_fail : Fail Init Handler
 * **************************************************************************************/
int HANDLER_API Handler_Get_Status( HANDLER_THREAD_STATUS * pOutStatus )
{
	int iRet = handler_fail; 

	if(!pOutStatus) return iRet;

	switch (g_status)
	{
	default:
	case handler_status_no_init:
	case handler_status_init:
	case handler_status_stop:
		*pOutStatus = g_status;
		break;
	case handler_status_start:
	case handler_status_busy:
		{
			/*time_t tv;
			time(&tv);
			if(difftime(tv, g_monitortime)>5)
				g_status = handler_status_busy;
			else
				g_status = handler_status_start;*/
			*pOutStatus = g_status;
		}
		break;
	}
	
	iRet = handler_success;
	return iRet;
}


/* **************************************************************************************
 *  Function Name: Handler_OnStatusChange
 *  Description: Agent can notify handler the status is changed.
 *  Input :  PLUGIN_INFO *pluginfo
 *  Output: None
 *  Return:  None
 * ***************************************************************************************/
void HANDLER_API Handler_OnStatusChange( HANDLER_INFO *pluginfo )
{
	printf(" %s> Update Status", strPluginName);
	if(pluginfo)
		memcpy(&g_PluginInfo, pluginfo, sizeof(HANDLER_INFO));
	else
	{
		memset(&g_PluginInfo, 0, sizeof(HANDLER_INFO));
		snprintf( g_PluginInfo.Name, sizeof( g_PluginInfo.Name), "%s", strPluginName );
		g_PluginInfo.RequestID = iRequestID;
		g_PluginInfo.ActionID = iActionID;
	}
}


/* **************************************************************************************
 *  Function Name: Handler_Start
 *  Description: Start Running
 *  Input :  None
 *  Output: None
 *  Return:  0  : Success Init Handler
 *              -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLER_API Handler_Start( void )
{
	char* result = NULL;
	char modulePath[200]={0};
	char iniPath[200]={0};
	printf("> %s Handler_Start\r\n", strPluginName);

	srand(time(NULL));

	// Load ini file
	bFind=read_INI_Platform(modulePath,iniPath);

	if(Modbus_Log)
	{
		MRLog_path=(char *)calloc(1,strlen(modulePath)+strlen("MR.txt")+1);
		MSLog_path=(char *)calloc(1,strlen(modulePath)+strlen("MS.txt")+1);
		strcpy(MRLog_path,modulePath);
		strcpy(MSLog_path,modulePath);
		strcat(MRLog_path,"MR.txt");
		strcat(MSLog_path,"MS.txt");
	}
		

	
	g_RetrieveContex.isThreadRunning = true;
	g_AutoReportContex.isThreadRunning = true;
	g_AutoUploadContex.isThreadRunning = true;
	
	if(bFind)
	{	AI_Regs_temp_Ary = (uint16_t *)calloc(2,sizeof(uint16_t));
		AO_Regs_temp_Ary = (uint16_t *)calloc(2,sizeof(uint16_t));
		AO_Set_Regs_temp_Ary= (uint16_t *)calloc(2,sizeof(uint16_t));
		if(!bIsSimtor)
		{
					if(iTCP_RTU==0)
					{
						ctx = modbus_new_tcp(Modbus_Clent_IP, Modbus_Client_Port);
						Modbus_Connect();
					}
					else if(iTCP_RTU==1)
					{
						char com_str[16]="";
						char parity='N';

						#if defined(_WIN32) || defined(WIN32)
							sprintf(com_str,"\\\\.\\%s",Modbus_Slave_Port);
						#else
							sprintf(com_str,"/dev/%s",Modbus_Slave_Port);
						#endif
						printf("com_str : %s\n", com_str);

						if(strcmp(Modbus_Parity,"None")==0)
							parity='N';
						else if(strcmp(Modbus_Parity,"Even")==0)
							parity='E';
						else if(strcmp(Modbus_Parity,"Odd")==0)
							parity='O';
						else
							;	// Not N, E, O

						ctx = modbus_new_rtu(com_str, Modbus_Baud, parity, Modbus_DataBits, Modbus_StopBits);
						Modbus_Connect();
					}
					else
					{
						printf("Protocol error!!\n");
						ctx=NULL;
					}
		}
		
	}
	else
	{
		printf("INI file not found!!\n");
		ctx=NULL;
	}

	if(pthread_create(&g_RetrieveContex.threadHandler,NULL, RetrieveThreadStart, &g_RetrieveContex) != 0)
	{
		g_RetrieveContex.isThreadRunning = false;
		printf("> start Retrieve thread failed!\r\n");	
		return handler_fail;
    }

	if(pthread_create(&g_AutoReportContex.threadHandler,NULL, AutoReportThreadStart, &g_AutoReportContex) != 0)
	{
		g_AutoReportContex.isThreadRunning = false;
		printf("> start AutoReport thread failed!\r\n");	
		return handler_fail;
    }

	if(pthread_create(&g_AutoUploadContex.threadHandler,NULL, AutoUploadThreadStart, &g_AutoUploadContex) != 0)
	{
		g_AutoUploadContex.isThreadRunning = false;
		printf("> start AutoUpload thread failed!\r\n");	
		return handler_fail;
    }

	g_status = handler_status_start;


	time(&g_monitortime);
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Stop
 *  Description: Stop the handler
 *  Input :  None
 *  Output: None
 *  Return:  0  : Success Init Handler
 *              -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLER_API Handler_Stop( void )
{
	void *status;
	
	if(g_RetrieveContex.isThreadRunning == true)
	{
		g_RetrieveContex.isThreadRunning = false;
		pthread_join((pthread_t)g_RetrieveContex.threadHandler,&status);
		g_RetrieveContex.threadHandler = NULL;
	}

	if(g_AutoReportContex.isThreadRunning == true)
	{
		int i=0;
		g_AutoReportContex.isThreadRunning = false;
		pthread_join((pthread_t)g_AutoReportContex.threadHandler,&status);
		g_AutoReportContex.threadHandler = NULL;

		for(i=0;i<Report_array_size; i++)
		{
			if(Report_Data_paths!=NULL)
				if(Report_Data_paths[i]!=NULL)
					free(Report_Data_paths[i]);
		}
		if(Report_Data_paths!=NULL)
		{
			free(Report_Data_paths);
			Report_Data_paths=NULL;
		}
	}

	if(g_AutoUploadContex.isThreadRunning == true)
	{

		g_AutoUploadContex.isThreadRunning = false;
		pthread_join((pthread_t)g_AutoUploadContex.threadHandler,&status);
		g_AutoUploadContex.threadHandler = NULL;
	}


	if(DI_Bits)
		free(DI_Bits);
	if(DO_Bits)
		free(DO_Bits);
	if(AI_Regs)
		free(AI_Regs);
	if(AI_Regs_temp_Ary)
		free(AI_Regs_temp_Ary);
	if(AO_Regs)
		free(AO_Regs);
	if(AO_Regs_temp_Ary)
		free(AO_Regs_temp_Ary);
	if(AO_Set_Regs_temp_Ary)
		free(AO_Set_Regs_temp_Ary);
	if(DI)
		free(DI);
	if(DO)
		free(DO);
	if(AI)
		free(AI);
	if(AO)
		free(AO);
	if(modbus_get_socket(ctx)>0)
		Modbus_Disconnect();			
	modbus_free(ctx);

	g_status = handler_status_stop;
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Recv
 *  Description: Receive Packet from MQTT Server
 *  Input : char * const topic, 
 *			void* const data, 
 *			const size_t datalen
 *  Output: void *pRev1, 
 *			void* pRev2
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_Recv(char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2  )
{
	int cmdID = 0;
	char errorStr[128] = {0};
	ModbusLog(g_loghandle, Normal, " %s>Recv Topic [%s] Data %s", strPluginName, topic, (char*) data );
	printf(" >Recv Topic [%s] Data %s", topic, (char*) data );

	if(!ParseReceivedData(data, datalen, &cmdID))
		return;
	switch(cmdID)
	{
		case modbus_get_capability_req:
		{
			GetCapability();
			break;
		}

		case modbus_get_sensors_data_req:
		{
			char curSessionID[256] = {0};
			sensor_info_list sensorInfoList = CreateSensorInfoList();
			if(Parser_ParseGetSensorDataReqEx(data, sensorInfoList, curSessionID))
			{
				if(strlen(curSessionID))
				{   
					if(sensorInfoList != NULL || curSessionID != NULL)	
						GetSensorsDataEx(sensorInfoList, curSessionID);	
				}
			}
			else
			{
				char * errorRepJsonStr = NULL;
				char errorStr[128];
				sprintf(errorStr, "Command(%d) parse error!", modbus_set_sensors_data_req);
				int jsonStrlen = Parser_PackModbusError(errorStr, &errorRepJsonStr);

				if(jsonStrlen > 0 && errorRepJsonStr != NULL)
				{
					g_sendcbf(&g_PluginInfo, modbus_error_rep, errorRepJsonStr, strlen(errorRepJsonStr)+1, NULL, NULL);
				}
				if(errorRepJsonStr)free(errorRepJsonStr);
			}
			DestroySensorInfoList(sensorInfoList);
			break;

		}

		case modbus_set_sensors_data_req:
		{
			char curSessionID[256] = {0};
			sensor_info_list sensorInfoList = CreateSensorInfoList();
			if(Parser_ParseSetSensorDataReqEx(data, sensorInfoList, curSessionID))
			{
				if(strlen(curSessionID))
				{   
					if(sensorInfoList != NULL || curSessionID != NULL)
						SetSensorsDataEx(sensorInfoList, curSessionID);
				}
			}
			else
			{
				char * errorRepJsonStr = NULL;
				char errorStr[128];
				sprintf(errorStr, "Command(%d) parse error!", modbus_set_sensors_data_req);
				int jsonStrlen = Parser_PackModbusError(errorStr, &errorRepJsonStr);
				//printf("error : %s\n",errorStr);
				if(jsonStrlen > 0 && errorRepJsonStr != NULL)
				{
					g_sendcbf(&g_PluginInfo, modbus_error_rep, errorRepJsonStr, strlen(errorRepJsonStr)+1, NULL, NULL);
				}
				if(errorRepJsonStr)free(errorRepJsonStr);
			}
			DestroySensorInfoList(sensorInfoList);
			break;

		}

		case modbus_auto_upload_req:
		{

			unsigned int intervalTimeMs = 0; //ms
			unsigned int continueTimeMs = 0;
			char tmpRepFilter[4096] = {0};
			bool bRet = Parser_ParseAutoUploadCmd((char *)data, &intervalTimeMs, &continueTimeMs, tmpRepFilter);

			if(bRet)
			{
				AutoUploadParams.intervalTimeMs = intervalTimeMs; //ms
				AutoUploadParams.continueTimeMs = continueTimeMs;
				memset(AutoUploadParams.repFilter, 0, sizeof(AutoUploadParams.repFilter));
				if(strlen(tmpRepFilter)) strcpy(AutoUploadParams.repFilter, tmpRepFilter);
				g_bAutoUpload=true;
				Start_Upload=time(NULL);
			}
			else
			{
				char * errorRepJsonStr = NULL;
				char errorStr[128];
				sprintf(errorStr, "Command(%d) parse error!", modbus_set_sensors_data_req);
				int jsonStrlen = Parser_PackModbusError(errorStr, &errorRepJsonStr);

				if(jsonStrlen > 0 && errorRepJsonStr != NULL)
				{
					g_sendcbf(&g_PluginInfo, modbus_error_rep, errorRepJsonStr, strlen(errorRepJsonStr)+1, NULL, NULL);
				}
				if(errorRepJsonStr)free(errorRepJsonStr);
			}
			
			break;

		}

	}

}

/* **************************************************************************************
 *  Function Name: Handler_AutoReportStart
 *  Description: Start Auto Report
 *  Input : char *pInQuery
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
//FULL_FUNC : For distinghishing handler's name having Modbus_Handler or not in AutoReport
//No FULL_FUNC : Not dinsinghish handler's name having Modbus_Handler or not in AutoReport
void HANDLER_API Handler_AutoReportStart(char *pInQuery)
{

#ifdef FULL_FUNC	//Server doesnt support Modbus_Handler now  
	int i=0; 
	
	if(g_bAutoReport)
	{
		for(i=0;i<Report_array_size; i++)
		{
			if(Report_Data_paths!=NULL)
				if(Report_Data_paths[i]!=NULL)
					free(Report_Data_paths[i]);
		}
		if(Report_Data_paths!=NULL)
		{
			free(Report_Data_paths);
			Report_Data_paths=NULL;
		}
	}

	#pragma region root 
	//printf("\n\npInQuery = %s\n\n",pInQuery);
	Report_item=NULL;
	Report_it=NULL;
	Report_js_name=NULL;
	Report_first=NULL;
	Report_second_interval=NULL;
	Report_second=NULL;
	Report_third=NULL;
	Report_js_list=NULL;

	Report_root=NULL;
	Report_Reply_All=false;
	Report_Data_paths=NULL;
	Report_array_size=0;
	
	Report_root = cJSON_Parse(pInQuery);
	#pragma region Report_root
	if(!Report_root) 
	{
		printf("get root failed !\n");
		
	}
	else
	{	
				Report_first=cJSON_GetObjectItem(Report_root,"susiCommData");
				if(Report_first)
				{
					Report_second_interval=cJSON_GetObjectItem(Report_first,"autoUploadIntervalSec");
					if(Report_second_interval)
					{		Report_interval=Report_second_interval->valueint;
							//printf("interval : %d\n",Report_interval);
							Report_second=cJSON_GetObjectItem(Report_first,"requestItems");
							if(Report_second)
							{	Report_third=cJSON_GetObjectItem(Report_second,"All");
								if(Report_third)
									Report_Reply_All=true;
								else
								{
								//Report_third=cJSON_GetObjectItem(Report_second,DEF_HANDLER_NAME);
									Report_third=cJSON_GetObjectItem(Report_second,strPluginName);
								if(Report_third)
									Report_js_list=cJSON_GetObjectItem(Report_third,"e");

								}
												
							}	
					}
				}

				#pragma region Report_Reply_All
				if(!Report_Reply_All)
				{		
						#pragma region Report_js_list
						if(!Report_js_list)
						{
							printf("no list!\n");
							g_bAutoReport=false;
						}
						else
						{	
							Report_array_size = cJSON_GetArraySize(Report_js_list);
							Report_Data_paths = (char **)calloc(Report_array_size, sizeof(char *));

							char *p  = NULL;

							for(i=0; i< Report_array_size; i++)
							{

									Report_item = cJSON_GetArrayItem(Report_js_list, i);
									if(Report_Data_paths)
										Report_Data_paths[i] = (char *)calloc(200, sizeof(char));   //set Data_path size as 200*char
									if(!Report_item)    
									{
										printf("no item!\n");
									}
									else
									{
										p = cJSON_PrintUnformatted(Report_item);
										Report_it = cJSON_Parse(p);
										
										if(!Report_it)
											continue;
										else
										{
											Report_js_name = cJSON_GetObjectItem(Report_it, "n");
											if(Report_Data_paths)
												if(Report_Data_paths[i])
													strcpy(Report_Data_paths[i],Report_js_name->valuestring);
											//printf("Data : %s\n",Report_Data_paths[i]);
										}

										if(p)
											free(p);
										if(Report_it)
											cJSON_Delete(Report_it);

										if(strcmp(Report_Data_paths[i],strPluginName)==0)
										{	Report_Reply_All=true;
											printf("Report_Reply_All=true;\n");
											break;
										}
														
									}

							}
							if(Report_third)
								g_bAutoReport=true;
							else
								g_bAutoReport=false;
											
						}
						#pragma endregion Report_js_list
				}
				else
				{	printf("Report_Reply_All=true;\n");
					if(Report_third)
						g_bAutoReport=true;
					else
						g_bAutoReport=false;
				}
				#pragma endregion Report_Reply_All
	}
	
	if(Report_root)
		cJSON_Delete(Report_root);
	#pragma endregion Report_root
#else
	Report_root=NULL;
	Report_root = cJSON_Parse(pInQuery);
	if(!Report_root) 
	{
		printf("get root failed !\n");
		
	}
	else
	{	
		Report_first=cJSON_GetObjectItem(Report_root,"susiCommData");
		if(Report_first)
		{
			Report_second_interval=cJSON_GetObjectItem(Report_first,"autoUploadIntervalSec");
			if(Report_second_interval)
			{	
				Report_interval=Report_second_interval->valueint;
			}
		}
	}
	if(Report_root)
		cJSON_Delete(Report_root);
	printf("Reply_All=true;\n");
	//Report_interval=1;
	Report_Reply_All=true;
	g_bAutoReport=true;
#endif
	
}

/* **************************************************************************************
 *  Function Name: Handler_AutoReportStop
 *  Description: Stop Auto Report
 *  Input : None
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_AutoReportStop(char *pInQuery)
{
	/*TODO: Parsing received command*/
#ifdef FULL_FUNC	//Server doesnt support Modbus_Handler now  
	int i=0;
	for(i=0;i<Report_array_size; i++)
	{
		if(Report_Data_paths!=NULL)
			if(Report_Data_paths[i]!=NULL)
				free(Report_Data_paths[i]);
	}
	if(Report_Data_paths!=NULL)
	{
		free(Report_Data_paths);
		Report_Data_paths=NULL;
	}
	g_bAutoReport = false;
#else
	g_bAutoReport = false;
#endif

}

/* **************************************************************************************
 *  Function Name: Handler_Get_Capability
 *  Description: Get Handler Information specification. 
 *  Input :  None
 *  Output: char ** : pOutReply       // JSON Format
 *  Return:  int  : Length of the status information in JSON format
 *                :  0 : no data need to trans
 * **************************************************************************************/
int HANDLER_API Handler_Get_Capability( char ** pOutReply ) // JSON Format
{
	char* result = NULL;
	int len = 0;
	if(!pOutReply) return len;
	bFind = read_INI();
	if(g_Capability)
	{
		IoT_ReleaseAll(g_Capability);
		g_Capability = NULL;
	}
	if(bFind)
		bAllDataAlloc=true;

	g_Capability = CreateCapability();
	
	result = IoT_PrintCapability(g_Capability);
   
	printf("Handler_Get_Capability=%s\n",result);
	printf("---------------------\n");

	len = strlen(result);
	*pOutReply = (char *)malloc(len + 1);
	memset(*pOutReply, 0, len + 1);
	strcpy(*pOutReply, result);      
	free(result);
	return len;
}

