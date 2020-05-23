#include <stdio.h>
#include <stdlib.h>
#include "list.h"

upo_List_t upo_list_init() {
    upo_List_t list = malloc(sizeof(upo_List_s));
    if(list == NULL){
        fprintf(stderr, "Unable to create list\n");
        abort();
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

void upo_list_add(upo_List_t l, int d) {
    upo_list_add_impl(&l->head, d);
    l->size++;
}

void upo_list_add_impl(upo_listNode_t *l, int d) {
    upo_listNode_t q, p;

    p = malloc(sizeof(upo_listNode_s));
    p->next = NULL;
    p->data = d;
    
    q = *l;

    if(q == NULL){
        *l = p;
    }
    else{
        while (q->next != NULL){q = q->next;}
        q->next = p;
    }
}

int upo_list_remove_head(upo_List_t l) {
    upo_listNode_t n = l->head;
    int val = n->data;
    l->head = n->next;
    l->size--;
    free(n);
    return val;
}

void upo_list_clear(upo_List_t l) {
    while(l->size > 0){
        upo_list_remove_head(l);
    }
}

void upo_list_destroy(upo_List_t l) {
    upo_list_clear(l);
    free(l);
}

int upo_list_size(upo_List_t l) {
    return l->size;
}