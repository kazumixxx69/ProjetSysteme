#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "asde_dlist.h"

struct DList{
  data_type data;
  struct DList* next;
  struct DList* previous;
};


DList 
asde_dlist_alloc(void){
  DList p;
  assert(p = malloc(sizeof(struct DList)));
  p->next = NULL;
  p->previous = NULL;
  return p;
}

DList 
asde_dlist_prepend(DList L, data_type data){
	DList p = asde_dlist_alloc();
	p->data = data;
	p->next = L;
	p->previous = NULL;
	if (L!=NULL){
		L->previous = p;
	}
	return p;
}

DList 
asde_dlist_delete_first(DList L){
	if (L==NULL){return NULL;}	
	DList p = L->next;	
	(L->next)->previous = NULL;
	L->next = NULL;
	asde_dlist_free_link(L);
	L=NULL;
	return p;
}

DList 
asde_dlist_insert_after(DList L, DList p, data_type data){
	DList new = asde_dlist_alloc();
	new->data = data;
	new->previous = p;
	new->next = p->next;
	(p->next)->previous = new;
	p->next = new;
	return L;
}

DList 
asde_dlist_delete_after(DList L, DList p){
	DList next = (p->next)->next;	
	next->previous = p;	
	(p->next)->previous = NULL;
	(p->next)->next = NULL;
	asde_dlist_free_link(p->next);
	p->next = next;
	return L;
}

DList 
asde_dlist_insert_before(DList L, DList p, data_type data){
	if (L==p){return asde_dlist_prepend(L, data);}
	DList new = asde_dlist_alloc();
	new->data = data;
	new->previous = p->previous;
	new->next = p;
	(p->previous)->next = new;
	p->previous = new;
	return L;
}

DList 
asde_dlist_delete_before(DList L, DList p){
	if (L==p){return L;}	
	DList previous = (p->previous)->previous;	
	previous->next = p;	
	(p->previous)->next = NULL;
	(p->previous)->previous = NULL;
	asde_dlist_free_link(p->previous);
	p->previous = previous;
	return L;
}


DList  
asde_dlist_next(DList L){
	if(L!=NULL){return L->next;}
	else {return NULL;}
}

DList  
asde_dlist_prev(DList L){
	if(L!=NULL){return L->previous;}
	else {return NULL;}
}

data_type
asde_dlist_data(DList L){
	if(L!=NULL){return L->data;}
	else {
		fprintf(stderr, "No data\n");
		exit(EXIT_FAILURE);
	}
}

void 
asde_dlist_free_link(DList link_){
  if(link_ != NULL)
    free(link_);
}
