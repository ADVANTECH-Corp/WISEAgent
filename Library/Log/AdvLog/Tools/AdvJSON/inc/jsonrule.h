/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_RULE_H__
#define __JSON_RULE_H__

int IsString(const char *json);
int IsNumber(const char *json);
int IsValue(const char *json);
int IsObject(const char *json);
int IsArray(const char *json);

#endif //__JSON_RULE_H__