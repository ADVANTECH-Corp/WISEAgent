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
#include "AdvPlatform.h"
#include "pthread.h"
#include "util_path.h"
#include "unistd.h"
#include "susiaccess_handler_api.h"
#include "IoTMessageGenerate.h"
#include "Modbus_HandlerLog.h"
#include "Modbus_Parser.h"
#include "ReadINI.h"
#include "HandlerKernel.h"

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

//-----------------------------------------------------------------------------
// Internal Prototypes:
//-----------------------------------------------------------------------------
//

typedef struct{
   pthread_t threadHandler;
   bool isThreadRunning;
}handler_context_t;

typedef struct WISE_Data_t
{
	char name[SENSOR_NAME_LENGTH];
	int  address;
	double max;
	double min;
	double precision;
	uint8_t Bits;
	uint16_t Regs;
	float fv;
	uint32_t uiv;		//32 bits unsigned int
	int iv;
	char unit[10];
	int sw_mode;
	int ad_mode;
	bool bRevFin;
	MSG_ATTRIBUTE_T* attr;
}WISE_Data;


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

static handler_context_t g_RetrieveContex;

static HANDLER_THREAD_STATUS g_status = handler_status_no_init;
static bool g_bRetrieve = false;
static Handler_info  g_HandlerInfo; //global Handler info structure
static HandlerSendCbf  g_sendcbf = NULL;						// Client Sends information (in JSON format) to Cloud Server	

static char * CurIotDataJsonStr = NULL;
//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
//const char strPluginName[MAX_TOPIC_LEN] = {"Modbus_Handler"};
char *strPluginName = NULL;										//be used to set the customized handler name by module_config.xml
const int iRequestID = cagent_request_custom;
const int iActionID = cagent_custom_action;
MSG_CLASSIFY_T *g_Capability = NULL;
MSG_ATTRIBUTE_T* g_ConnectAttr = NULL;
int Rev_Fail_Num=0;
bool g_bRev_Fail=false;

bool bConnectionFlag=false;
bool g_bIsSimtor=false;
bool g_bFind=false; //Find INI file
int  iTCP_RTU=0;
char Modbus_Protocol[20]={0};
char Device_Name[20]="";
//--------Modbus_TCP
char Modbus_Clent_IP[16]={0}; 
int Modbus_Client_Port=502;
int Modbus_UnitID=1;
//--------Modbus_RTU
char Modbus_Slave_Port[6]={0}; 
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

WISE_Data *DI;
WISE_Data *AI;
WISE_Data *DO;
WISE_Data *AO;

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

