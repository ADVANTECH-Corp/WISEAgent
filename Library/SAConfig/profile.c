#include "network.h"
#include "XMLBase.h"
#include "profile.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util_path.h"

#define XML_CONFIG_ROOT "XMLConfigSettings"
#define XML_CONFIG_BASIC "Profiles"

#define DEVICE_NAME_KEY			"DeviceName"
#define SW_VERSION_KEY			"SWVersion"
#define DEV_ID_KEY				"DevID"
#define DEV_TYPE_KEY			"DevType"
#define DEV_PRODUCT_KEY			"Product"
#define DEV_MANUFACTURE_KEY		"Manufacture"
#define DEV_SN_KEY				"SN"
#define DEV_LAL_KEY				"Lal"
#define DEV_WORKDIR_KEY			"WorkDir"
#define USER_NAME_KEY			"UserName"
#define USER_PASSWORD_KEY		"UserPassword"

int profile_first_mac_get(char* mac)
{
	int iRet = -1;
	if(NULL == mac) return iRet;
	{
		char macsStr[DEF_MAC_LENGTH] = {0};
		int count = 600;
		do{
			if(count <=0) return iRet;
			count--;
			iRet = network_mac_get_ex(macsStr);
			usleep(100*1000);
		}while(iRet != 0);
		if(iRet == 0)
		{
			strcpy(mac, macsStr);
		}
	}
	return iRet;
}

bool SACONFIG_API profile_load(char const * configFile, susiaccess_agent_profile_body_t * profile)
{
	bool bRet = false;
	xml_doc_info *  doc = NULL;
	bool bModify = false;
	char mac[DEF_MAC_LENGTH] = {0};
	if(configFile == NULL) 
		return bRet;
	if(profile == NULL)
		return bRet;
	
	doc = xml_Loadfile(configFile, XML_CONFIG_ROOT, XML_CONFIG_BASIC);
	if(doc == NULL)
		return bRet;

	memset(profile, 0, sizeof(susiaccess_agent_profile_body_t));

	if(!xml_GetItemValue(doc, DEVICE_NAME_KEY, profile->hostname, sizeof(profile->hostname)))
	{
		memset(profile->hostname, 0, sizeof(profile->hostname));
	}
	if(strlen(profile->hostname)<=0)
	{
		network_host_name_get(profile->hostname, sizeof(profile->hostname));
		xml_SetItemValue(doc,DEVICE_NAME_KEY, profile->hostname);
		bModify = true;
	}

	if(!xml_GetItemValue(doc, SW_VERSION_KEY, profile->version, sizeof(profile->version)))
		memset(profile->version, 0, sizeof(profile->version));

	if(profile_first_mac_get(mac)==0)
	{
		strcpy(profile->mac, mac);
	}

	if(!xml_GetItemValue(doc, DEV_ID_KEY, profile->devId, sizeof(profile->devId)))
	{
		memset(profile->devId, 0, sizeof(profile->devId));
	}
	if(strlen(profile->devId)<=0)
	{
		if(strlen(mac)>0)
		{
			sprintf(profile->devId, "0000%s", mac);
			xml_SetItemValue(doc,DEV_ID_KEY, profile->devId);
			bModify = true;
		}
	}

	if(!xml_GetItemValue(doc, DEV_SN_KEY, profile->sn, sizeof(profile->sn)))
	{
		memset(profile->sn, 0, sizeof(profile->sn));
	}
	if(strlen(profile->sn)<=0)
	{
		if(strlen(mac)>0)
		{
			strncpy(profile->sn, mac, strlen(profile->sn));
			xml_SetItemValue(doc,DEV_SN_KEY, profile->sn);
			bModify = true;
		}
	}

	if(!xml_GetItemValue(doc, DEV_TYPE_KEY, profile->type, sizeof(profile->type)))
	{
		memset(profile->type, 0, sizeof(profile->type));
	}
	if(strlen(profile->type)<=0)
	{
		strncpy(profile->type, "IPC", strlen("IPC")+1);
		xml_SetItemValue(doc, DEV_TYPE_KEY, profile->type);
		bModify = true;
	}

	if(!xml_GetItemValue(doc, DEV_PRODUCT_KEY, profile->product, sizeof(profile->product)))
		memset(profile->product, 0, sizeof(profile->product));
	if(!xml_GetItemValue(doc, DEV_MANUFACTURE_KEY, profile->manufacture, sizeof(profile->manufacture)))
		memset(profile->manufacture, 0, sizeof(profile->manufacture));

	if(!xml_GetItemValue(doc, DEV_LAL_KEY, profile->lal, sizeof(profile->lal)))
		memset(profile->lal, 0, sizeof(profile->lal));

	if(!xml_GetItemValue(doc, DEV_WORKDIR_KEY, profile->workdir, sizeof(profile->workdir)))
	{
		memset(profile->workdir, 0, sizeof(profile->workdir));
	}
	if(strlen(profile->workdir)<=0)
	{
		util_module_path_get(profile->workdir);
		xml_SetItemValue(doc, DEV_WORKDIR_KEY, profile->workdir);
		bModify = true;
	}

	if(!xml_GetItemValue(doc, USER_NAME_KEY, profile->account, sizeof(profile->account)))
	{
		memset(profile->account, 0, sizeof(profile->account));
	}
	if(strlen(profile->account)<=0)
	{
		strncpy(profile->account, "anonymous", strlen("anonymous")+1);
		xml_SetItemValue(doc,USER_NAME_KEY, profile->account);
		bModify = true;
	}

	if(!xml_GetItemValue(doc, USER_PASSWORD_KEY, profile->passwd, sizeof(profile->passwd)))
	{
		memset(profile->passwd, 0, sizeof(profile->passwd));
	}
	/*if(strlen(profile->passwd)<=0)
	{
		memset(profile->passwd, 0, sizeof(profile->passwd));
		xml_SetItemValue(doc,USER_PASSWORD_KEY, profile->passwd);
		bModify = true;
	}*/

	if(bModify)
	{
		xml_SaveFile(configFile, doc);
	}
	bRet = true;
	xml_FreeDoc(doc);
	return bRet;
}

