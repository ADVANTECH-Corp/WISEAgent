/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_TOOL_H__
#define __JSON_TOOL_H__

#ifdef __cplusplus
extern "C" {
#endif
char *skip(char *in);
void doubleQuote(char *target, const char *source, int *len);
char *jmalloc(char *target, int *alloc, int len);
#ifdef __cplusplus
}
#endif
#endif //__JSON_TOOL_H__