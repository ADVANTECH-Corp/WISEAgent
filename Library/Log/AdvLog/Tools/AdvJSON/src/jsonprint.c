#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "jsonprint.h"
#include "platform.h"

#define PPADING "    "

int PrintVoid(char* buffer, int size, JSONode *json) {
	int len = 0;
	////ADV_DEBUG("<%s,%d>json->s = %p\n",__FILE__,__LINE__, json->s);
	if(size > 5) {
		snprintf(buffer,5,"void");
		len = 4;
	} else {
		snprintf(buffer,size,"void");
		len = size;
	}
	////ADV_DEBUG("<%s,%d>len = %d\n",__FILE__,__LINE__,len);
	return len;
}

int PrintString(char* buffer, int size, JSONode *json) {
	int len = 0;
	if(size > json->len+3) {
		snprintf(buffer,json->len+3,"\"%.*s\"",json->len, json->s);
		len = json->len+2;
	} else {
		snprintf(buffer,size,"\"%.*s\"",json->len,json->s);
		len = size;
	}
	////ADV_DEBUG("<%s,%d>len = %d\n",__FILE__,__LINE__,len);
	return len;
}


int PrintNumber(char* buffer, int size, JSONode *json) {
	int len = 0;
	if(size > json->len+1) {
		snprintf(buffer,json->len+1,"%.*s",json->len,json->s);
		len = json->len;
	} else {
		snprintf(buffer,size,"%.*s",json->len,json->s);
		len = size;
	}
	return len;
}


int PrintValue(char* buffer, int size, JSONode *json, int depth) {
	int len = 0;
	//ADV_DEBUG("<print value>json->type = %d\n", json->type);
	switch(json->type) {
		case JSON_TYPE_STRING:
			len = PrintString(buffer, size, json);
		break;
		case JSON_TYPE_NUMBER:
			len = PrintNumber(buffer, size, json);
		break;
		case JSON_TYPE_OBJECT:
			/*if(depth >= 0) {
				sprintf(buffer,"\n");
				len = PrintObject(buffer+1, size-1, json, depth) + 1;
			} else */{
				len = PrintObject(buffer, size, json, depth);
			}
		break;
		case JSON_TYPE_ARRAY:
			/*if(depth >= 0) {
				sprintf(buffer,"\n");
				len = PrintArray(buffer+1, size-1, json, depth) + 1;
			} else */{
				len = PrintArray(buffer, size, json, depth);
			}
		break;
		case JSON_TYPE_VALUE:
		case JSON_TYPE_BOOL:
			if(size > json->len+1) {
				snprintf(buffer,json->len+1,"%.*s",json->len,json->s);
				len = json->len;
			} else {
				snprintf(buffer,size,"%.*s",json->len,json->s);
				len = size;
			}	
		break;
		case JSON_TYPE_VOID:
			PrintVoid(buffer, size, json);
		default:
		break;
	}
	
	return len;
}
int PrintObject(char* buffer, int size, JSONode *json, int depth) {
	JSONode *head = json->next;
	//ADV_DEBUG("PrintObject head = %p, json->root = %p\n", head, json->root);
	char *pos = buffer;
	int total = 1;
	int len = 0;
	int i = 0;
	if(depth >= 0) {
		/*for(i = 0 ; i < depth ; i++) {
			len = sprintf(pos, PPADING);
			pos += len;
			total += len;
		}*/
		depth++;
	}
	pos += sprintf(pos,"{");
	total += 1;
	if(depth >= 0) {
		pos += sprintf(pos,"\n");
		total += 1;
		for(i = 0 ; i < depth ; i++) {
			len = sprintf(pos, PPADING);
			pos += len;
			total += len;
		}
	}
	////ADV_DEBUG("<%s,%d>json = %p, head = %p\n",__FILE__,__LINE__, json, head);
	////ADV_DEBUG("<%s,%d>pos = %p, buffer = %p\n",__FILE__,__LINE__, pos, buffer);
	////ADV_DEBUG("<%s,%d>head = %p, type = %d\n",__FILE__,__LINE__, head, head->type);
	if(head != NULL) {
		if(head->type != JSON_TYPE_VOID) {
			len = PrintString(pos, size-total, head->key);
			//ADV_DEBUG("<%s,%d>pos = %s\n",__FILE__,__LINE__, pos);
			pos += len;
			total += len;
			////ADV_DEBUG("<%s,%d>pos = %p, buffer = %p\n",__FILE__,__LINE__, pos, buffer);
			pos += sprintf(pos,":");
			total += 1;
			if(depth >= 0) {
				pos += sprintf(pos,"  ");
				total += 2;
			}
			
			////ADV_DEBUG("<%s,%d>buffer = %s\n",__FILE__,__LINE__, buffer);
			////ADV_DEBUG("<%s,%d>PrintValue\n",__FILE__,__LINE__);
			//ADV_DEBUG("<%s,%d>head->value = %p, head->value->type = %d\n",__FILE__,__LINE__, head->value, head->value->type);
			len = PrintValue(pos, size-total, head->value,depth);
			pos += len;
			total += len;
			////ADV_DEBUG("<%s,%d>buffer = %s\n",__FILE__,__LINE__, buffer);
			head = head->next;
				
			while(head != NULL) {
				pos += sprintf(pos,",");
				total += 1;
				if(depth >= 0) {
					pos += sprintf(pos,"\n");
					total += 1;
					for(i = 0 ; i < depth ; i++) {
						len = sprintf(pos, PPADING);
						pos += len;
						total += len;
					}
				}
			
				////ADV_DEBUG("<%s,%d>PrintString\n",__FILE__,__LINE__);
				len = PrintString(pos, size-total, head->key);
				////ADV_DEBUG("<%s,%d>len = %d\n",__FILE__,__LINE__, len);
				pos += len;
				total += len;
				////ADV_DEBUG("<%s,%d>pos = %p, buffer = %p\n",__FILE__,__LINE__, pos, buffer);
				pos += sprintf(pos,":");
				total += 1;
				if(depth >= 0) {
					pos += sprintf(pos,"  ");
					total += 2;
				}
				////ADV_DEBUG("<%s,%d>buffer = %s\n",__FILE__,__LINE__, buffer);
				////ADV_DEBUG("<%s,%d>PrintValue\n",__FILE__,__LINE__);
				len = PrintValue(pos, size-total, head->value,depth);
				pos += len;
				total += len;
				////ADV_DEBUG("<%s,%d>buffer = %s\n",__FILE__,__LINE__, buffer);

				head = head->next;
				
				////ADV_DEBUG("<%s,%d>head = %p\n",__FILE__,__LINE__, head);
			}
		}
	}
	if(depth >= 0) {
		pos += sprintf(pos,"\n");
		total += 1;
		for(i = 0 ; i < depth-1 ; i++) {
			len = sprintf(pos, PPADING);
			pos += len;
			total += len;
		}
	}
	pos += sprintf(pos,"}");
	total += 1;
	////ADV_DEBUG("PrintObject buffer = %s\n", buffer);
	return (int)((long)pos - (long)buffer);
}

