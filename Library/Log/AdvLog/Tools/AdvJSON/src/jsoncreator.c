#include <string.h>
#include "jsontool.h"
#include "jsoncreator.h"

JSONode *CreateVoid(JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode* node = GetNewItem(collect);
	node->type = JSON_TYPE_VOID;
	node->root = root;
	node->prev = prev;
	return node;
}

JSONode *CreateString(const char *name, JSONCollect* collect, JSONode *root, JSONode *prev) {
	int len = strlen(name);
	JSONode* key = GetNewItem(collect);
	key->type = JSON_TYPE_STRING;
	key->s = jmalloc(key->s, &key->alloc, len + 1);
	memcpy(key->s,name,len);
	key->s[len] = 0;
	key->len = len;
	key->root = root;
	key->prev = prev;
	return key;
}

JSONode *CreateNullValue(JSONCollect* collect, JSONode *root, JSONode *prev) {
	//ADV_DEBUG("CreateNullValue");
	JSONode* value = GetNewItem(collect);
	value->type = JSON_TYPE_VALUE;
	value->s = jmalloc(value->s, &value->alloc, 5);
	memcpy(value->s,"null",4);
	value->s[4] = 0;
	value->len = 4;
	value->root = root;
	value->prev = prev;
	return value;
}

JSONode *CreatePair(const char *name, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = GetNewItem(collect);
	node->type = JSON_TYPE_PAIR;
	node->key = CreateString(name, collect, node, node);
	node->value = CreateNullValue(collect, node, node);
	node->root = root;
	node->prev = prev;
	return node;
}

JSONode *CreateObject(const char *name, JSONCollect* collect, JSONode *root, JSONode *prev, JSONode *old) {
	JSONode *node;
	if(old == NULL) {
		node = GetNewItem(collect);
	} else {
		node = ClearItem(old);
	}
	node->type = JSON_TYPE_OBJECT;
	node->next = CreatePair(name, collect, node, node);
	node->root = root;
	node->prev = prev;
	return node;
}

JSONode *CreateElement(JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = GetNewItem(collect);
	node->type = JSON_TYPE_ELEMENT;
	node->value = CreateNullValue(collect, node, node);
	node->root = root;
	node->prev = prev;
	return node;
}

JSONode *CreateArray(JSONCollect* collect, JSONode *root, JSONode *prev, JSONode *old) {
	JSONode *node;
	if(old == NULL) {
		node = GetNewItem(collect);
	} else {
		node = ClearItem(old);
	}
	node->type = JSON_TYPE_ARRAY;
	node->array = CreateElement(collect, node, node);
	node->root = root;
	node->prev = prev;
	return node;
}

///DELETE///
void DeletePair(JSONode *del) ;
void DeleteElement(JSONode *del);
void DeleteValue(JSONode *del);

