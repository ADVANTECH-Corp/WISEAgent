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
#include "jsoncollection.h"
#include "jsonrule.h"
#include "jsonparser.h"
#include "jsoncreator.h"
#include "jsonprint.h"
#ifdef __cplusplus
}
#endif

#include "AdvJSON.h"

using namespace std;

void AdvJSON::Init() {
	node = NULL;
	collection = NULL;
	mode = MODE_NONE;
	data = NULL;
	parallel = NULL;
}

void AdvJSON::Release() {
	if(collection != NULL) {
		ReleaseCollection((JSONCollect**)&collection);
	}
	
	if(data != NULL) {
		free(data);
		data = NULL;
	}
	
	if(parallel != NULL) {
		delete parallel;
		parallel = NULL;
	}
	
	Init();
}

bool AdvJSON::IsNULL() {
	if(node == NULL) return true;
	else return false;
}

AdvJSON::AdvJSON() {
	Init();
}

AdvJSON::AdvJSON(const char *json) {
	Init();
	if(json != NULL) {
		int ret = JSON_Validator(json);
		if(0 == ret) {
			JSONCollect *c = NewCollection();
			data = strdup(json);
			collection = c;
			node = ParseObject(data, c, NULL, NULL);
		} else {
			printf("format error(%d)\n",ret);
		}
	} 
}

AdvJSON::AdvJSON(AdvJSON *nJson) {
	Init();
	node = nJson->node;
	mode = nJson->mode;
}

AdvJSON::AdvJSON(const AdvJSON &rJson) {
	Init();
	node = rJson.node;
	mode = rJson.mode;
}

AdvJSON::~AdvJSON() {
	Release();
}

AdvJSON AdvJSON::New() { 
	AdvJSON rt;
	rt(node).mode = MODE_NEW;
	return rt;
}

AdvJSON AdvJSON::Edit() { 
	AdvJSON rt;
	rt(node).mode = MODE_EDIT; 
	return rt;
}

int AdvJSON::Validator(const char *json) {
	return JSON_Validator(json);
}

AdvJSON& AdvJSON::operator()(JSONode *jnode) {
	Init();
	node = jnode;
	return *this;
}

AdvJSON& AdvJSON::operator()(const char *str) {
	//printf("<%s,%d> str = %s\n",__FILE__,__LINE__,str);
	if(str == NULL) return *this;
	if(node == NULL) {
		char *pos = skip((char *)str);
		if(*pos == '{') {
			if(0 == JSON_Validator(pos)) {
				JSONCollect *c = NewCollection();
				Init();
				data = strdup(str);
				collection = c;
				node = ParseObject(data, c, NULL, NULL);
			}
		}
	}
	//printf("<%s,%d>\n",__FILE__,__LINE__);
	if(node == NULL || mode == MODE_NONE) return *this;
	//printf("<%s,%d>\n",__FILE__,__LINE__);

	int len;
	switch(node->type) {
		case JSON_TYPE_VALUE:
		case JSON_TYPE_STRING:
			len = strlen(str);
			node->s = jmalloc(node->s, &node->alloc, len + 1);
			node->type = JSON_TYPE_STRING;
			//doubleQuote(node->s, str, &len);
			memcpy(node->s,str,len);
			node->s[len] = 0;
			node->len = len;
			return *this;
		default:
			return *this;
	}
	return *this;
}

