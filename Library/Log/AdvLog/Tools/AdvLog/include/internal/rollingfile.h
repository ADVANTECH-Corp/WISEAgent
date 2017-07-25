/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/07/07 by Fred Chang									*/
/* Modified Date: 2015/07/07 by Fred Chang									*/
/* Abstract     : Advantech Logging Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ROLLING_FILE_H__
#define __ROLLING_FILE_H__

void RollFile_Open(const char *path);
void RollFile_SetLimit(int byte);
FILE *RollFile_Check();
void RemoveOverFiles();
void RollFile_StreamIn(const char *stream, int length);
void RollFile_Flush(int length);
void RollFile_RefreshConfigure();
void RollFile_Flush();
void RollFile_Close();


#endif //__ROLLING_FILE_H__


