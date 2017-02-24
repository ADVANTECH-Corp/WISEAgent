/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_TYPE_H__
#define __JSON_TYPE_H__

#ifndef JSONODE_DEFINED
#define JSONODE_DEFINED
typedef struct __JSONode JSONode;
#endif

struct __JSONode {
	int type;
	int alloc;
	char *s;
	int len;
	void *collection;
	JSONode *key;
	JSONode *value;
	
	JSONode *array;
	JSONode *next;
	JSONode *prev;
	JSONode *root;
	
	void *manager;
};


#define JSON_TYPE_INVAILD 	0
#define JSON_TYPE_STRING 	1
#define JSON_TYPE_NUMBER	2
#define JSON_TYPE_OBJECT	4
#define JSON_TYPE_ARRAY		8
#define JSON_TYPE_VALUE		16
#define JSON_TYPE_PAIR		32
#define JSON_TYPE_BOOL		64
#define JSON_TYPE_ELEMENT	128
#define JSON_TYPE_VOID		256

#endif //__JSON_TYPE_H__