//==========================================================
//read platform section in INI file
bool read_INI_Platform(char *modulePath,char *iniPath)
{
	FILE *fPtr;
	char temp_INI_name[DEF_MAX_PATH]={0};
	sprintf(temp_INI_name, "%s.ini", strPluginName);
	// Load ini file
	util_module_path_get(modulePath);
	util_path_combine(iniPath,modulePath,temp_INI_name);

	//printf("iniFile: %s\n",iniPath);

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
		}
		else
		{
			printf("Protocol error!!\n");
			iTCP_RTU=-1;	
		}

		if(strcmp(Device_Name,DEF_SIMULATOR_NAME)==0)
		{
			printf("bIsSimtor=true;\n");		
			g_bIsSimtor=true;
		}
		else
		{
			printf("bIsSimtor=false;\n");	
			g_bIsSimtor=false;
		}

        fclose(fPtr);
		return true;
    }
    else {
        printf("INI Opened Failed...\n");
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

	char temp_INI_name[DEF_MAX_PATH]={0};
	
	sprintf(temp_INI_name, "%s.ini", strPluginName);
	// Load ini file
	util_module_path_get(modulePath);
	util_path_combine(iniPath,modulePath,temp_INI_name);

	//printf("iniFile: %s\n",iniPath);

    fPtr = fopen(iniPath, "r");

	if (fPtr) {				
		printf("INI Opened Successful...\n");
		#pragma region Find_INI
		strcpy(Modbus_Protocol,GetIniKeyString("Platform","Protocol",iniPath));
		strcpy(Device_Name,GetIniKeyString("Platform","Name",iniPath));

		if(strcmp(Modbus_Protocol,DEF_MODBUS_TCP)==0)
		{	
			printf("Protocol : Modbus_TCP\n");
			iTCP_RTU=0;
			
			strcpy(Modbus_Clent_IP,GetIniKeyString("Platform","ClientIP",iniPath));
			Modbus_Client_Port=GetIniKeyInt("Platform","ClientPort",iniPath);
			Modbus_UnitID=GetIniKeyInt("Platform","UnitID",iniPath);
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
		}
		else
		{
			printf("Protocol error!!\n");
			iTCP_RTU=-1;	
		}

		if(strcmp(Device_Name,DEF_SIMULATOR_NAME)==0)
		{
			printf("bIsSimtor=true;\n");		
			g_bIsSimtor=true;
		}
		else
		{
			printf("bIsSimtor=false;\n");	
			g_bIsSimtor=false;
		}

		//--------------------DI
		//numberOfDI=GetPrivateProfileInt("Discrete Inputs", "numberOfDI", 0, iniPath); 
		numberOfDI=GetIniKeyInt("Discrete Inputs","numberOfIB",iniPath);
		if(numberOfDI!=0)
		{	
			DI=(WISE_Data *)calloc(numberOfDI,sizeof(WISE_Data));
		}
		for(int i=0;i<numberOfDI;i++){
			char strNumberOfDI[10];
			DI[i].Regs=0;
			DI[i].Bits=0;
			DI[i].fv=0;
			DI[i].uiv=0;
			DI[i].iv=0;
			DI[i].bRevFin=false;	
			DI[i].ad_mode = 0;

			sprintf(strNumberOfDI, "IB%d", i);
			//GetPrivateProfileString("Discrete Inputs", strNumberOfDI, "", str, sizeof(str), iniPath); 			
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
		//numberOfDO=GetPrivateProfileInt("Coils", "numberOfDO", 0, iniPath); 
		numberOfDO=GetIniKeyInt("Coils","numberOfB",iniPath);
		if(numberOfDO!=0)
		{
			DO=(WISE_Data *)calloc(numberOfDO,sizeof(WISE_Data));	
		}
		for(int i=0;i<numberOfDO;i++){
			char strNumberOfDO[10];
			DO[i].Regs=0;
			DO[i].Bits=0;
			DO[i].fv=0;
			DO[i].uiv=0;
			DO[i].iv=0;
			DO[i].bRevFin=false;
			DO[i].ad_mode = 1;

			sprintf(strNumberOfDO, "B%d", i);
			//GetPrivateProfileString("Coils", strNumberOfDO, "", str, sizeof(str), iniPath); 			
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
			AI[i].uiv=0;
			AI[i].iv=0;
			AI[i].bRevFin=false;
			AI[i].ad_mode = 2;

			sprintf(strNumberOfAI, "IR%d", i);
			//GetPrivateProfileString("Input Registers", strNumberOfAI, "", str, sizeof(str), iniPath); 		
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

			printf("%d. AI address: %d name: %s min: %lf max: %lf pre: %lf unit: %s sw_mode: %d\n",i,AI[i].address,AI[i].name,AI[i].min,AI[i].max,AI[i].precision,AI[i].unit,AI[i].sw_mode);
		}
		
		//--------------------AO
		pstr=NULL;
		//numberOfAO=GetPrivateProfileInt("Holding Registers", "numberOfAO", 0, iniPath); 
		numberOfAO=GetIniKeyInt("Holding Registers","numberOfR",iniPath);
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
			AO[i].uiv=0;
			AO[i].iv=0;
			AO[i].bRevFin=false;
			AO[i].ad_mode = 3;

			sprintf(strNumberOfAO, "R%d", i);
			//GetPrivateProfileString("Holding Registers", strNumberOfAO, "", str, sizeof(str), iniPath); 	
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
			printf("%d. AO address: %d name: %s min: %lf max: %lf pre: %lf unit: %s sw_mode: %d\n",i,AO[i].address,AO[i].name,AO[i].min,AO[i].max,AO[i].precision,AO[i].unit,AO[i].sw_mode);
		}
		#pragma endregion Find_INI
		fclose(fPtr);
		return true;
	}
    else {
        printf("INI Opened Failed...\n");
		return false;
    }


}


