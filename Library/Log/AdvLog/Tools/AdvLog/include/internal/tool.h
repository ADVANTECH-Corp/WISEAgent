/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/07/07 by Fred Chang									*/
/* Modified Date: 2015/07/07 by Fred Chang									*/
/* Abstract     : Advantech Logging Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __TOOL_H__
#define __TOOL_H__

#define ABS(x) (x < 0 ? -x : x)

void GetNowTimeString(char *string, int length, const char *fmt);
//void ReduceBackGroundAndDumpString(const char *buffer, char **output);
int GetFileSize(FILE *fp);
char *fmalloc(const char *filename);
#endif //__TOOL_H__