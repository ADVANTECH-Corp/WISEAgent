#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "jsontool.h"
#include "jsonparser.h"

#define NODE_ASSIGN(n,start,length) do {\
										n->s = start;\
										n->len = length;\
										}while(0)


#define NODE_ALLOCATE(n,c) do {\
							 n = GetNewItem(c);\
							 if(n->alloc != 0) {\
								 free(n->s);\
							 }\
							 n->alloc = 0;\
							 n->s = NULL;\
						   }while(0)


JSONode *ParseString(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = NULL;
	int len = 0;
	char *s;
	char *e;
	char *b = NULL;
	char *p = (char *)json;
	
	p = skip(p);
	////ADV_DEBUG("<%s,%s,%d> p = %s\n", __FILE__,__func__,__LINE__,p);
	if(*p != '\"') {
		return NULL;
	}
	
	NODE_ALLOCATE(node,collect);
	node->type = JSON_TYPE_STRING;
	node->root = root;
	node->prev = prev;
	s = p; //string start;
	p++;
	p = strchr(p,'\"');
	
	b = p-1;
	while(*b == '\\') {
		p++;
		p = strchr(p,'\"');
		b = p-1;
	}
	
	e = p; //string end;
	len = (int)((long)e - (long)s) + 1;
	NODE_ASSIGN(node,s+1,len-2);
	return node;
}

JSONode *ParseNumber(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node;
	int len = 0;
	char *p = (char *)json;
	char *s;

	p = skip(p);
	if(*p != '-' && !isdigit(*p)) {
		return NULL;
	}
	
	NODE_ALLOCATE(node,collect);
	node->type = JSON_TYPE_NUMBER;
	node->root = root;
	node->prev = prev;
	
	s = p;
	len = strspn(p,"+-01234567890.eE");
	NODE_ASSIGN(node,s,len);
	return node;
	
}

JSONode *ParseObject(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node;
	JSONode *head = NULL;
	JSONode *leaf = NULL;
	JSONode *element = NULL;
	JSONode *temp = NULL;
	int len = 0;
	char *s;
	char *e;
	char *es;
	char *ee;
	char *p = (char *)json;
	
	p = skip(p);
	if(*p != '{') {
		return NULL;
	}
	
	NODE_ALLOCATE(node,collect);
	node->type = JSON_TYPE_OBJECT;
	node->root = root;
	node->prev = prev;
	////ADV_DEBUG("@@@@@@@@@@@@@@@@@node = %p\n", node);
	
	s = p; //object start
	p++;
	p = skip(p);
	////ADV_DEBUG("<%s,%d>\n",__FILE__,__LINE__);
	if(*p == '}') {
		e = p; //object end
		len = (int)((long)e - (long)s) + 1;
		NODE_ASSIGN(node,s,len);
		return node;
	}
	////ADV_DEBUG("<%s,%d>\n",__FILE__,__LINE__);
	
	
	NODE_ALLOCATE(leaf,collect);
	
	leaf->type = JSON_TYPE_PAIR;
	es = p; //object start
	element = ParseString(p, collect, leaf, leaf);
	////ADV_DEBUG("<%s,%d>key: element->len = %d\n",__FILE__,__LINE__, element->len);
	p += element->len + 2;
	//assign key
	leaf->key = element;
	element->prev = leaf;
	
	p = skip(p);
	p++; //skip ':'
	p = skip(p);
	element = ParseValue(p, collect, leaf, leaf);
	////ADV_DEBUG("<%s,%d>value: p = %s\n",__FILE__,__LINE__, p);
	////ADV_DEBUG("<%s,%d>value: element->len = %d\n",__FILE__,__LINE__, element->len);
	////ADV_DEBUG("<%s,%d>value: element->type = %d\n",__FILE__,__LINE__, element->type);
	////ADV_DEBUG("<%s,%d>value: element->s = %p\n",__FILE__,__LINE__, element->s);
	p += element->len;
	if(element->type == JSON_TYPE_STRING) p+=2;
	//assign value
	leaf->value = element;
	element->prev = leaf;
	
	ee = p; //object end
	len = (int)((long)ee - (long)es) + 1;
	NODE_ASSIGN(leaf,es,len);
	
	node->next = leaf;
	leaf->prev = node;
	leaf->root = node;
	////ADV_DEBUG("leaf = %p\n", leaf);
	
	p = skip(p);
	head = leaf;
	while(*p == ',') {
		NODE_ALLOCATE(leaf,collect);
		leaf->type = JSON_TYPE_PAIR;
		head->next = leaf;
		leaf->prev = head;
		leaf->root = node;
		////ADV_DEBUG("leaf = %p\n", leaf);
		p++;
		p = skip(p);
		es = p; //object start
		element = ParseString(p, collect, leaf, leaf);
		p += element->len + 2;
		//assign key
		leaf->key = element;
		element->prev = leaf;
		
		p = skip(p);
		p++; //skip ':'
		p = skip(p);
		element = ParseValue(p, collect, leaf, leaf);
		p += element->len;
		if(element->type == JSON_TYPE_STRING) p+=2;
		//assign value
		leaf->value = element;
		element->prev = leaf;
		element->key = leaf->key;
		
		ee = p; //object end
		len = (int)((long)ee - (long)es) + 1;
		NODE_ASSIGN(leaf,es,len);
		
		p = skip(p);
		head = leaf;
	}
	head->next = NULL;
	e = p;
	len = (int)((long)e - (long)s) + 1;
	NODE_ASSIGN(node,s,len);
	////ADV_DEBUG("@@@@@@@@@@@@@@@@@node = %p, node->array = %p\n", node, node->array);
	
	return node;
}

