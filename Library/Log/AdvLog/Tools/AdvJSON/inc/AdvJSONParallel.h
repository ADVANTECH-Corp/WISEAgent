/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADVANTECH_JSON_PARALLEL_H__
#define __ADVANTECH_JSON_PARALLEL_H__


#include <list>
#include <string>
typedef struct __JSONode JSONode;

class AdvVariousType;
class AdvJSON;
class AdvJSONParallel {
public:
	friend class AdvJSON;	

private:
	std::list<AdvJSON> paraList;
	int mode;
	AdvJSON json;
	
private:
	AdvJSONParallel &operator[](const char *name);
	//AdvJSONParallel &operator[](int index);
	
public:
	AdvJSONParallel() {mode = AdvJSON::MODE_NONE;}
	AdvJSONParallel(AdvJSON json, int mode) { this->json = json; this->json.mode = (AdvJSON::MODE)mode; this->mode = mode; paraList.clear();}
	~AdvJSONParallel() { this->json.Release(); paraList.clear();}
	
	void operator()(AdvJSON json, int mode) { this->json = json; this->json.mode = (AdvJSON::MODE)mode; this->mode = mode; paraList.clear();}
	void operator()() { this->json.Release(); this->mode = AdvJSON::MODE_NONE; paraList.clear();}
	
#if __cplusplus > 199711L
	AdvJSON &operator=(std::initializer_list<AdvVariousType> list);
#endif
	
};

#endif //__ADVANTECH_JSON_PARALLEL_H__