bool SACONFIG_API profile_save(char const * configFile, susiaccess_agent_profile_body_t const * const profile)
{
	bool bRet = false;
	xml_doc_info *  doc = NULL;

	if(configFile == NULL) 
		return bRet;
	if(profile == NULL)
		return bRet;
	
	doc = xml_Loadfile(configFile, XML_CONFIG_ROOT, XML_CONFIG_BASIC);
	if(doc == NULL)
	{
		doc = xml_CreateDoc( XML_CONFIG_ROOT, XML_CONFIG_BASIC);
	}

	if(xml_SetItemValue(doc,DEVICE_NAME_KEY, profile->hostname)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc,SW_VERSION_KEY, profile->version)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc,DEV_ID_KEY, profile->devId)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc,DEV_SN_KEY, profile->sn)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc, DEV_TYPE_KEY, profile->type)<0)
		goto SAVE_EXIT;
		
	if(xml_SetItemValue(doc, DEV_PRODUCT_KEY, profile->product)<0)
		goto SAVE_EXIT;
		
	if(xml_SetItemValue(doc, DEV_MANUFACTURE_KEY, profile->manufacture)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc, DEV_LAL_KEY, profile->lal)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc, DEV_WORKDIR_KEY, profile->workdir)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc,USER_NAME_KEY, profile->account)<0)
		goto SAVE_EXIT;

	if(xml_SetItemValue(doc,USER_PASSWORD_KEY, profile->passwd)<0)
		goto SAVE_EXIT;

	xml_SaveFile(configFile, doc);
	bRet = true;
SAVE_EXIT:
	xml_FreeDoc(doc);
	return bRet;
}

