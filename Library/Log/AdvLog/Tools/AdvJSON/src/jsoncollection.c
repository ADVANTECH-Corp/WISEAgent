#include <stdio.h>
#include <stdlib.h>
#include "jsontype.h"
#include "jsoncollection.h"


JSONCollect *gc;
JSONCollect *spare;

void GC_Init() {
	gc = (JSONCollect *)malloc(sizeof(JSONCollect));
	gc->tail = NULL;
	gc->count = 0;
	
	spare = (JSONCollect *)malloc(sizeof(JSONCollect));
	spare->tail = NULL;
	spare->count = 0;
}

void GC_Release() {
	JSONGarbage *tail = NULL;
	JSONGarbage *prev;
	JSONode *node;
	if(gc == NULL)
		return;
	tail = gc->tail;
	while(tail != NULL) {
		prev = tail->prev;
		node = tail->garbage;
		if(node->alloc != 0) {
			free(node->s);
		}
		ClearItem(node);
		node->collection = NULL;
		free(node);
		free(tail);
		tail = prev;
	}

	free(gc);
	gc = NULL;
	
	tail = spare->tail;
	while(tail != NULL) {
		prev = tail->prev;
		node = tail->garbage;
		if(node->alloc != 0) {
			free(node->s);
		}
		ClearItem(node);
		node->collection = NULL;
		free(node);
		free(tail);
		tail = prev;
	}

	free(spare);
	spare = NULL;
}

JSONode *GC_NewItem() {
	JSONode *node = NULL;
	if(spare->tail == NULL) {
		node = GetNewItem(gc);
	} else {
		JSONGarbage *g = spare->tail;
		JSONGarbage *prev = g->prev;
		node = g->garbage;
		//unlink
		spare->tail = prev;
		if(prev != NULL) {
			prev->next = NULL;
		}
		//initial
		ClearItem(node);
		node->collection = gc;
		
		//link
		if(gc->tail == NULL) {
			gc->count = 1;
			gc->tail = g;
			g->prev = NULL;
			g->next = NULL;
			g->garbage = node;
			node->manager = g;
		} else {
			gc->count++;
			g->prev = gc->tail;
			g->next = NULL;
			g->prev->next = g;
			gc->tail = g;
			g->garbage = node;
			node->manager = g;
		}
	}
	return node;
}

void GC_DestroyItem(JSONode *node) {
	if(node == NULL || gc->tail == NULL) return;
	
	JSONGarbage *g = (JSONGarbage *)node->manager;
	JSONGarbage *prev = g->prev;
	JSONGarbage *next = g->next;
	if(g == gc->tail) {
		gc->tail = prev;
		if(prev != NULL) {
			prev->next = NULL;
		}
	} else {
		next->prev = prev;
		if(prev != NULL) {
			prev->next = next;
		}
	}
	
	ClearItem(node);
	node->collection = spare;
	
	if(spare->tail == NULL) {
		spare->count = 1;
		spare->tail = g;
		g->prev = NULL;
		g->next = NULL;
		g->garbage = node;
		node->manager = g;
	} else {
		spare->count++;
		g->prev = spare->tail;
		g->next = NULL;
		g->prev->next = g;
		spare->tail = g;
		g->garbage = node;
		node->manager = g;
	}
	
	return;
}

JSONCollect *NewCollection() {
	JSONCollect *c = (JSONCollect *)malloc(sizeof(JSONCollect));
	c->tail = NULL;
	c->count = 0;
	return c;
}


JSONode *GetNewItem(JSONCollect* collect) {
	JSONode *node = NULL;
	if(spare->tail == NULL) {
		//printf("@@@@NEW\n");
		node = (JSONode *)malloc(sizeof(JSONode));
		node->s = NULL;
		node->alloc = 0;
		ClearItem(node);
		node->collection = collect;
			
		if(collect->tail == NULL) {
			collect->count = 1;
			JSONGarbage *g = (JSONGarbage *)malloc(sizeof(JSONGarbage));
			collect->tail = g;
			g->prev = NULL;
			g->next = NULL;
			g->garbage = node;
			node->manager = g;
		} else {
			collect->count++;
			JSONGarbage *g = (JSONGarbage *)malloc(sizeof(JSONGarbage));
			g->prev = collect->tail;
			g->next = NULL;
			g->prev->next = g;
			collect->tail = g;
			g->garbage = node;
			node->manager = g;
		}
		
	} else {
		//printf("@@@@SPARE\n");
		JSONGarbage *g = spare->tail;
		JSONGarbage *prev = g->prev;
		node = g->garbage;
		//unlink
		spare->tail = prev;
		if(prev != NULL) {
			prev->next = NULL;
		}
		//initial
		ClearItem(node);
		node->collection = collect;
		
		//link
		if(collect->tail == NULL) {
			collect->count = 1;
			collect->tail = g;
			g->prev = NULL;
			g->next = NULL;
			g->garbage = node;
			node->manager = g;
		} else {
			collect->count++;
			g->prev = collect->tail;
			g->next = NULL;
			g->prev->next = g;
			collect->tail = g;
			g->garbage = node;
			node->manager = g;
		}
	}

	return node;
}

