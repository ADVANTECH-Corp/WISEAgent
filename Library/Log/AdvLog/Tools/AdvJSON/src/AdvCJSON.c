#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AdvJSON.h"

#include "jsontype.h"
#include "jsoncollection.h"
#include "jsonrule.h"
#include "jsonparser.h"
#include "jsoncreator.h"
#include "jsonprint.h"
#include "jsontool.h"
#include "platform.h"

int JSON_Validator(const char *json) {
	if(json == NULL) return -1;
	int check;
	char *s = strchr((char *)json,'{');
	char *e;
	if(s == NULL) return -1;
	
	check = IsObject(json);
	if(check == 0) return 1;
	
	e = s + check - 1;
	
	//printf("check = %d, e = %c(%d)\n", check , *e, *e);
	if(*e == '}') return 0;
	else return check;
}

int JSON_ShowError(const char *json, int pos) {
	if(pos == 0) return 0;
	char *s = strchr((char *)json,'{');
	char *t = s + pos -1;
	char temp[32];
	int len = strlen(s);
	unsigned int i;
	
	if(pos > 10) {
		s = t-10;
	} 
	
	strncpy(temp,s,31);
	temp[strlen(temp)] = 0;
	for(i = 0 ; i < strlen(temp) ; i++) {
		if(i == 10 || i == 11) {
			if(temp[i] != '\n') {
				//ADV_DEBUG("\033[31m%c\033[0m",temp[i]);
			}
		} else {
			//ADV_DEBUG("%c",temp[i]);
		}
	}
	return 0;
}

JSONode *JSON_Parser(const char *json) {
	if(0 == JSON_Validator(json)) {
		return ParseObject(json, NewCollection(), NULL, NULL);
	} else {
		//ADV_DEBUG("format error\n");
		return NULL;
	}
}

JSONode *JSON_Copy(JSONode *json) {
	if(json == NULL) return NULL;
	return CopyNode(json, NULL);
}

void JSON_Destory(JSONode **json) {
	if(*json != NULL) {
		JSONCollect *collect = (JSONCollect *)(*json)->collection;
		DeleteNode(*json);
		ReleaseCollection(&collect);
		*json = NULL;
	}
}

void JSON_Print(JSONode *json) {
	if(json == NULL) return;
	char temp[4096];
	//ADV_DEBUG("JSON = (%p)\n",json);
	PrintObject(temp, sizeof(temp), json, -1);
	//printf("\033[33m%s\033[0m\n",temp);
}
void JSON_PrintLink(JSONode *json) {
	if(json == NULL) return;
	PLObject(json, 0);
}
void JSON_ReleaseCollection(JSONode *node) {
	if(node == NULL) return;
	JSONCollect *collect = (JSONCollect *)node->collection;
	ReleaseCollection((JSONCollect **)&collect);
}






/////////////////////////////////////////////////////////
static int isInteger(char *str, int len) {
	int i = 0;
	for(i = 0 ; i < len ; i++) {
		if(!isdigit(str[i])) return 0;
	}
	return 1;
}

static int isFloat(char *str, int len) {
	int i = 0;
	for(i = 0 ; i < len ; i++) {
		if(!isdigit(str[i])) return 0;
	}
	return 1;
}

static JSONode *__JSON_Find_Element(JSONode *node, const char *name) {
	JSONode *find = NULL;
	JSONode *target = NULL;
	int index = 0;
	int count = 0;
	char *key = NULL;
	
	int len = strlen(name);
	
	do {
		if(isInteger((char *)name, len) && node->type == JSON_TYPE_ARRAY) {
			//ADV_DEBUG("array\n");
			index = atoi(name);
			
			find = node->array;
			while(count != index && find != NULL) {
				count++;
				find = find->array;
			}
			if(find == NULL) {
				node = NULL;
				break;
			}
			node = find->value;
		} else {
			//ADV_DEBUG("object\n");
			if(*name == '\"') {
				key = strdup(name+1);
			} else {
				if(key != NULL) {
					free(key);
					key = NULL;
				}
				key = (char *)malloc(len + 1);
				sprintf(key,"%s",name);
			}
			if(node->type == JSON_TYPE_OBJECT) {
				find = node->next;
				target = NULL;
				while(find != NULL) {
					//printf("name = %s, key = %s, find->key->s = %s, find->key->len = %d, len = %d\n", name, key, find->key->s, find->key->len, len);
					if(len == find->key->len && strncmp(key,find->key->s,find->key->len) == 0) {
						//printf("find->value = %p, find->value->type = %d\n", find->value, find->value->type);
						target = find->value;
						break;
					}
					find = find->next;
				}
				if(target == NULL) {
					node = NULL;
					break;
				}
				node = target;
			} else {
				node = NULL;
				break;
			}
		}
	} while(0);
	
	if(key != NULL) {
		free(key);
	}
	
	return node;
}

