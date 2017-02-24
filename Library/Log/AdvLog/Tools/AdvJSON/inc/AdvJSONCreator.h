/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADVANTECH_JSON_CREATOR_H__
#define __ADVANTECH_JSON_CREATOR_H__

typedef struct __JSONode JSONode;

class AdvJSONCreatorParallel;
class AdvJSON;
class AdvJSONCreator {
public:
	friend class AdvJSONCreatorParallel;
	
public:
	AdvJSONCreator();
	
	AdvJSONCreator(AdvJSON &json);
	
	AdvJSONCreator(const AdvJSONCreator &creator);
	~AdvJSONCreator();
	AdvJSON &operator=(AdvJSON other) 	{ collection = other.collection;}

#if __cplusplus > 199711L
	AdvJSONCreatorParallel &operator[](std::initializer_list<const char *> list);
#endif

private:
	void *collection;
	AdvJSONCreatorParallel *parallel;
};

#endif //__ADVANTECH_JSON_CREATOR_H__