AdvJSON& AdvJSON::operator()(double number) {
	if(node == NULL || mode == MODE_NONE) return *this;
	switch(node->type) {
		case JSON_TYPE_VALUE:
		case JSON_TYPE_NUMBER:
			node->type = JSON_TYPE_NUMBER;
			if (fabs(((double)(int)number)-number)<=DBL_EPSILON && number<=INT_MAX && number>=INT_MIN)
			{
				node->s = jmalloc(node->s, &node->alloc, 21);
				if(node->s) sprintf(node->s,"%d",(int)number);
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
			return *this;
		default:
			return *this;
	}
	return *this;
}

AdvJSON& AdvJSON::operator()(bool boolean) {
	if(node == NULL || mode == MODE_NONE) return *this;
	switch(node->type) {
		case JSON_TYPE_VALUE:
		case JSON_TYPE_BOOL:
			node->s = jmalloc(node->s, &node->alloc, 5);
			node->type = JSON_TYPE_BOOL;
			if(boolean) {
				strcpy(node->s,"true");
				node->len = 4;
			} else {
				strcpy(node->s,"false");
				node->len = 5;
			}
			return *this;
		default:
			return *this;
	}
	return *this;
}

bool AdvJSON::operator==(void *ptr) {
	if(node == ptr) return true;
	else return false;
}

bool AdvJSON::operator==(AdvJSON other) {
	if(node == other.node) return true;
	else return false;
}

AdvJSON AdvJSON::operator[](const char *name) {
	JSONode* target = NULL;
	JSONode* key = NULL;
	JSONode* root = NULL;
	JSONode* prev = NULL;
	AdvJSON rt;
	if(node == NULL) return rt;
	char *token = (char *)name;
	//char token[256];
	//int len = strlen(name);
	//doubleQuote(token, name, &len);
	if(mode == MODE_NONE || mode == MODE_EDIT) {
		if(node->type == JSON_TYPE_OBJECT) {
			target = node->next;
			while(target != NULL) {
				key = target->key;
				if(strncmp(token,key->s,key->len) == 0) {
					return mode == MODE_EDIT ? rt(target->value).Edit() : rt(target->value);
				}
				target = target->next;
			}
		}
		return rt;
	} else if(mode == MODE_NEW) {
		
		switch(node->type) {
			case JSON_TYPE_OBJECT:
				target = node->next;
				if(target == NULL) {
					node->next = CreatePair(token, (JSONCollect *)node->collection, node, node);
					target = node->next;
				} else {
					while(target != NULL) {
						key = target->key;
						if(strncmp(token,key->s,key->len) == 0) {
							return rt(target->value).New();
						}
						prev = target;
						target = target->next;
					}
					target = prev;
					target->next = CreatePair(token, (JSONCollect *)target->collection, node, target);
					target = target->next;
				}
				return rt(target->value).New();
			break;
			case JSON_TYPE_VALUE:
				root = node->root;
				target = CreateObject(token, (JSONCollect *)node->collection, root, root, node);
				root->value = target;
				node = target;
				return rt(node->next->value).New();
			default:
				return rt;
		}
		return rt;
	}
	return rt;
}

JSONode* AdvJSON::NewArray(JSONode*node, int mode, int index) {
	int count = 0;
	JSONode* target = NULL;
	JSONode* root = NULL;
	if(node == NULL) return NULL;
	if(mode == MODE_NONE || mode == MODE_EDIT) {
		switch(node->type) {
			case JSON_TYPE_ARRAY:
				target = node->array;
				while(count != index && target != NULL) {
					count++;
					target = target->array;
				}
				if(target == NULL) return NULL;
				return target->value;
			default:
				return NULL;
		}
	} else if(mode == MODE_NEW) {
		switch(node->type) {
			case JSON_TYPE_VALUE:
				root = node->root;
				target = CreateArray((JSONCollect *)node->collection, root, root, node);
				root->value = target;				
				node = target;
			case JSON_TYPE_ARRAY:
				target = node->array;
				if(target == NULL) {
					node->array = CreateElement((JSONCollect *)node->collection, node, node);
					target = node->array;
				}
				
				while(count != index && target != NULL) {
					count++;
					if(target->array == NULL) {
						target->array = CreateElement((JSONCollect *)node->collection, node, target);
					}
					target = target->array;
				}

				return target->value;
			default:
				return NULL;
		}
	}
	return NULL;
}

AdvJSON AdvJSON::operator[](int index) {
	int count = 0;
	JSONode* target = NULL;
	JSONode* root = NULL;
	AdvJSON rt;
	if(node == NULL) return rt;
	if(mode == MODE_NONE || mode == MODE_EDIT) {
		switch(node->type) {
			case JSON_TYPE_ARRAY:
				target = node->array;
				while(count != index && target != NULL) {
					count++;
					target = target->array;
				}
				if(target == NULL) return rt;
				return mode == MODE_EDIT ? rt(target->value).Edit() : rt(target->value);
			case JSON_TYPE_OBJECT:
				target = node->next;
				while(count != index && target != NULL) {
					count++;
					target = target->next;
				}
				if(target == NULL) return rt;
				return mode == MODE_EDIT ? rt(target->value).Edit() : rt(target->value);
			default:
				return rt;
		}
	} else if(mode == MODE_NEW) {
		switch(node->type) {
			case JSON_TYPE_VALUE:
				root = node->root;
				target = CreateArray((JSONCollect *)node->collection, root, root, node);
				root->value = target;				
				node = target;
			case JSON_TYPE_ARRAY:
				target = node->array;
				if(target == NULL) {
					node->array = CreateElement((JSONCollect *)node->collection, node, node);
					target = node->array;
				}
				
				while(count != index && target != NULL) {
					count++;
					if(target->array == NULL) {
						target->array = CreateElement((JSONCollect *)node->collection, node, target);
					}
					target = target->array;
				}

				return rt(target->value).New();
			default:
				return rt;
		}
	}
	return rt;
}

AdvJSON &AdvJSON::operator<<(const char *str) {
	JSONode *key = NULL;
	int len;
	if(node == NULL || mode == MODE_NONE) return *this;
	if(node->root != NULL) {
		key = node->root->key;
		if(key != NULL) {
			switch(key->type) {
				case JSON_TYPE_STRING:
					len = strlen(str);
					key->s = jmalloc(key->s, &key->alloc, len + 1);
					//doubleQuote(key->s, str, &len);
					memcpy(key->s,str,len);
					key->s[len] = 0;
					key->len = len;
					return *this;
				default:
					return *this;
			}
		}
	}
	return *this;
}

string AdvJSON::Key() {
	if(node == NULL) return string("NULL");
	JSONode *key = node->prev->key;
	if(key->s == NULL || key->len == 0) return string("NULL");
	if(key->type == JSON_TYPE_STRING) return string(key->s,key->len);
	return string("Type Error!!");
}

int AdvJSON::Size() {
	if(node == NULL) return 0;
	if(node->type == JSON_TYPE_ARRAY) {
		int count = 0;
		JSONode *target = node->array;
		while(target != NULL) {
			count++;
			target = target->array;
		}
		return count;
	} else if(node->type == JSON_TYPE_OBJECT) {
		int count = 0;
		JSONode *target = node->next;
		while(target != NULL) {
			count++;
			target = target->next;
		}
		return count;
	}
	return 0;
}

string AdvJSON::Value() {
	if(node == NULL || node->s == NULL || node->len == 0) return string("NULL");
	if(node->s[0] == '\"') {
		if(node->len == 2) return string("");
		return string(node->s);
	}
	
	return string(node->s,node->len);
}

string AdvJSON::String() {
	if(node == NULL || node->s == NULL || node->len == 0) return string("NULL");
	if(node->type == JSON_TYPE_STRING) return string(node->s,node->len);
	return string("Type Error!!");
}

static double TransToNumber(double sign, double n, double scale, int subscale, int signsubscale) {
	return sign*n*pow(10.0,(scale+subscale*signsubscale));
}

static double ToNumber(char *num, int len) {
	double n=0;
	double sign=1;
	double scale=0;
	int subscale=0;
	int signsubscale=1;

	if (*num=='-') {
		sign=-1;num++;len--;
	}

	if (*num=='0') {
		num++;len--;
	}

	if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);
	
	if (*num>='1' && *num<='9')	{
		do{
			n=(n*10.0)+(*num++ -'0');
			len--;
			if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);
		} while (*num>='0' && *num<='9');
	}
	
	if (*num=='.' && num[1]>='0' && num[1]<='9') {
		num++;
		len--;
		if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);		
		do {
			n=(n*10.0)+(*num++ -'0');
			scale--; 
			len--;
			if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);
		} while (*num>='0' && *num<='9');
	}
	
	if (*num=='e' || *num=='E')
	{	
		num++;
		if (*num=='+') {
			num++;
			len--;
			if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);
		} else if (*num=='-') {
			signsubscale=-1;
			num++;
			len--;
			if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);
		}
		
		while (*num>='0' && *num<='9') {
			subscale=(subscale*10)+(*num++ - '0');
			len--;
			if(len == 0) return TransToNumber(sign, n, scale, subscale, signsubscale);
		}
	}
	return TransToNumber(sign, n, scale, subscale, signsubscale);;
}

