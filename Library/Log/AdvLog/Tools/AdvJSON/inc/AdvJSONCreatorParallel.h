/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADVANTECH_JSON_CREATOR_PARALLEL_H__
#define __ADVANTECH_JSON_CREATOR_PARALLEL_H__


#include <list>
typedef struct __JSONode JSONode;

class AdvJSONCreator;
class AdvJSONCreatorParallel {
public:
	friend class AdvJSONCreator;
	
private:
	AdvJSON json;
	std::list<AdvJSON> paraList;

private:
	AdvJSONCreatorParallel &operator[](const char *name);
	
public:
	AdvJSONCreatorParallel() {}
	AdvJSONCreatorParallel(AdvJSONCreator &jc);
	void operator()(AdvJSONCreator &jc);
	
	
#if __cplusplus > 199711L
	JSONode *operator()(std::initializer_list<AdvVariousType> list);
#endif
	
};

#endif //__ADVANTECH_JSON_PARALLEL_H__