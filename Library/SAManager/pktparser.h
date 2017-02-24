#ifndef _SA_PARSER_H_
#define _SA_PARSER_H_

#include "susiaccess_def.h"

#define PJSON   void*

#ifdef __cplusplus
extern "C" {
#endif

char *pkg_parser_print(PJSON item);

char *pkg_parser_print_unformatted(PJSON item);

void pkg_parser_free(PJSON ptr);

PJSON pkg_parser_packet_create(susiaccess_packet_body_t const * pPacket);

char* pkg_parser_packet_print(susiaccess_packet_body_t * pkt);

int pkg_parser_recv_message_parse(void* data, int datalen, susiaccess_packet_body_t * pkt);

#ifdef __cplusplus
}
#endif

#endif