static JSONode *__JSON_Find_Node(JSONode *json, const char *path) {
	
	int pathlen = strlen(path);
	char *jpath = strdup(path);
	
	int index = 0;
	int count = 0;
	char *saveptr;
	JSONode *node = json;
	
	int len;
	char *pos;
	
	if(*path == '[') {
		pos = strtok_r(jpath,"[]", &saveptr); 
		while(pos != NULL) {
			len = strlen(pos);
			node = __JSON_Find_Element(node, pos);
			if(node == NULL) break;
			pos = strtok_r(NULL,"[]", &saveptr);
		}
		
		free(jpath);
		return node;
	} else if(*path == '/') {
		pos = strtok_r(jpath,"/", &saveptr); 
		while(pos != NULL) {
			len = strlen(pos);
			node = __JSON_Find_Element(node, pos);
			if(node == NULL) break;
			pos = strtok_r(NULL,"/", &saveptr);
		}
		free(jpath);
		return node;
	}
	
	free(jpath);
	return NULL;
}


void JSON_Get(JSONode *json, const char *path, char *result, int size) {
	if(json == NULL) {
		snprintf(result,size,"NULL");
		return;
	}

	JSONode * node = __JSON_Find_Node(json, path);
	if(node == NULL) {
		strncpy(result,"NULL",size);
	} else {
		if(node->s == NULL) {
			strncpy(result,"NULL",size);
		} else {
			if(size > node->len) {
					size = node->len;
			}
			strncpy(result,node->s,size);
		}
	}

	return;
	
}

