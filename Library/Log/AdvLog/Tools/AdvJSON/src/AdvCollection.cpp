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
#include "jsoncollection.h"
#ifdef __cplusplus
}
#endif

#include "AdvCollection.h"

class GlobalCollection {
public:
	GlobalCollection() {
		GC_Init();
	}
	~GlobalCollection() {
		GC_Release();
	}
	
	JSONode *Allocate() {
		return GC_NewItem();
	}
	
	void Destory(JSONode *node) {
		GC_DestroyItem(node);
	}
	
	JSONode *AllocateNumber(double number) {
		JSONode *node = Allocate();
		node->type = JSON_TYPE_NUMBER;
		if (fabs(((double)(int)number)-number)<=DBL_EPSILON && number<=INT_MAX && number>=INT_MIN)
		{
			node->s = jmalloc(node->s, &node->alloc, 21);
			if (node->s) sprintf(node->s,"%d",(int)number);
		} else {
			node->s = jmalloc(node->s, &node->alloc, 64);
			if (node->s)
			{
				if (fabs(floor(number)-number)<=DBL_EPSILON && fabs(number)<1.0e60)	sprintf(node->s,"%.0f",number);
				else if (fabs(number)<1.0e-6 || fabs(number)>1.0e9)					sprintf(node->s,"%e",number);
				else																sprintf(node->s,"%f",number);
			}
		}

		node->len = strlen(node->s);
		node->alloc = node->len+1;
		return node;
	}
};

static GlobalCollection gCollect;

JSONode *Allocate_GC() {
	return gCollect.Allocate();
}

void Destory_GC(JSONode *node) {
	gCollect.Destory(node);
}

JSONode *AllocateNumber_GC(double number) {
	return gCollect.AllocateNumber(number);
}