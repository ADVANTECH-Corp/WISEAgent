#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <math.h>
#include <float.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "jsontool.h"
#include "jsontype.h"
#include "jsoncreator.h"
#ifdef __cplusplus
}
#endif
#include "AdvCollection.h"
#include "AdvVariousType.h"




AdvVariousType::AdvVariousType(const char *str) {
	int len = strlen(str);
	jn = Allocate_GC();
	jn->type = JSON_TYPE_STRING;
	jn->s = jmalloc(jn->s, &jn->alloc, len + 1);
	//doubleQuote(jn->s, str, &len);
	memcpy(jn->s,str,len);
	jn->s[len] = 0;
	jn->len = len;
	count = 1;
	//a = 0;
}

AdvVariousType::AdvVariousType(bool b) {
	jn = Allocate_GC();
	jn->type = JSON_TYPE_BOOL;
	jn->s = jmalloc(jn->s, &jn->alloc, 5);
	
	
	if(b) {
		strcpy(jn->s,"true");
		jn->len = 4;
	} else {
		strcpy(jn->s,"false");
		jn->len = 5;
	}
	
	count = 1;
	//a = 0;
}

AdvVariousType::AdvVariousType(int num) {
	jn = AllocateNumber_GC((double)num);
	count = 1;
	//a = 0;
}
AdvVariousType::AdvVariousType(float num) {
	jn = AllocateNumber_GC((double)num);
	count = 1;
	//a = 0;
}
AdvVariousType::AdvVariousType(double num) {
	jn = AllocateNumber_GC((double)num);
	count = 1;
	//a = 0;
}

AdvVariousType::AdvVariousType(JSONode *node) {
	count = 1;
	jn = node;
	//a = 0;
}
#if __cplusplus > 199711L
AdvVariousType::AdvVariousType(std::initializer_list<AdvVariousType> list) {
	jn = NULL;
	count = 0;
	AdvVariousType* element;
	AdvVariousType target;
	//a = new AdvVariousType[list.size()];
	for (auto iter = list.begin(); iter != list.end(); iter++) {
		element = (AdvVariousType*)iter;
		a[count].jn = element->jn;
		a[count].a = element->a;
		a[count].count = element->count;
		
		element->jn = NULL;
		element->count = 0;
		element->a = NULL;
		count++;
	}
}
#endif
AdvVariousType::AdvVariousType(const AdvVariousType &other) {

}

AdvVariousType *AdvVariousType::Get(int i) {
	return &a[i];
}

AdvVariousType::~AdvVariousType() {
	Destory_GC(jn);
	jn = NULL;
	/*if(a != NULL) {
		delete [] a;
		a = NULL;
	}*/
}