int AdvJSON::Int() {
	if(node == NULL || node->s == NULL || node->len == 0) return 0;
	if(node->type == JSON_TYPE_NUMBER) return (int)ToNumber(node->s, node->len);
	return 0;
}
float AdvJSON::Float() {
	if(node == NULL || node->s == NULL || node->len == 0) return 0;
	if(node->type == JSON_TYPE_NUMBER) return (float)ToNumber(node->s, node->len);
	return 0;
}
double AdvJSON::Double() {
	if(node == NULL || node->s == NULL || node->len == 0) return 0;
	if(node->type == JSON_TYPE_NUMBER) return ToNumber(node->s, node->len);
	return 0;
}
bool AdvJSON::Bool() {
	if(node == NULL || node->s == NULL || node->len == 0) return false;
	if(node->type == JSON_TYPE_BOOL) {
		if(node->s[0] == 't') return true;
		else return false;
	}
	return false;
}

void AdvJSON::Print(char *buffer, int size, int depth) {
	if(node == NULL) return;
	switch(node->type) {
		case JSON_TYPE_STRING:
			PrintString(buffer, size, node);
			return;
		case JSON_TYPE_NUMBER:
			PrintNumber(buffer, size, node);
			return;
		case JSON_TYPE_OBJECT:
			PrintObject(buffer, size, node, depth);
			return;
		case JSON_TYPE_ARRAY:
			PrintArray(buffer, size, node, depth);
			return;
		case JSON_TYPE_VALUE:
			PrintValue(buffer, size, node, depth);
			return;
		default:
			return;
	}
}
void AdvJSON::Print() {
	char temp[8192];
	Print(temp, sizeof(temp));
	printf("\033[33m%s\033[0m\n",temp);
}