void DeleteVoid(JSONode *del) {
	//ADV_DEBUG("<DeleteVoid> del = %p\n", del);
	if(del == NULL) return;
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeleteKey(JSONode *del) {
	//ADV_DEBUG("<DeleteString> del = %p\n", del);
	if(del == NULL) return;
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeleteString(JSONode *del) {
	//ADV_DEBUG("<DeleteString> del = %p\n", del);
	if(del == NULL) return;
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeleteBool(JSONode *del) {
	if(del == NULL) return;
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeleteNumber(JSONode *del) {
	if(del == NULL) return;
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeletePair(JSONode *del) {
	if(del == NULL) return;
	if(del->root == del->prev) {
		if(del->root != NULL) {
			del->root->next = del->next;
		}
		if(del->next != NULL) {
			del->next->prev = del->prev;
		}
	} else {
		if(del->prev != NULL) {
			del->prev->next = del->next;
		}
		if(del->next != NULL) {
			del->next->prev = del->prev;
		}
	}

	//ADV_DEBUG("<DeleteKey> del->key = %p\n", del->key);
	DeleteKey(del->key);
	del->key = NULL;
	//ADV_DEBUG("<DeleteKey> del->value = %p\n", del->value);
	if(del->value != NULL) {
		DeleteValue(del->value);
		del->value = NULL;
	}
	//ADV_DEBUG("<DeletePair> del = %p\n", del);
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeleteObject(JSONode *del) {
	if(del == NULL) return;
	JSONode *next = del->next;
	JSONode *temp;

	while(next != NULL) {
		temp = next->next;
		DeletePair(next);
		next = temp;
	}
	//ADV_DEBUG("<DeleteObject> del = %p\n", del);
	DestroyItem((JSONCollect *)del->collection, del);
}


void DeleteElement(JSONode *del) {
	if(del == NULL) return;
	//ADV_DEBUG("del->root = %p, del->prev = %p\n", del->root, del->prev);
	if(del->root == del->prev) {
		if(del->root != NULL) {
			del->root->array = del->array;
		}
		if(del->array != NULL) {
			del->array->prev = del->prev;
		}
	} else {
		if(del->prev != NULL) {
			del->prev->array = del->array;
		}
		if(del->array != NULL) {
			del->array->prev = del->prev;
		}
	}
	
	if(del->value != NULL) {
		DeleteValue(del->value);
		del->value = NULL;
	}
	//ADV_DEBUG("<DeleteElement> del = %p\n", del);
	DestroyItem((JSONCollect *)del->collection, del);
}
void DeleteArray(JSONode *del) {
	if(del == NULL) return;
	JSONode *array = del->array;
	JSONode *next;
	
	while(array != NULL) { 
		next = array->array;
		DeleteElement(array);
		array = next;
	}
	DestroyItem((JSONCollect *)del->collection, del);
}

void DeleteValue(JSONode *del) {
	if(del == NULL) return;
	//ADV_DEBUG("<DeleteValue> del = %p\n", del);
	switch(del->type) {
		case JSON_TYPE_STRING:
			//ADV_DEBUG("<DeleteValue><String>\n");
			DeleteString(del);
			break;
		case JSON_TYPE_NUMBER:
			DeleteNumber(del);
			break;
		case JSON_TYPE_OBJECT:
			DeleteObject(del);
			break;
		case JSON_TYPE_PAIR:
			DeletePair(del);
			break;
		case JSON_TYPE_ARRAY:
			DeleteArray(del);
			break;
		case JSON_TYPE_ELEMENT:
			DeleteElement(del);
			break;
		case JSON_TYPE_BOOL:
			DeleteBool(del);
			break;
		case JSON_TYPE_VALUE:
			if(del->root != NULL) {
				switch(del->root->type) {
					case JSON_TYPE_PAIR:
						del->root->value = NULL;
						DeletePair(del->root);
						del->root = NULL;
					break;
					case JSON_TYPE_ARRAY:
						if(del->root == del->prev) {
							del->root->array = del->array;
						} else {
							del->prev->array = del->array;
						}
					break;
				}
			}
			DestroyItem((JSONCollect *)del->collection, del);
			break;
		case JSON_TYPE_VOID:
		default:
			DeleteVoid(del);
		break;
	}
}

void DeleteNode(JSONode *del) {
	JSONode *root = del->root;
	DeleteValue(del);
	if(root != NULL) {
		switch(root->type) {
			case JSON_TYPE_PAIR:
				root->value = NULL;
				DeletePair(root);
			break;
			case JSON_TYPE_ELEMENT:
				root->value = NULL;
				DeleteElement(root);
			break;
		}
	}
	
}

/*************************************************************************************/
//Copy///////////////////////////////////////////////////////////////

JSONode *CopyValue(JSONode *json, JSONCollect* collect, JSONode *root, JSONode *prev);

JSONode *CopyPair(JSONode *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = GetNewItem(collect);
	node->type = JSON_TYPE_PAIR;
	node->key = CopyValue(json->key, collect, node, node);
	node->value = CopyValue(json->value, collect, node, node);
	node->root = root;
	node->prev = prev;
	return node;
}

JSONode *CopyObject(JSONode *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = GetNewItem(collect);
	JSONode *head = json;
	JSONode *next = node;
	node->type = JSON_TYPE_OBJECT;
	node->root = root;
	node->prev = prev;
	
	if(head->next != NULL) {
		node->next = CopyPair(head->next, collect, node, next);
		head = head->next;
		next = next->next;
		while(head->next != NULL) {
			next->next = CopyPair(head->next, collect, node, next);
			head = head->next;
			next = next->next;
		}
	}
	return node;
}

JSONode *CopyElement(JSONode *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = GetNewItem(collect);
	node->type = JSON_TYPE_ELEMENT;
	//node->key = CopyValue(json->key, collect, node, node);
	node->value = CopyValue(json->value, collect, node, node);
	node->root = root;
	node->prev = prev;
	return node;
}

JSONode *CopyArray(JSONode *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode *node = GetNewItem(collect);
	JSONode *head = json;
	JSONode *array = node;
	node->type = JSON_TYPE_ARRAY;
	node->root = root;
	node->prev = prev;
	if(head->array != NULL) {
		node->array = CopyElement(head->array, collect, node, array);
		head = head->array;
		array = array->array;
		while(head->array != NULL) {
			array->array = CopyElement(head->array, collect, node, array);
			head = head->array;
			array = array->array;
		}
	}
	return node;
}

JSONode *CopyValue(JSONode *json, JSONCollect* collect, JSONode *root, JSONode *prev) {
	JSONode* node = NULL;
	switch(json->type) {
		case JSON_TYPE_OBJECT:
			node = CopyObject(json, collect, root, prev);
		break;
		case JSON_TYPE_ARRAY:
			node = CopyArray(json, collect, root, prev);
		break;
		case JSON_TYPE_STRING:
		case JSON_TYPE_NUMBER:
		case JSON_TYPE_VALUE:
		case JSON_TYPE_BOOL:
		case JSON_TYPE_VOID:
			node = GetNewItem(collect);
			node->type = json->type;
			node->s = jmalloc(node->s, &node->alloc, json->alloc);
			strcpy(node->s,json->s);
			node->len = json->len;
			node->root = root;
			node->prev = prev;
			return node;
		default:
		break;
	}
	
	return node;
}

JSONode *CopyNode(JSONode *node, JSONCollect* collect) {
	if(collect == NULL) collect = NewCollection();
	return CopyValue(node, collect, NULL, NULL);
}