JSONode *ParseArray(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node;
	JSONode *head = NULL;
	JSONode *leaf = NULL;
	JSONode *element = NULL;
	int len = 0;
	char *s;
	char *e;
	char *es;
	char *ee;
	char *p = (char *)json;
	
	p = skip(p);
	if(*p != '[') {
		return NULL;
	}
	
	NODE_ALLOCATE(node,collect);
	node->type = JSON_TYPE_ARRAY;
	node->root = root;
	node->prev = prev;
	
	s = p; //array start
	p++;
	p = skip(p);
	if(*p == ']') {
		e = p; //array end
		len = (int)((long)e - (long)s) + 1;
		NODE_ASSIGN(node,s,len);
		return node;
	}
	es = p; //array start
	NODE_ALLOCATE(leaf,collect);
	leaf->type = JSON_TYPE_ELEMENT;
	
	////ADV_DEBUG("\t<%s,%s,%d>ParseValue start\n",__FILE__,__func__,__LINE__);
	element = ParseValue(p, collect, leaf, leaf);
	////ADV_DEBUG("\t<%s,%s,%d>ParseValue end\n",__FILE__,__func__,__LINE__);
	////ADV_DEBUG("element->len = %d\n", element->len);
	p += element->len;
	if(element->type == JSON_TYPE_STRING) p+=2;
	
	leaf->value = element;
	element->prev = leaf;
	element->root = leaf;
	ee = p;
	len = (int)((long)ee - (long)es) + 1;
	NODE_ASSIGN(leaf,es,len);
	
	node->array = leaf;
	leaf->prev = node;
	leaf->root = node;
	
	p = skip(p);
	////ADV_DEBUG("<%s,%s,%d>p =%s\n",__FILE__,__func__,__LINE__,p);
	head = leaf;
	while(*p == ',') {
		NODE_ALLOCATE(leaf,collect);
		leaf->type = JSON_TYPE_ELEMENT;
		head->array = leaf;
		leaf->prev = head;
		leaf->root = node;
		p++;
		p = skip(p);
		es = p;
		////ADV_DEBUG("\t<%s,%s,%d>ParseValue start\n",__FILE__,__func__,__LINE__);
		element = ParseValue(p, collect, leaf, leaf);
		////ADV_DEBUG("\t<%s,%s,%d>ParseValue end\n",__FILE__,__func__,__LINE__);
		p += element->len;
		if(element->type == JSON_TYPE_STRING) p+=2;
		ee = p; //object end
		len = (int)((long)ee - (long)es) + 1;
		NODE_ASSIGN(leaf,es,len);
		
		leaf->value = element;
		element->prev = leaf;
		element->root = leaf;
		p = skip(p);
		////ADV_DEBUG("<%s,%s,%d>p =%s\n",__FILE__,__func__,__LINE__,p);
		head = leaf;
	}
	head->array = NULL;

	e = p; //array end
	len = (int)((long)e - (long)s) + 1;
	NODE_ASSIGN(node,s,len);
	return node;
}

JSONode *ParseValue(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = NULL;
	int len = 0;
	char *p = (char *)json;
	char *s;
	p = skip(p);

	switch(*p) {
		case 't':
			if(strncmp(p,"true",4) == 0) {
				s = p;
				NODE_ALLOCATE(node,collect);
				node->type = JSON_TYPE_BOOL;
				node->root = root;
				node->prev = prev;
				NODE_ASSIGN(node,s,4);
				return node;
				
			}
		case 'f':
			if(strncmp(p,"false",5) == 0) {
				s = p;
				NODE_ALLOCATE(node,collect);
				node->type = JSON_TYPE_BOOL;
				node->root = root;
				node->prev = prev;
				NODE_ASSIGN(node,s,5);
				return node;
			}
		break;
		case 'n':
			if(strncmp(p,"null",4) == 0) {
				s = p;
				NODE_ALLOCATE(node,collect);
				node->type = JSON_TYPE_VALUE;
				node->root = root;
				node->prev = prev;
				NODE_ASSIGN(node,s,4);
				return node;
			}
		default:
			node = ParseString(p, collect, root, prev);
			////ADV_DEBUG("<%s,%d>V: ParseString(%d)\n",__FILE__,__LINE__, node->len);
			if(node != NULL) {
				return node;
			}
			////ADV_DEBUG("<%s,%d>V: ParseObject start\n",__FILE__,__LINE__);
			node = ParseObject(p, collect, root, prev);
			////ADV_DEBUG("<%s,%d>V: ParseObject(%d)\n",__FILE__,__LINE__, node->len);
			if(node != NULL) {
				return node;
			}
			////ADV_DEBUG("<%s,%d>V: ParseArray start\n",__FILE__,__LINE__);
			node = ParseArray(p, collect, root, prev);
			////ADV_DEBUG("<%s,%d>V: ParseArray(%d)\n",__FILE__,__LINE__, node->len);
			if(node != NULL) {
				return node;
			}
			node = ParseNumber(p, collect, root, prev);
			////ADV_DEBUG("<%s,%d>V: ParseNumber(%d)\n",__FILE__,__LINE__, node->len);
			if(node != NULL) {
				return node;
			}
			break;
	}
	
	return NULL;
}