void AdvJSON::Print(int depth) {
	char temp[8192];
	Print(temp, sizeof(temp),depth);
	printf("\033[33m%s\033[0m\n",temp);
}

void AdvJSON::PrintLink(int depth) {
	if(node == NULL) return;
	PLObject(node, 0);
}
void AdvJSON::Delete() {
	if(node == NULL || mode == MODE_NONE) return;
	JSONode *del = node->root;
	switch(del->type) {
		case JSON_TYPE_PAIR:
		case JSON_TYPE_ELEMENT:
			DeleteNode(del);
			break;
	}
	return;
}

void AdvJSON::Erase() {
	if(node == NULL || mode == MODE_NONE) return;
	JSONode *head = node->root;
	switch(head->type) {
		case JSON_TYPE_PAIR:
		case JSON_TYPE_ELEMENT:
			if(node->type != JSON_TYPE_VALUE) {
				node->root = NULL;
				DeleteNode(node);
				node = CreateNullValue((JSONCollect *)head->collection, head, head);
			}
			break;
	}
	return;
}


#if __cplusplus > 199711L
AdvJSONParallel &AdvJSON::operator[](std::initializer_list<const char *> list) {
	if(parallel == NULL) parallel = new AdvJSONParallel();
	(*parallel)();
	if(mode != MODE_NONE) {
		(*parallel)(*this,mode);
		for (initializer_list<const char *>::iterator iter = list.begin(); iter != list.end(); iter++) {
			(*parallel)[*iter];		
		}
	}

	return *parallel;
}
#endif

