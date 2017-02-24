/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/07/07 by Fred Chang									*/
/* Modified Date: 2015/07/07 by Fred Chang									*/
/* Abstract     : Advantech Logging Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

int AdvLog_Configure_Init(int pid);
int AdvLog_Configure_OptParser(int argc, char **argv, int pid);

int AdvLog_Configure_GetPid();
int AdvLog_Configure_GetStaticLevel();
int AdvLog_Configure_GetStaticGray();
int AdvLog_Configure_GetStaticInfo();
int AdvLog_Configure_GetDynamicLevel();
int AdvLog_Configure_GetDynamicGray();
int AdvLog_Configure_GetDynamicInfo();
const char *AdvLog_Configure_GetPath();
int AdvLog_Configure_GetLimit();
int AdvLog_Configure_Hide_Enable();
int AdvLog_Configure_Is_Hiden(int level);

void AdvLog_Configure_Uninit();

#endif //__CONFIGURE_H__