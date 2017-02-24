/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADVANTECH_JSON_H__
#define __ADVANTECH_JSON_H__

#if defined(WIN32)
	#pragma once
	#include <AdvPlatform.h>
	#ifdef ADVJSON_EXPORTS
		#define ADVJSON_CALL __stdcall
		#define ADVJSON_EXPORT __declspec(dllexport)
	#else
		#define ADVJSON_CALL __stdcall
		#define ADVJSON_EXPORT
	#endif
#else
	#define ADVJSON_CALL
	#define ADVJSON_EXPORT
#endif

#ifndef JSONODE_DEFINED
#define JSONODE_DEFINED
typedef struct __JSONode JSONode;
#endif

#ifdef __cplusplus

#include <string>
//using namespace std;

class AdvVariousType;
class AdvJSONParallel;
class AdvJSONCreator;
class AdvJSON;
class ADVJSON_EXPORT AdvJSON {
public:
	friend class AdvJSONParallel;
	friend class AdvJSONCreator;
	friend class AdvJSONCreatorParallel;
	
private:
	AdvJSON& operator()(JSONode *jnode);
	static void GetInitialArray(AdvVariousType *avt, JSONode *jn, int mode = MODE_NEW);
	static JSONode* NewArray(JSONode*node, int mode, int index);
	
public:
	AdvJSON& operator()(const char *str);
	AdvJSON& operator()(double number);
	AdvJSON& operator()(bool boolean);
	
public:
	void Init();
	void Release();
	bool IsNULL();
	
public:
	static int Validator(const char *json);

public:
	AdvJSON();
	AdvJSON(const char *json);
	~AdvJSON();

	AdvJSON(AdvJSON *nJson);
	AdvJSON(const AdvJSON &rJson);
	
public:
	bool operator==(void *ptr);
	bool operator==(AdvJSON other);
	
	AdvJSON operator[](const char *name);
	AdvJSON operator[](int index);
	
	
	AdvJSON &operator=(short number)	{	return (*this)((double)number);	}
	AdvJSON &operator=(int number)		{	return (*this)((double)number);	}
	AdvJSON &operator=(long number)		{	return (*this)((double)number);	}
	AdvJSON &operator=(float number)	{	return (*this)((double)number);	}
	AdvJSON &operator=(double number)	{	return (*this)((double)number);	}
	
	AdvJSON &operator=(const char *str) {	return (*this)(str);			}
	AdvJSON &operator=(std::string str) {	return (*this)(str.c_str());	}
	
	AdvJSON &operator=(bool boolean)	{	return (*this)(boolean);		}
	AdvJSON &operator=(AdvJSON other) 	{ Init(); node = other.node; mode = other.mode; return *this;}
	
	AdvJSON &operator=(JSONode *other);
	
#if __cplusplus > 199711L
	AdvJSON &operator=(std::initializer_list<AdvVariousType> list);
	AdvJSON &operator+=(std::initializer_list<AdvVariousType> list);
	AdvJSONParallel &operator[](std::initializer_list<const char *> list);
#endif

	AdvJSON &operator<<(const char *str);
	
	std::string Key();
	int Size();
	std::string Value();
	std::string String();
	int Int();
	float Float();
	double Double();
	bool Bool();
	
	void Print(char *buffer, int size, int depth = -1);
	void Print();
	void Print(int depth);
	void PrintLink(int depth);
	void Conf() {
		printf("CONF node = %p\n", node);
		printf("CONF collection = %p\n", collection);
		printf("CONF mode = %d\n", mode);
		printf("CONF data = %p\n", data);
		printf("CONF parallel = %p\n", parallel);
		
	}
	AdvJSON New();
	AdvJSON Edit();
	void Delete();
	void Erase();
	
	//test
	JSONode *GetNode() {return node;};

public:
	enum MODE {
		MODE_NONE,
		MODE_NEW,
		MODE_EDIT,
	};
private:
	void *collection;
	JSONode *node;
	MODE mode;
	char *data;
	AdvJSONParallel *parallel;
	AdvJSON *rt;
};

#include "AdvVariousType.h"
#include "AdvJSONParallel.h"
#include "AdvJSONCreator.h"
#include "AdvJSONCreatorParallel.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int JSON_Validator(const char *json);
int JSON_ShowError(const char *json, int pos);

JSONode *JSON_Parser(const char *json);
void JSON_Destory(JSONode **json);

JSONode *JSON_Copy(JSONode *json);
void JSON_Get(JSONode *json, const char *path, char *result, int size);
void JSON_Cmd(JSONode *json, const char *path, char *assign, int size);
void JSON_Print(JSONode *json);
void JSON_PrintLink(JSONode *json);

void JSON_ReleaseCollection(JSONode *node);

#ifdef __cplusplus
}
#endif

#endif //__ADVANTECH_JSON_H__