int PrintArray(char* buffer, int size, JSONode *json, int depth) {
	JSONode *root = json;
	//ADV_DEBUG("root->next = %p, root->prev = %p, root->array = %p, root->root = %p, root->key = %p, root->value = %p\n",root->next, root->prev, root->array, root->root, root->key, root->value);
	
	JSONode *head = json->array;
	char *pos = buffer;
	int total = 1;
	int len = 0;
	int i = 0;
	if(depth >= 0) {
		/*for(i = 0 ; i < depth ; i++) {
			len = sprintf(pos, PPADING);
			pos += len;
			total += len;
		}*/
		depth++;
	}
	pos += sprintf(pos,"[");
	total += 1;
	if(depth >= 0) {
		pos += sprintf(pos,"\n");
		total += 1;
		for(i = 0 ; i < depth ; i++) {
			len = sprintf(pos, PPADING);
			pos += len;
			total += len;
		}
	}
	//ADV_DEBUG("<array> head = %p\n", head);
	if(head != NULL) {
		if(head->type != JSON_TYPE_VOID) {
			len = PrintValue(pos, size-total, head->value,depth);
			pos += len;
			total += len;
			
			head = head->array;
			while(head != NULL) {
				//ADV_DEBUG("<array> head = %p\n", head);
				pos += sprintf(pos,",");
				total += 1;
				if(depth >= 0) {
					pos += sprintf(pos,"\n");
					total += 1;
					for(i = 0 ; i < depth ; i++) {
						len = sprintf(pos, PPADING);
						pos += len;
						total += len;
					}
				}
				
				len = PrintValue(pos, size-total, head->value,depth);
				pos += len;
				total += len;
				
				head = head->array;
			}
		}
	}
	if(depth >= 0) {
		pos += sprintf(pos,"\n");
		total += 1;
		for(i = 0 ; i < depth-1 ; i++) {
			len = sprintf(pos, PPADING);
			pos += len;
			total += len;
		}
	}
	pos += sprintf(pos,"]");
	total += 1;
	return (int)((long)pos - (long)buffer);
}


