#ifndef _CORE_PARSER_H_
#define _CORE_PARSER_H_

#include "SAClient.h"

#define PJSON   void*

#ifdef __cplusplus
extern "C" {
#endif

char *scparser_utf8toansi(const char* str);

char * scparser_ansitoutf8(char* wText);

char *scparser_print(PJSON item);

char *scparser_unformatted_print(PJSON item);

void scparser_free(PJSON ptr);

PJSON scparser_agentinfo_create(susiaccess_agent_profile_body_t const * pProfile, int status);

char* scparser_agentinfo_print(susiaccess_agent_profile_body_t const * pProfile, int status);

PJSON scparser_osinfo_create(susiaccess_agent_profile_body_t const * pProfile);

char* scparser_osinfo_print(susiaccess_agent_profile_body_t const * pProfile);

PJSON scparser_packet_create(susiaccess_packet_body_t const * pPacket);

char* scparser_packet_print(susiaccess_packet_body_t const * pPacket);

int scparser_message_parse(void* data, int datalen, susiaccess_packet_body_t * pkt);

#ifdef __cplusplus
}
#endif

#endif