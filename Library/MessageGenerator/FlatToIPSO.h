#ifndef _FLAT_IPSO_TRANSFER_H_
#define _FLAT_IPSO_TRANSFER_H_
#include <stdbool.h>
#include "MsgGenerator.h"
#ifdef __cplusplus
extern "C" {
#endif

bool transfer_parse_json(const char* data, MSG_CLASSIFY_T *pGroup);

#ifdef __cplusplus
}
#endif
#endif