void DestroyItem(JSONCollect* collect, JSONode *node) {
	if(node == NULL || collect->tail == NULL) return;
	
	JSONGarbage *g = (JSONGarbage *)node->manager;
	JSONGarbage *prev = g->prev;
	JSONGarbage *next = g->next;
	if(g == collect->tail) {
		collect->tail = prev;
		if(prev != NULL) {
			prev->next = NULL;
		}
	} else {
		next->prev = prev;
		if(prev != NULL) {
			prev->next = next;
		}
	}
	
	ClearItem(node);
	node->collection = spare;
	
	if(spare->tail == NULL) {
		spare->count = 1;
		spare->tail = g;
		g->prev = NULL;
		g->next = NULL;
		g->garbage = node;
		node->manager = g;
	} else {
		spare->count++;
		g->prev = spare->tail;
		g->next = NULL;
		g->prev->next = g;
		spare->tail = g;
		g->garbage = node;
		node->manager = g;
	}
}


JSONode *ClearItem(JSONode *node) {
	if(node == NULL) return NULL;
	/*if(node->alloc != 0) {
		node->s[0] = 0;
	}*/
	node->type = JSON_TYPE_INVAILD;
	node->len = 0;
	node->array = NULL;
	node->next = NULL;
	node->key = NULL;
	node->value = NULL;
	node->root = NULL;
	node->prev = NULL;
	node->manager = NULL;
	return node;
}

void ReleaseCollection(JSONCollect** collect) {
	JSONGarbage *g;
	JSONGarbage *tail = (*collect)->tail;
	JSONGarbage *prev;
	JSONode *node;
	while(tail != NULL) {
		prev = tail->prev;
		node = tail->garbage;
		/*******/
		if(spare != NULL) {
			g = tail;
			ClearItem(node);
			node->collection = spare;
			
			if(spare->tail == NULL) {
				spare->count = 1;
				spare->tail = g;
				g->prev = NULL;
				g->next = NULL;
				g->garbage = node;
				node->manager = g;
			} else {
				spare->count++;
				g->prev = spare->tail;
				g->next = NULL;
				g->prev->next = g;
				spare->tail = g;
				g->garbage = node;
				node->manager = g;
			}
		} else {
			ClearItem(node);
			node->collection = NULL;
			free(node);
			free(tail);
		}
		/*******/
		tail = prev;
	}

	free(*collect);
	*collect = NULL;
}

void LinkItem(JSONCollect* collect, JSONode *node) {
	JSONGarbage *g;
	if(collect->tail == NULL) {
		collect->count = 1;
		g = (JSONGarbage *)node->manager;
		collect->tail = g;
		g->prev = NULL;
		g->next = NULL;
		g->garbage = node;
		node->manager = g;
	} else {
		collect->count++;
		g = (JSONGarbage *)node->manager;
		g->prev = collect->tail;
		g->next = NULL;
		g->prev->next = g;
		collect->tail = g;
		g->garbage = node;
		node->manager = g;
	}
	node->collection = collect;
}

void UnlinkItem(JSONCollect* collect, JSONode *node) {
	if(node == NULL || collect->tail == NULL) return;
	JSONGarbage *tail = (JSONGarbage *)node->manager;
	JSONGarbage *prev = tail->prev;
	JSONGarbage *next = tail->next;
	if(tail == collect->tail) {
		collect->tail = prev;
		if(prev != NULL) {
			prev->next = NULL;
		}
	} else {
		next->prev = prev;
		if(prev != NULL) {
			prev->next = next;
		}
	}
	return;
}
