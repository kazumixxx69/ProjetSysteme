#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "asde_dlist.h"

struct DList{
  data_type data;
  void* param;
  int id;
  struct DList* next;
  struct DList* previous;
};


DList 
asde_dlist_alloc(void){
  DList p;
  assert(p = malloc(sizeof(struct DList)));
  p->next = NULL;
  p->previous = NULL;
  p->param = NULL;
  return p;
}

DList 
asde_dlist_prepend(DList L, data_type data, void* param, int id){
	DList p = asde_dlist_alloc();
	p->data = data;
	p->param = param;
	p->id = id;
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
	if((L->next) != NULL){	
	  (L->next)->previous = NULL;
	}
	L->next = NULL;
	L->param = NULL;
	asde_dlist_free_link(L);
	L=NULL;
	return p;
}

DList 
asde_dlist_insert_after(DList L, DList p, data_type data, void* param, int id){
	DList new = asde_dlist_alloc();
	new->data = data;
	new->param = param;
	new->id = id;
	new->previous = p;
	new->next = p->next;
	if((p->next) != NULL){
	  (p->next)->previous = new;
	}
	p->next = new;
	return L;
}

DList 
asde_dlist_delete_after(DList L, DList p){
	DList next = (p->next)->next;
	if(next != NULL){	
	  next->previous = p;	
	}
	(p->next)->previous = NULL;
	(p->next)->next = NULL;
	asde_dlist_free_link(p->next);
	p->next = next;
	return L;
}

DList 
asde_dlist_insert_before(DList L, DList p, data_type data, void* param, int id){
	if (L==p){return asde_dlist_prepend(L, data, param, id);}
	DList new = asde_dlist_alloc();
	new->data = data;
	new->param = param;
	new->id = id;
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
asde_dlist_modify_data(DList L, data_type data){
	if(L!=NULL){L->data=data;}
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

void*
asde_dlist_param(DList L){
	if(L!=NULL){return L->param;}
	else {
		fprintf(stderr, "No param\n");
		exit(EXIT_FAILURE);
	}
}

int
asde_dlist_id(DList L){
	if(L!=NULL){return L->id;}
	else {
		fprintf(stderr, "No id\n");
		exit(EXIT_FAILURE);
	}
}

void 
asde_dlist_free_link(DList link_){
  if(link_ != NULL)
    free(link_);
}