static JSONode *__JSON_Complete_Array(JSONode *node, int index) {
	int count = 0;
	JSONode *root = NULL;
	JSONode *target = NULL;
	
	switch(node->type) {
		case JSON_TYPE_VALUE:
			root = node->root;
			target = CreateArray((JSONCollect *)node->collection, root, root, node);
			root->value = target;
			//DestroyItem((JSONCollect *)node->collection,node);				
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
	return NULL;
}

static JSONode *__JSON_Complete_Object(JSONode *node, const char *name) {
	JSONode *prev = NULL;
	JSONode *target = NULL;
	char *key = NULL;
	int len = strlen(name);
	if(*name == '\"') {
		key = strdup(name);
	} else {
		key = (char *)malloc(len + 3);
		sprintf(key,"\"%s\"",name);
	}
	
	switch(node->type) {
		case JSON_TYPE_OBJECT:
			target = node->next;
			if(target == NULL) {
				node->next = CreatePair(key, (JSONCollect *)node->collection, node, node);
				target = node->next;
			} else {
				while(target != NULL) {
					//ADV_DEBUG("key = %s\n", key);
					if(strncmp(key,target->key->s,target->key->len) == 0) {
						//ADV_DEBUG("found!!\n");
						free(key);
						return target->value;
					}
					prev = target;
					target = target->next;
				}
				target = prev;
				target->next = CreatePair(key, (JSONCollect *)target->collection, node, target);
				target = target->next;
			}
			free(key);
			return target->value;
		break;
		case JSON_TYPE_VALUE:
			prev = node->root;
			target = CreateObject(key, (JSONCollect *)node->collection, prev, prev, node);
			prev->value = target;
			free(key);
			return target->next->value;
		default:
			free(key);
			return NULL;
	}
	free(key);
	return NULL;
}

static JSONode *__JSON_Complete_Node(JSONode *json, const char *path) {
	int pathlen = strlen(path);
	char *jpath = strdup(path);
	
	int index = 0;
	int count = 0;
	char *saveptr;
	JSONode *prev = NULL;
	JSONode *node = json;
	JSONode *target = NULL;
	int len;
	char *pos;
	
	if(*path == '[') {
		pos = strtok_r(jpath,"[]", &saveptr); 
		while(pos != NULL) {
			len = strlen(pos);
			prev = node;
			node = __JSON_Find_Element(node, pos);
			if(node == NULL) {
				node = prev;
				if(isInteger(pos, strlen(pos))) {
					index = atoi(pos);
					node = __JSON_Complete_Array(prev, index);
				} else {
					node = __JSON_Complete_Object(prev, pos);
				}
			}
			if(node == NULL) break;
			pos = strtok_r(NULL,"[]", &saveptr);
		}
		
		free(jpath);
		return node;
	} else if(*path == '/') {
		pos = strtok_r(jpath,"/", &saveptr); 
		while(pos != NULL) {
			len = strlen(pos);
			node = __JSON_Find_Element(node, pos);
			if(node == NULL) {
				node = prev;
				if(isInteger(pos, strlen(pos))) {
					index = atoi(pos);
					node = __JSON_Complete_Array(prev, index);
				} else {
					node = __JSON_Complete_Object(prev, pos);
				}
			}
			pos = strtok_r(NULL,"/", &saveptr);
		}
		free(jpath);
		return node;
	}
	
	free(jpath);
	return NULL;
}


void JSON_New(JSONode *json, const char *path, char *result, int size) {
	if(json == NULL) {
		return;
	}
	int len;
	JSONode *node = __JSON_Complete_Node(json, path);
	if(result != NULL) {
		switch(node->type) {
			case JSON_TYPE_VALUE:
			case JSON_TYPE_STRING:
				len = strlen(result);
				node->s = jmalloc(node->s, &node->alloc, len + 3);
				node->type = JSON_TYPE_STRING;
				sprintf(node->s,"\"%s\"",result);
				node->len = len + 2;
				return;
			default:
				return;
		}
	}
	
	return;
	
}

void JSON_Edit(JSONode *json, const char *path, char *result, int size) {
	if(json == NULL || result == NULL) {
		return;
	}
	int len;
	JSONode *node = __JSON_Find_Node(json, path);
	if(node != NULL) {
		switch(node->type) {
			case JSON_TYPE_VALUE:
			case JSON_TYPE_STRING:
				len = strlen(result);
				node->s = jmalloc(node->s, &node->alloc, len + 1);
				node->type = JSON_TYPE_STRING;
				sprintf(node->s,"%s",result);
				node->len = len;
				return;
			default:
				return;
		}
	}
	
	return;
	
}

void JSON_Delete(JSONode *json, const char *path) {
	if(json == NULL) {
		return;
	}
	
	JSONode *node = __JSON_Find_Node(json, path);
	JSONode *del = node->root;
	switch(del->type) {
		case JSON_TYPE_PAIR:
		case JSON_TYPE_ELEMENT:
			DeleteNode(del);
			break;
	}
	return;
	
}

//Get		GET
//New		POST
//Edit		PUT
//Delete	DELETE

void JSON_Cmd(JSONode *json, const char *path, char *assign, int size) {
	if(json == NULL) {
		return;
	}

	int len = strlen(path);
	char *pos = NULL;
	int cmpn = 0;
	switch(*path) {
		case 'G':
			cmpn = len < 3 ? len : 3;
			if(strncmp(path,"Get",cmpn) == 0 || strncmp(path,"GET",cmpn) == 0) {
				pos = (char *)path + cmpn + 1;
				JSON_Get(json, pos, assign, size);
			}
		break;
		case 'N':
			cmpn = len < 3 ? len : 3;
			if(strncmp(path,"New",cmpn) == 0) {
				pos = (char *)path + cmpn + 1;
				JSON_New(json, pos, assign, size);
			}
		break;
		case 'E':
			cmpn = len < 4 ? len : 4;
			if(strncmp(path,"Edit",cmpn) == 0) {
				pos = (char *)path + cmpn + 1;
				JSON_Edit(json, pos, assign, size);
			}
		break;
		case 'D':
			cmpn = len < 6 ? len : 6;
			if(strncmp(path,"Delete",cmpn) == 0 || strncmp(path,"DELETE",cmpn) == 0) {
				pos = (char *)path + cmpn + 1;
				JSON_Delete(json, pos);
			}
		break;
		case 'P':
			if(*(path+1) == 'O') {
				cmpn = len < 4 ? len : 4;
				if(strncmp(path,"POST",cmpn) == 0) {
					pos = (char *)path + cmpn + 1;
					JSON_New(json, pos, assign, size);
				}
				break;
			} else if(*(path+1) == 'P') {
				cmpn = len < 3 ? len : 3;
				if(strncmp(path,"PUT",cmpn) == 0) {
					pos = (char *)path + cmpn + 1;
					JSON_Edit(json, pos, assign, size);
				}
				break;
			}
		default:
		break;
	}
}