//Prepare to Upload Data
void UpDataPrepare(WISE_Data *Data, uint16_t *src)	//DI,DO already keep the latest data,so dont have to take care here.
{
	switch(Data->sw_mode)
	{
		case 1:
			Data->fv=custom_get_float(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->fv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 2:
			Data->fv=custom_get_float_dcba(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->fv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 3:
			Data->fv=custom_get_float_badc(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->fv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 4:
			Data->fv=custom_get_float_cdab(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->fv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 5:
			Data->uiv=custom_get_unsigned_int(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->uiv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 6:
			Data->uiv=custom_get_unsigned_int_cdab(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->uiv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 7:
			Data->iv=custom_get_int(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->iv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		case 8:
			Data->iv=custom_get_int_cdab(src);
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->iv,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
		default:
			Data->Regs=*src;
			IoT_SetDoubleValueWithMaxMin(Data->attr,Data->Regs,IoT_READWRITE,Data->max,Data->min,Data->unit);
			break;
	}

}
//Prepare to Download Data
int DownDataPrepare(WISE_Data *Data,float fv,uint32_t uiv,int iv, bool ret)
{
	int rc;
	uint16_t data[2] = {0};
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
				//printf("%f\n",f);
				custom_set_float(fv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 2:
				//printf("%f\n",f);
				custom_set_float_dcba(fv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 3:
				//printf("%f\n",f);
				custom_set_float_badc(fv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 4:
				//printf("%f\n",f);
				custom_set_float_cdab(fv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 5:
				custom_set_unsigned_int(uiv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 6:
				custom_set_unsigned_int_cdab(uiv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 7:
				custom_set_int(iv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			case 8:
				custom_set_int_cdab(iv,data);
				//printf("%d %d\n",AO_Set_Regs_temp_Ary[0],AO_Set_Regs_temp_Ary[1]);
				rc=modbus_write_register(ctx,Data->address,data[0]);
				rc=modbus_write_register(ctx,Data->address+1,data[1]);
				return rc;
				break;
			default:
				return modbus_write_register(ctx,Data->address,fv/Data->precision);
		}
	}
}

//-------------------------------------Customize_end
MSG_CLASSIFY_T * CreateCapability()
{
	MSG_CLASSIFY_T *myCapability = IoT_CreateRoot((char*) strPluginName);
	MSG_CLASSIFY_T *myGroup;
	MSG_ATTRIBUTE_T* attr;
	IoT_READWRITE_MODE mode=IoT_READONLY;

    char Client_Port[6];
	char Client_UnitID[6];
	char SlaveID[6];

	/*
	//-------------------------------------Customize
	char barcode[BARCODE_NUM]={""};
	//-------------------------------------Customize_end
	*/

	if(g_bFind)
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


			g_ConnectAttr = IoT_AddSensorNode(myGroup, "Connection");
			if(g_ConnectAttr)
			{
				IoT_SetBoolValue(g_ConnectAttr,true,mode);
			}	
		}

		
		if(numberOfDI!=0)
		{	
			myGroup = IoT_AddGroup(myCapability, "Discrete Inputs");
			if(myGroup)
			{
				mode=IoT_READONLY;
				for(int i=0;i<numberOfDI;i++){
					attr = IoT_AddSensorNode(myGroup, DI[i].name);
					if(attr)
					{
						IoT_SetBoolValue(attr,DI[i].Bits,mode);	
						//attr->pRev1 = &DI[i];
						DI[i].attr = attr;
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
				for(int i=0;i<numberOfDO;i++)
				{
					attr = IoT_AddSensorNode(myGroup, DO[i].name);
					if(attr)
					{
						IoT_SetBoolValue(attr,DO[i].Bits,mode);	
						//attr->pRev1 = &DO[i];
						DO[i].attr = attr;
					}
				}
			}
		}
		if(numberOfAI!=0)
		{	int AIRcur=0;
			myGroup = IoT_AddGroup(myCapability, "Input Registers");
			if(myGroup)
			{
				mode=IoT_READONLY;
				for(int i=0;i<numberOfAI;i++){
					attr = IoT_AddSensorNode(myGroup, AI[i].name);
					if(attr)
					{		
						if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4)
						{	
							IoT_SetDoubleValueWithMaxMin(attr,AI[i].fv,mode,AI[i].max,AI[i].min,AI[i].unit);
						}
						else
						{
							IoT_SetDoubleValueWithMaxMin(attr,AI[i].Regs*AI[i].precision,mode,AI[i].max,AI[i].min,AI[i].unit);
						}
						//attr->pRev1 = &AI[i];
						AI[i].attr = attr;
					}
				}
			}
		}

		if(numberOfAO!=0)
		{	int AORcur=0;
			myGroup = IoT_AddGroup(myCapability, "Holding Registers");
			if(myGroup)
			{
				mode=IoT_READWRITE;
				for(int i=0;i<numberOfAO;i++){
					attr = IoT_AddSensorNode(myGroup, AO[i].name);
					if(attr)
					{		
						IoT_SetDoubleValueWithMaxMin(attr,AO[i].fv,mode,AO[i].max,AO[i].min,AO[i].unit);
						//attr->pRev1 = &AO[i];
						AO[i].attr = attr;
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
		if(g_ConnectAttr)
		{
			IoT_SetBoolValue(g_ConnectAttr,false, IoT_READONLY);
		}	
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

		if(g_ConnectAttr)
		{
			IoT_SetBoolValue(g_ConnectAttr,true, IoT_READONLY);
		}	
		return true;
	}

}

//Disconnect a Modbus Connection
bool Modbus_Disconnect()
{
	modbus_close(ctx);
	ModbusLog(g_loghandle, Warning, "Modbus Disconnection!!");
	if(g_ConnectAttr)
	{
		IoT_SetBoolValue(g_ConnectAttr,false, IoT_READONLY);
	}	
	return false;
}

//Receive Data From Modbus
bool Modbus_Rev()
{

	int ret=-1;
	int i=0;
	bool bFCon=false;

	if(g_bRev_Fail==true)
		bFCon=true;

	g_bRev_Fail=false;

	//printf("Rev_Fail_Num : %d\n",Rev_Fail_Num);
	
	if(Rev_Fail_Num>5) //reconnect
	{
		
		Modbus_Disconnect();
		sleep(5);			//sleep 5 secs for avoiding lots of sockets created and not released yet
		if(!Modbus_Connect())
			if(!g_bIsSimtor)
			{
				ModbusLog(g_loghandle, Warning, "Modbus read fail!!");
				return false;	
			}
			else
				Rev_Fail_Num=0;
		else
			Rev_Fail_Num=0;
	}

	if(numberOfDI!=0)
	{	
		for(i=0;i<numberOfDI;i++)
		{	
			DI[i].bRevFin=false;	//not in use actually
			if(bConnectionFlag)
				ret = modbus_read_input_bits(ctx, DI[i].address, 1, &DI[i].Bits);
			else if(g_bIsSimtor)
			{
				ret = 0;
				DI[i].Bits=rand()%2;
			}
			else
				ret = -1;
			if (ret == -1) {
				ModbusLog(g_loghandle, Error, "modbus_read_input_bits failed - IB[%d] Rev ERROR : %s",i,modbus_strerror(errno));
				g_bRev_Fail=true;
			}
			else
			{
				DI[i].bRevFin=true;	//not in use actually
				IoT_SetBoolValue(DI[i].attr,DI[i].Bits,IoT_READONLY);	
				
			}
		}
	}

	if(numberOfDO!=0)
	{	
		for(i=0;i<numberOfDO;i++)
		{
			DO[i].bRevFin=false;	//not in use actually
			if(bConnectionFlag)
				ret = modbus_read_bits(ctx, DO[i].address, 1, &DO[i].Bits);
			else if(g_bIsSimtor)
			{
				ret = 0;
				DO[i].Bits=rand()%2;
			}
			else
				ret = -1;
			if (ret == -1) {
				ModbusLog(g_loghandle, Error, "modbus_read_bits failed - B[%d] Rev ERROR : %s",i,modbus_strerror(errno));
				g_bRev_Fail=true;
			}
			else
			{
				DO[i].bRevFin=true;	//not in use actually
				IoT_SetBoolValue(DO[i].attr,DO[i].Bits,IoT_READWRITE);	
			}
		}
	}

	if(numberOfAI!=0)
	{	
		int AIRRevcur=0;
		for(i=0;i<numberOfAI;i++)
		{	
			AI[i].bRevFin=false;
			if(AI[i].sw_mode==1||AI[i].sw_mode==2||AI[i].sw_mode==3||AI[i].sw_mode==4||AI[i].sw_mode==5||AI[i].sw_mode==6||AI[i].sw_mode==7||AI[i].sw_mode==8)
			{				
				uint16_t data[2] = {0};
				if(bConnectionFlag)
					ret = modbus_read_input_registers(ctx, AI[i].address, 2, (uint16_t*)&data);
				else if(g_bIsSimtor)
				{
					float fv = 0;
					ret = 0;
					if(AI[i].max>0)
						fv = AI[i].min/AI[i].precision + rand()%(int)(AI[i].max/AI[i].precision-AI[i].min/AI[i].precision + 1);
					switch(AI[i].sw_mode)
					{
						case 1:
							custom_set_float(fv,data);						
							break;
						case 2:
							custom_set_float_dcba(fv,data);
							break;
						case 3:
							custom_set_float_badc(fv,data);
							break;
						case 4:
							custom_set_float_cdab(fv,data);
							break;
						case 5:
							custom_set_unsigned_int((unsigned int)fv,data);
							break;
						case 6:
							custom_set_unsigned_int_cdab((unsigned int)fv,data);
							break;
						case 7:
							custom_set_int((int)fv,data);
							break;
						case 8:
							custom_set_int_cdab((int)fv,data);
							break;
					}
				}
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_input_registers failed - IR[%d] Rev ERROR : %s",i,modbus_strerror(errno));
					g_bRev_Fail=true;
				}
				else
				{
					UpDataPrepare(&AI[i], (uint16_t*)&data);
				}

				AIRRevcur+=2;
				AI[i].bRevFin=true;
			}
			else
			{
				uint16_t data = 0;
				if(bConnectionFlag)
					ret = modbus_read_input_registers(ctx, AI[i].address, 1,&data);
				else if(g_bIsSimtor)
				{
					ret = 0;
					if(AI[i].max>0)
						data = (uint16_t)(AI[i].min/AI[i].precision + rand()%(int)(AI[i].max/AI[i].precision-AI[i].min/AI[i].precision + 1));
				}
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_input_registers failed - IR[%d] Rev ERROR : %s",i,modbus_strerror(errno));
					g_bRev_Fail=true;
				}
				else	
				{
					UpDataPrepare(&AI[i], &data);
				}
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
			AO[i].bRevFin=false;
			if(AO[i].sw_mode==1||AO[i].sw_mode==2||AO[i].sw_mode==3||AO[i].sw_mode==4||AO[i].sw_mode==5||AO[i].sw_mode==6||AO[i].sw_mode==7||AO[i].sw_mode==8)
			{	
				uint16_t data[2] = {0};
				if(bConnectionFlag)
					ret = modbus_read_registers(ctx, AO[i].address, 2, (uint16_t*)&data);
				else if(g_bIsSimtor)
				{
					float fv = 0;
					ret = 0;
					if(AO[i].max>0)
						fv = AO[i].min/AO[i].precision + rand()%(int)(AO[i].max/AO[i].precision-AO[i].min/AO[i].precision + 1);
					switch(AO[i].sw_mode)
					{
						case 1:
							custom_set_float(fv,data);						
							break;
						case 2:
							custom_set_float_dcba(fv,data);
							break;
						case 3:
							custom_set_float_badc(fv,data);
							break;
						case 4:
							custom_set_float_cdab(fv,data);
							break;
						case 5:
							custom_set_unsigned_int((unsigned int)fv,data);
							break;
						case 6:
							custom_set_unsigned_int_cdab((unsigned int)fv,data);
							break;
						case 7:
							custom_set_int((int)fv,data);
							break;
						case 8:
							custom_set_int_cdab((int)fv,data);
							break;
					}
				}
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_registers failed - R[%d] Rev ERROR : %s",i,modbus_strerror(errno));
					g_bRev_Fail=true;
				}
				else
				{
					UpDataPrepare(&AO[i], (uint16_t*)&data);
				}
				AORRevcur+=2;
				AO[i].bRevFin=true;
			}
			else
			{
				uint16_t data = 0;
				if(bConnectionFlag)
					ret = modbus_read_registers(ctx, AO[i].address, 1, &data);
				else if(g_bIsSimtor)
				{
					ret = 0;
					if(AO[i].max>0)
						data = (uint16_t)(AO[i].min/AO[i].precision + rand()%(int)(AO[i].max/AO[i].precision-AO[i].min/AO[i].precision + 1));
				}
				else
					ret = -1;
				if (ret == -1) {
					ModbusLog(g_loghandle, Error, "modbus_read_registers failed - R[%d] Rev ERROR : %s",i,modbus_strerror(errno));
					g_bRev_Fail=true;
				}
				else
				{
					UpDataPrepare(&AO[i], &data);					
				}
				AORRevcur++;
				AO[i].bRevFin=true;
			}
		}
	}
		
	if(bFCon && g_bRev_Fail)
		Rev_Fail_Num++;
	return true;
}

//--------------------------------------------------------------------------------------------------------------
//------------------------------------------------Threads-------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//Modbus Retrieve Thread
static void* RetrieveThreadStart(void *args)
{

	handler_context_t *pHandlerContex = (handler_context_t *)args;

	#pragma region Modbus_Retrieve	
	while(pHandlerContex->isThreadRunning)
	{  // printf("Retrieve.......\n");
		if(g_bIsSimtor)
			g_bRetrieve=Modbus_Rev();
		else if(ctx!=NULL)
			g_bRetrieve=Modbus_Rev();
		usleep(500*1000);	//To fix yocto
	}
	#pragma endregion Modbus_Retrieve

	return 0;

}

WISE_Data* FindWISEData(MSG_ATTRIBUTE_T* attr)
{
	WISE_Data* target = NULL;
	if(target == NULL && numberOfDI > 0)
	{
		int i=0;
		for(i=0; i<numberOfDI; i++)
		{
			if(DI[i].attr == attr)
			{
				target = &DI[i];
				break;
			}
		}
	}

	if(target == NULL && numberOfDO > 0)
	{
		int i=0;
		for(i=0; i<numberOfDO; i++)
		{
			if(DO[i].attr == attr)
			{
				target = &DO[i];
				break;
			}
		}
	}

	if(target == NULL && numberOfAI > 0)
	{
		int i=0;
		for(i=0; i<numberOfAI; i++)
		{
			if(AI[i].attr == attr)
			{
				target = &AI[i];
				break;
			}
		}
	}

	if(target == NULL && numberOfAO > 0)
	{
		int i=0;
		for(i=0; i<numberOfAO; i++)
		{
			if(AO[i].attr == attr)
			{
				target = &AO[i];
				break;
			}
		}
	}
	return target;
}

/*callback function to handle set sensor data event*/
bool on_set_sensor(set_data_t* objlist, void *pRev)
{
	set_data_t *current = objlist;
	if(objlist == NULL) return false;
	while(current)
	{
		MSG_ATTRIBUTE_T* attr = IoT_FindSensorNodeWithPath(g_Capability, current->sensorname);
		current->errcode = STATUSCODE_NOT_FOUND;
		strcpy(current->errstring, STATUS_NOT_FOUND);
		if(attr != NULL)
		{
			WISE_Data* data = FindWISEData(attr);
			if(data->ad_mode == 0)
			{
				current->errcode = STATUSCODE_READ;
				strcpy(current->errstring, STATUS_READ);
			}
			else if(data->ad_mode == 1)
			{
				if(current->newtype == attr_type_boolean)
				{
					int ret=modbus_write_bit(ctx, data->address, current->bv);
					if(ret != -1)
					{
						current->errcode = STATUSCODE_SUCCESS;
						strcpy(current->errstring, STATUS_SUCCESS);
					}
					else
					{
						current->errcode = STATUSCODE_FAIL;
						strcpy(current->errstring, STATUS_FAIL);
					}
				}
				else if(current->newtype == attr_type_numeric)
				{
					if(current->v == 0)
					{
						int ret=modbus_write_bit(ctx, data->address, 0);
						if(ret != -1)
						{
							current->errcode = STATUSCODE_SUCCESS;
							strcpy(current->errstring, STATUS_SUCCESS);
						}
						else
						{
							current->errcode = STATUSCODE_FAIL;
							strcpy(current->errstring, STATUS_FAIL);
						}
					}
					else if(current->v == 1)
					{
						int ret=modbus_write_bit(ctx, data->address, 1);
						if(ret != -1)
						{
							current->errcode = STATUSCODE_SUCCESS;
							strcpy(current->errstring, STATUS_SUCCESS);
						}
						else
						{
							current->errcode = STATUSCODE_FAIL;
							strcpy(current->errstring, STATUS_FAIL);
						}
					}
					else
					{
						current->errcode = STATUSCODE_FORMAT_ERROR;
						strcpy(current->errstring, STATUS_FORMAT_ERROR);
					}
				}
				else
				{
					current->errcode = STATUSCODE_FORMAT_ERROR;
					strcpy(current->errstring, STATUS_FORMAT_ERROR);
				}

			}
			else if(data->ad_mode ==2)
			{
				current->errcode = STATUSCODE_READ;
				strcpy(current->errstring, STATUS_READ);
			}
			else
			{
				if(current->newtype == attr_type_numeric)
				{
					int ret=DownDataPrepare(data, current->v,current->v,current->v,true); 
					if(ret != -1)
					{
						current->errcode = STATUSCODE_SUCCESS;
						strcpy(current->errstring, STATUS_SUCCESS);
					}
					else
					{
						current->errcode = STATUSCODE_FAIL;
						strcpy(current->errstring, STATUS_FAIL);
					}
				}
				else
				{
					current->errcode = STATUSCODE_FORMAT_ERROR;
					strcpy(current->errstring, STATUS_FORMAT_ERROR);
				}
			}
		}
		current = current->next;
	}

	return true;
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
	//snprintf( pluginfo->Name, sizeof(pluginfo->Name), "%s", strPluginName );
	strPluginName=(char *)calloc(strlen(pluginfo->Name)+1,sizeof(char));	
	strcpy(strPluginName,pluginfo->Name);

	pluginfo->RequestID = iRequestID;
	pluginfo->ActionID = iActionID;
	printf(" >Name: %s\r\n", strPluginName);
	// 2. Copy agent info 
	memcpy(&g_HandlerInfo, pluginfo, sizeof(HANDLER_INFO));
	HandlerKernel_Initialize(pluginfo);
	g_sendcbf = pluginfo->sendcbf;

	g_RetrieveContex.threadHandler = NULL;
	g_RetrieveContex.isThreadRunning = false;
	
	g_bFind = read_INI();

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

	if(g_RetrieveContex.isThreadRunning == true)
	{
		g_RetrieveContex.isThreadRunning = false;
		pthread_cancel((pthread_t)g_RetrieveContex.threadHandler);
		pthread_join((pthread_t)g_RetrieveContex.threadHandler, NULL);
		g_RetrieveContex.threadHandler = NULL;
	}
	
	if(DI)
		free(DI);
	DI = NULL;
	if(DO)
		free(DO);
	DO = NULL;
	if(AI)
		free(AI);
	AI = NULL;
	if(AO)
		free(AO);
	AO = NULL;
	if(modbus_get_socket(ctx)>0)
		Modbus_Disconnect();			
	modbus_free(ctx);
	ctx = NULL;

	HandlerKernel_Uninitialize();
	if(strPluginName)
		free(strPluginName);
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
		memcpy(&g_HandlerInfo, pluginfo, sizeof(HANDLER_INFO));
	else
	{
		memset(&g_HandlerInfo, 0, sizeof(HANDLER_INFO));
		snprintf( g_HandlerInfo.Name, sizeof( g_HandlerInfo.Name), "%s", strPluginName );
		g_HandlerInfo.RequestID = iRequestID;
		g_HandlerInfo.ActionID = iActionID;
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

	g_RetrieveContex.isThreadRunning = true;
	
	if(!g_bIsSimtor)
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

	if(pthread_create(&g_RetrieveContex.threadHandler,NULL, RetrieveThreadStart, &g_RetrieveContex) != 0)
	{
		g_RetrieveContex.isThreadRunning = false;
		printf("> start Retrieve thread failed!\r\n");	
		return handler_fail;
    }

	g_status = handler_status_start;

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
	//printf("Handler Stop..................\n");
	
	if(g_RetrieveContex.isThreadRunning == true)
	{
		g_RetrieveContex.isThreadRunning = false;
		pthread_join((pthread_t)g_RetrieveContex.threadHandler, NULL);
		g_RetrieveContex.threadHandler = NULL;
	}

	if(DI)
		free(DI);
	DI = NULL;
	if(DO)
		free(DO);
	DO = NULL;
	if(AI)
		free(AI);
	AI = NULL;
	if(AO)
		free(AO);
	AO = NULL;
	if(modbus_get_socket(ctx)>0)
		Modbus_Disconnect();			
	modbus_free(ctx);
	ctx = NULL;
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
	char sessionID[33] = {0};
	ModbusLog(g_loghandle, Normal, " %s>Recv Topic [%s] Data %s", strPluginName, topic, (char*) data );
	printf(" >Recv Topic [%s] Data %s", topic, (char*) data );

	if(HandlerKernel_ParseRecvCMDWithSessionID((char*)data, &cmdID, sessionID) != handler_success)
	{
		char repMsg[32] = {0};
		int len = 0;
		sprintf( repMsg, "{\"errorRep\":\"Command parse error!\"}" );
		len= strlen(repMsg) ;
		if ( g_sendcbf ) g_sendcbf( & g_HandlerInfo, hk_error_rep, repMsg, len, NULL, NULL );
		return;
	}

	switch(cmdID)
	{
		case hk_get_capability_req:
		{
			//printf("\n----------------------------------------\n");
			//printf("\n-------modbus_get_capability_req--------\n");
			//printf("\n----------------------------------------\n");
			//GetCapability();
			if(!g_Capability)
			{
				g_Capability = CreateCapability();
				HandlerKernel_SetCapability(g_Capability, true);
			}
			break;
		}

		case hk_auto_upload_req:
		/*start live report*/
		HandlerKernel_LiveReportStart(hk_auto_upload_rep, (char*)data);
		break;
	case hk_set_thr_req:
		/*Stop threshold check thread*/
		HandlerKernel_StopThresholdCheck();
		/*setup threshold rule*/
		HandlerKernel_SetThreshold(hk_set_thr_rep,(char*) data);
		/*register the threshold check callback function to handle trigger event*/
		HandlerKernel_SetThresholdTrigger(NULL);
		/*Restart threshold check thread*/
		HandlerKernel_StartThresholdCheck();
		break;
	case hk_del_thr_req:
		/*Stop threshold check thread*/
		HandlerKernel_StopThresholdCheck();
		/*clear threshold check callback function*/
		HandlerKernel_SetThresholdTrigger(NULL);
		/*Delete all threshold rules*/
		HandlerKernel_DeleteAllThreshold(hk_del_thr_rep);
		break;
	case hk_get_sensors_data_req:
		/*Get Sensor Data with callback function*/
		HandlerKernel_GetSensorData(hk_get_sensors_data_rep, sessionID, (char*)data, NULL);
		break;
	case hk_set_sensors_data_req:
		/*Set Sensor Data with callback function*/
		HandlerKernel_SetSensorData(hk_set_sensors_data_rep, sessionID, (char*)data, on_set_sensor);
		break;
	default:
		{
			/* Send command not support reply message*/
			char repMsg[32] = {0};
			int len = 0;
			sprintf( repMsg, "{\"errorRep\":\"Unknown cmd!\"}" );
			len= strlen( repMsg ) ;
			if ( g_sendcbf ) g_sendcbf( & g_HandlerInfo, hk_error_rep, repMsg, len, NULL, NULL );
		}
		break;

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

	HandlerKernel_AutoReportStart(pInQuery);
	
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
	HandlerKernel_AutoReportStop(pInQuery);

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
	
	if(!g_Capability)
	{
		g_Capability = CreateCapability();
		HandlerKernel_SetCapability(g_Capability, false);
	}
	
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

/* **************************************************************************************
 *  Function Name: Handler_MemoryFree
 *  Description: free the memory allocated for Handler_Get_Capability
 *  Input : char *pInData.
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_MemoryFree(char *pInData)
{
	if(pInData)
	{
		free(pInData);
		pInData = NULL;
	}
	return;
}