void AdvJSON::GetInitialArray(AdvVariousType *avt, JSONode *jn, int mode) {
	if(jn == NULL) return;
	AdvVariousType *element = NULL;
	//int type;
	int size = avt->Count();
	//json.mode = MODE_NEW;
	for(int i = 0 ; i < size; i++) {
		element = avt->Get(i);
		if(element->Count() == 1) {
			//JSONode *target = json[i].node;
			JSONode *target = NewArray(jn, mode, i);
			JSONode *root;
			if(target != NULL) {
				if(target->type == JSON_TYPE_VALUE) {
					root = target->root;
					root->value = element->node();
					UnlinkItem((JSONCollect*)root->value->collection, root->value);
					LinkItem((JSONCollect*)target->collection, root->value);
					target->root = NULL;
					DeleteNode(target);
					target = root->value;
					target->root = root;
					target->prev = root;
				}
			}
		} else {
			AdvJSON::GetInitialArray(element, NewArray(jn, mode, i));
		}
	}
}

#if __cplusplus > 199711L
AdvJSON &AdvJSON::operator=(initializer_list<AdvVariousType> list) {
	if(node == NULL) return *this;
	if(mode != MODE_NONE) {
		int count = 0;
		int size;
		auto iter = list.begin();
		AdvVariousType *element = NULL;
		for (; iter != list.end(); iter++) {
			size = ((AdvVariousType*)iter)->count;
			element = (AdvVariousType*)iter;
			if(size == 1) {
				//JSONode *target = (*this)[count].node;
				JSONode *target = NewArray(node, mode, count);
				JSONode *root;
				if(target != NULL) {
					if(target->type == JSON_TYPE_VALUE) {
						root = target->root;
						root->value = element->node();
						UnlinkItem((JSONCollect*)root->value->collection, root->value);
						LinkItem((JSONCollect*)target->collection, root->value);
						target->root = NULL;
						DeleteNode(target);
						target = root->value;
						target->root = root;
						target->prev = root;
					}
				}
			} else {
				AdvJSON::GetInitialArray(element,  NewArray(node, mode, count), mode);
			}
			
			count++;
		}
	}
	return *this;
}
#endif

AdvJSON &AdvJSON::operator=(JSONode *other) {
	if(node == NULL || mode == MODE_NONE) return *this;
	JSONode *root = NULL;
	if(node->type == JSON_TYPE_VALUE) {
		root = node->root;
		root->value = CopyNode(other, (JSONCollect*)node->collection);
		node->root = NULL;
		DeleteNode(node);
		node = root->value;
		node->root = root;
		node->prev = root;
	}
	return *this;
}

#if __cplusplus > 199711L
AdvJSON &AdvJSON::operator+=(initializer_list<AdvVariousType> list) {
	if(node == NULL) return *this;
	if(mode == MODE_NEW) {
		int count = Size();
		int size;
		auto iter = list.begin();
		AdvVariousType *element = NULL;
		for (; iter != list.end(); iter++) {
			size = ((AdvVariousType*)iter)->count;
			element = (AdvVariousType*)iter;
			if(size == 1) {
				//JSONode *target = (*this)[count].node;
				JSONode *target = NewArray(node, mode, count);
				JSONode *root;
				if(target != NULL) {
					if(target->type == JSON_TYPE_VALUE) {
						root = target->root;
						root->value = element->node();
						UnlinkItem((JSONCollect*)root->value->collection, root->value);
						LinkItem((JSONCollect*)target->collection, root->value);
						target->root = NULL;
						DeleteNode(target);
						target = root->value;
						target->root = root;
						target->prev = root;
					}
				}
			} else {
				AdvJSON::GetInitialArray(element, NewArray(node, mode, count), mode);
			}
			
			count++;
		}
	}
	return *this;
}
#endif


/***********************************************************************************/

AdvJSONParallel &AdvJSONParallel::operator[](const char *name) {
	paraList.push_back(json[name]);
	return *this;
}

