#ifndef _CAGENT_CONFIGURATION_H_
#define _CAGENT_CONFIGURATION_H_
#include "susiaccess_def.h"
#include <stdbool.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
	#include <windows.h>
	#ifndef SACONFIG_API
		#define SACONFIG_API WINAPI
	#endif
#else
	#define SACONFIG_API
#endif

#define DEF_CONFIG_FILE_NAME	"agent_config.xml"

#define DEF_REMOTE_CONNECT_RUN_MODE                 "RemoteConnect"
#define DEF_STANDALONE_RUN_MODE                     "Standalone"

#ifdef __cplusplus
extern "C" {
#endif

bool SACONFIG_API cfg_load(char const * configFile, susiaccess_agent_conf_body_t * conf);
bool SACONFIG_API cfg_save(char const * configFile, susiaccess_agent_conf_body_t const * const conf);
bool SACONFIG_API cfg_create(char const * configFile, susiaccess_agent_conf_body_t * conf);
bool SACONFIG_API cfg_get(char const * const configFile, char const * const itemName, char * itemValue, int valueLen);
bool SACONFIG_API cfg_set(char const * const configFile, char const * const itemName, char const * const itemValue);

#ifdef __cplusplus
}
#endif

#endif