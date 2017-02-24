/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADVANTECH_VARIOUS_TYPE_H__
#define __ADVANTECH_VARIOUS_TYPE_H__

#include <vector>

/*enum VARIOUS_TYPE {
	VT_VOID,
	
	VT_CHAR,
	VT_CHAR_P,
	VT_UNSIGNED_CHAR,
	VT_UNSIGNED_CHAR_P,
	
	VT_SHORT,
	VT_SHORT_P,
	VT_UNSIGNED_SHORT,
	VT_UNSIGNED_SHORT_P,
	
	VT_INT,
	VT_INT_P,					//10
	VT_UNSIGNED_INT,
	VT_UNSIGNED_INT_P,
	
	VT_LONG,
	VT_LONG_P,
	VT_UNSIGNED_LONG,
	VT_UNSIGNED_LONG_P,
	
	VT_FLOAT,
	VT_FLOAT_P,
	VT_DOUBLE,
	VT_DOUBLE_P,				//20
	
	VT_ADVJSON,
	VT_ADVJSON_P,
	VT_JSONODE_P,
	
	VT_STRING,
	VT_NESTED_INIT_LIST,
	VT_BOOL,
	VT_ARRAY,
};*/

typedef struct __JSONode JSONode;
class AdvJSON;
class AdvVariousType {
public:
	friend class AdvJSON;
	friend class AdvJSONParallel;
private:
	/*enum VARIOUS_TYPE type;
	long n;
	float f;
	double d;
	void *p;
	std::string string;
	AdvJSON j;*/
	JSONode *jn;
	int count;
	AdvVariousType *a;
	
public:
	AdvVariousType() {	
						jn = 0;
						count = 0;
						//a = 0;
					 }
	AdvVariousType(const char *str);// {/*printf("@string@\n");*/type = VT_CHAR_P; string = str;}
	AdvVariousType(bool b);// {/*printf("@bool@\n");*/type = VT_BOOL, n = b;}
	AdvVariousType(int num);// {/*printf("@int@\n");*/type = VT_INT, n = num;}
	AdvVariousType(float num);// {/*printf("@float@\n");*/type = VT_FLOAT, f = num;}
	AdvVariousType(double num);// {/*printf("@double@\n");*/type = VT_DOUBLE, d = num;}
	//AdvVariousType(AdvJSON json);// {/*printf("@json@\n");*/type = VT_ADVJSON, j = json;}
	AdvVariousType(JSONode *node);
	AdvVariousType(const AdvVariousType &other);
	
	
#if __cplusplus > 199711L
	AdvVariousType(std::initializer_list<AdvVariousType> list);
#endif
	
	
	
	~AdvVariousType();
	/*int getType() {return (int)type;}
	
	char *getString() {return (char *)string.c_str();}
	int getNumber() {return n;}
	float getFloat() {return f;}
	double getDouble() {return d;}
	JSONode *getJSONode() {return jn;}*/
	JSONode *node() {return jn;}
	int Count() {return count;}
	AdvVariousType *Get(int i);
	//AdvVariousType &operator[](int index) {return a[index];}
	
	//void ToString(char *buffer, int size);
};


//#define V(element) element
#endif //__ADVANTECH_VARIOUS_TYPE_H__