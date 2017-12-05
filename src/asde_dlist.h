#ifndef ASDE_DLIST_H
#define ASDE_DLIST_H
typedef int data_type;
typedef struct DList *DList;

// allocates space for one DList element -> fonction creerCellule(): liste d'objet;
extern DList asde_dlist_alloc(void);

// frees space for one DList element -> fonction libererCellule(ref L:liste d'objet): vide;
extern  void asde_dlist_free_link(DList link_); 

//adds a new element on the start of the list -> fonction insereEnTete(val L: liste d'objet, val x objet): liste d'objet;
extern DList asde_dlist_prepend(DList L, data_type data);

// -> fonction supprimerEnTete(val L: liste d'objet): liste d'objet;
extern DList asde_dlist_delete_first(DList L);

// -> fonction insererApres(val L:liste d'objets, val p : liste d'objets, val x: objet): liste d'objet;
extern DList asde_dlist_insert_after(DList L, DList p, data_type data);

// -> fonction supprimerApres(val L:liste d'objets, val p : liste d'objets): liste d'objet;
extern DList asde_dlist_delete_after(DList L, DList p);

// -> fonction insererAvant(val L:liste d'objets, val p : liste d'objets, val x: objet): liste d'objet;
extern DList asde_dlist_insert_before(DList L, DList p, data_type data);

// -> fonction supprimerAvant(val L:liste d'objets, val p : liste d'objets): liste d'objet;
extern DList asde_dlist_delete_before(DList L, DList p);

// gets next element in a DList ->  fonction suivant(val L:liste d'objet): liste d'objet; 
extern  DList  asde_dlist_next(DList L);

// gets previous element in a DList ->  fonction precedent(val L:liste d'objet): liste d'objet; 
extern  DList  asde_dlist_prev(DList L);

// gets data in a DList ->  fonction valeur(val L:liste d'objet): objet;
extern  data_type asde_dlist_data(DList L);

#endif /* ASDE_DLIST_H */ 