//Print Link///////////////////////////////////////////////////////////////

int PLVoid(JSONode *json, int depth) {
	int i = 0;
	for(i = 0 ; i < depth ; i++) {
		printf(PPADING);
	}
	printf("(VOID)[%p]  @%p\n",json, json->collection);
	return depth;
}

int PLString(JSONode *json, int depth) {
	int i = 0;
	for(i = 0 ; i < depth ; i++) {
		printf(PPADING);
	}
	printf("(STRING)[%p]{%.*s}  @%p\n",json, json->len, json->s, json->collection);
	return depth;
}


int PLNumber(JSONode *json, int depth) {
	int i = 0;
	for(i = 0 ; i < depth ; i++) {
		printf(PPADING);
	}
	printf("(NUMBER)[%p]{%.*s}  @%p\n",json,json->len,json->s, json->collection);
	return depth;
}


int PLValue(JSONode *json, int depth) {
	int i = 0;
	switch(json->type) {
		case JSON_TYPE_STRING:
			PLString(json, depth);
		break;
		case JSON_TYPE_NUMBER:
			PLNumber(json, depth);
		break;
		case JSON_TYPE_OBJECT:
			PLObject(json, depth);
		break;
		case JSON_TYPE_ARRAY:
			PLArray(json, depth);
		break;
		case JSON_TYPE_VALUE:
			
			for(i = 0 ; i < depth ; i++) {
				printf(PPADING);
			}
			printf("(VALUE)[%p]{%.*s}  @%p\n",json, json->len, json->s, json->collection);
			break;
		case JSON_TYPE_BOOL:
			for(i = 0 ; i < depth ; i++) {
				printf(PPADING);
			}
			printf("(BOOL)[%p]{%.*s}  @%p\n",json, json->len, json->s, json->collection);
			break;
		case JSON_TYPE_VOID:
			PLVoid(json, depth);
		default:
		break;
	}
	
	return depth;
}
int PLObject(JSONode *json, int depth) {
	int i = 0;
	for(i = 0 ; i < depth ; i++) {
		printf(PPADING);
	}
	printf("(OBJ)[%p]  @%p\n",json, json->collection);
	depth++;
	
	JSONode *head = json->next;
	if(head != NULL) {
		if(head->type == JSON_TYPE_VOID) {
			PLVoid(head, depth);
		} else {
			for(i = 0 ; i < depth ; i++) {
				printf(PPADING);
			}
			printf("(PAIR)[%p]  @%p\n",head, json->collection);
			depth++;
			
			printf("K= ");
			PLString(head->key, depth);
			printf("V= ");
			PLValue(head->value, depth);
			printf("\n");
			head = head->next;
				
			while(head != NULL) {
				depth--;
				for(i = 0 ; i < depth ; i++) {
					printf(PPADING);
				}
				printf("(PAIR)[%p]  @%p\n",head, json->collection);
				depth++;
				printf("K= ");
				PLString(head->key, depth);
				printf("V= ");
				PLValue(head->value, depth);
				printf("\n");
				head = head->next;
			}
		}
	}
	return depth;
}

int PLArray(JSONode *json, int depth) {
	int i = 0;
	for(i = 0 ; i < depth ; i++) {
		printf(PPADING);
	}
	printf("(ARRAY)[%p]  @%p\n",json, json->collection);
	depth++;
	
	JSONode *head = json->array;
	if(head != NULL) {
		
		for(i = 0 ; i < depth ; i++) {
			printf(PPADING);
		}
		printf("(ELEMENT)[%p]  @%p\n",head, json->collection);
		depth++;
			
			
		printf("@= [%p]",head->value);
		PLValue(head->value, depth);

		head = head->array;
		while(head != NULL) {
			depth--;
			for(i = 0 ; i < depth ; i++) {
				printf(PPADING);
			}
			printf("(ELEMENT)[%p]  @%p\n",head, json->collection);
			depth++;
			printf("@= [%p]",head->value);
			PLValue(head->value, depth);
			head = head->array;
		}
	}
	return depth;
}