/*AdvJSONParallel &AdvJSONParallel::operator[](int index) {
	paraList.push_back(json[index]);
	return *this;
}*/

#if __cplusplus > 199711L
AdvJSON &AdvJSONParallel::operator=(initializer_list<AdvVariousType> list) {
	int type;
	auto iter = list.begin();
	auto item = paraList.begin();
	int count = 0;
	int size;
	AdvVariousType *element = NULL;
	for (; iter != list.end() && item != paraList.end(); iter++, item++) {
		item->mode = (AdvJSON::MODE)mode;
		
		size = ((AdvVariousType*)iter)->count;
		element = (AdvVariousType*)iter;
		if(size == 1) {
			JSONode *target = (*item).node;
			JSONode *root;
			if(target != NULL) {
				if(target->type == JSON_TYPE_VALUE) {
					root = target->root;
					root->value = element->node();
					UnlinkItem((JSONCollect*)root->value->collection, root->value);
					LinkItem((JSONCollect*)target->collection, root->value);
					target->root = NULL;
					DeleteNode(target);
					target = root->value;
					target->root = root;
					target->prev = root;
				}
			}
		} else {
			AdvJSON::GetInitialArray(element, (*item).node, mode);
		}
	}
}
#endif

/***********************************************************************************/
AdvJSONCreator::AdvJSONCreator() {
	collection = NULL;
	parallel = NULL;
}

AdvJSONCreator::AdvJSONCreator(AdvJSON &json) {
	collection = json.collection;
	parallel = NULL;
}

AdvJSONCreator::AdvJSONCreator(const AdvJSONCreator &creator) { 
	collection = creator.collection;
}

AdvJSONCreator::~AdvJSONCreator() {
	if(parallel != NULL) {
		delete parallel;
		parallel = NULL;
	}
}
#if __cplusplus > 199711L
AdvJSONCreatorParallel &AdvJSONCreator::operator[](initializer_list<const char *> list) {
	if(parallel == NULL) parallel = new AdvJSONCreatorParallel();
	(*parallel)(*this);

	for (auto iter = list.begin(); iter != list.end(); iter++) {
		(*parallel)[*iter];
	}
	
	return *parallel;
}
#endif
/***********************************************************************************/

AdvJSONCreatorParallel::AdvJSONCreatorParallel(AdvJSONCreator &jc) {
	paraList.clear();
	json(ParseObject("{}", (JSONCollect*)jc.collection, NULL, NULL));
}

void AdvJSONCreatorParallel::operator()(AdvJSONCreator &jc) {
	paraList.clear();
	json(ParseObject("{}", (JSONCollect*)jc.collection, NULL, NULL));
}

AdvJSONCreatorParallel &AdvJSONCreatorParallel::operator[](const char *name) {
	paraList.push_back(json.New()[name]);
	return *this;
}

#if __cplusplus > 199711L
JSONode *AdvJSONCreatorParallel::operator()(initializer_list<AdvVariousType> list) {
	int type;
	int keyType;
	auto iter = list.begin();
	auto item = paraList.begin();
	int count = 0;
	int size;
	AdvVariousType *element = (AdvVariousType*)iter;
	for (; iter != list.end() && item != paraList.end(); iter++, item++) {
		
		size = ((AdvVariousType*)iter)->Count();
		element = (AdvVariousType*)iter;
		if(size == 1) {
			JSONode *target = item->node;
			JSONode *root;
			if(target != NULL) {
				if(target->type == JSON_TYPE_VALUE) {
					root = target->root;
					root->value = element->node();
					UnlinkItem((JSONCollect*)root->value->collection, root->value);
					LinkItem((JSONCollect*)target->collection, root->value);
					target->root = NULL;
					DeleteNode(target);
					target = root->value;
					target->root = root;
					target->prev = root;
				}
			}
		} else {
			AdvJSON::GetInitialArray(element, item->node);
		}
	}
	
	return json.GetNode();
}
#endif