bool SACONFIG_API profile_create(char const * configFile, susiaccess_agent_profile_body_t * profile)
{
	bool bRet = false;
	xml_doc_info *  doc = NULL;
	char sn[DEF_MAX_STRING_LENGTH] = {0};

	if(configFile == NULL) 
		return bRet;
	if(profile == NULL)
		return bRet;

	memset(profile, 0, sizeof(susiaccess_agent_profile_body_t));

	doc = xml_Loadfile(configFile, XML_CONFIG_ROOT, XML_CONFIG_BASIC);
	if(doc == NULL)
	{
		doc = xml_CreateDoc( XML_CONFIG_ROOT, XML_CONFIG_BASIC);

		network_host_name_get(profile->hostname, sizeof(profile->hostname));
		if(xml_SetItemValue(doc,DEVICE_NAME_KEY, profile->hostname)<0)
			goto CREATE_EXIT;

		strncpy(profile->version, "1.0.0.0", strlen("1.0.0.0")+1);
		if(xml_SetItemValue(doc,SW_VERSION_KEY, profile->version)<0)
			goto CREATE_EXIT;

		if(profile_first_mac_get(sn)==0)
		{
			strncpy(profile->sn, sn, strlen(sn)+1);
			sprintf(profile->devId, "0000%s", sn);
		} else {
			memset(profile->sn, 0, sizeof(profile->sn));
			memset(profile->devId, 0, sizeof(profile->devId));
		}
		if(xml_SetItemValue(doc,DEV_ID_KEY, profile->devId)<0)
			goto CREATE_EXIT;

		if(xml_SetItemValue(doc,DEV_SN_KEY, profile->sn)<0)
			goto CREATE_EXIT;

		strncpy(profile->type, "IPC", strlen("IPC")+1);
		if(xml_SetItemValue(doc, DEV_TYPE_KEY, profile->type)<0)
			goto CREATE_EXIT;
		
		memset(profile->product, 0, sizeof(profile->product));
		if(xml_SetItemValue(doc, DEV_PRODUCT_KEY, profile->product)<0)
			goto CREATE_EXIT;
		
		memset(profile->manufacture, 0, sizeof(profile->manufacture));
		if(xml_SetItemValue(doc, DEV_MANUFACTURE_KEY, profile->manufacture)<0)
			goto CREATE_EXIT;

		util_module_path_get(profile->workdir);
		if(xml_SetItemValue(doc, DEV_WORKDIR_KEY, profile->workdir)<0)
			goto CREATE_EXIT;

		strncpy(profile->account, "anonymous", strlen("anonymous")+1);
		if(xml_SetItemValue(doc,USER_NAME_KEY, profile->account)<0)
			goto CREATE_EXIT;

		memset(profile->passwd, 0, sizeof(profile->passwd));
		if(xml_SetItemValue(doc,USER_PASSWORD_KEY, profile->passwd)<0)
			goto CREATE_EXIT;

		xml_SaveFile(configFile, doc);
		bRet = true;
CREATE_EXIT:
		xml_FreeDoc(doc);
		return bRet;
	}
	else
	{
		xml_FreeDoc(doc);
		return profile_load(configFile, profile);
	}
}

bool SACONFIG_API profile_get(char const * const configFile, char const * const itemName, char * itemValue, int valueLen)
{
	bool bRet = false;
	if(NULL == configFile || NULL == itemName || NULL == itemValue) return bRet;
	{
		xml_doc_info *  doc = NULL;
		doc = xml_Loadfile(configFile, XML_CONFIG_ROOT, XML_CONFIG_BASIC);
		if(doc)
		{
			bRet = xml_GetItemValue(doc, itemName, itemValue, valueLen);
			xml_FreeDoc(doc);
		}
	}
	return bRet;
}

bool SACONFIG_API profile_set(char const * const configFile, char const * const itemName, char const * const itemValue)
{
	int bRet = false;
   if(NULL == configFile || NULL == itemName || NULL == itemValue) return bRet;
	{
		xml_doc_info *  doc = NULL;
		doc = xml_Loadfile(configFile, XML_CONFIG_ROOT, XML_CONFIG_BASIC);
		if(doc)
		{
			if(xml_SetItemValue(doc, itemName, itemValue)<0)
				return bRet;

			xml_SaveFile(configFile, doc);
			xml_FreeDoc(doc);
			bRet = true;
		}
	}
	return bRet;
}