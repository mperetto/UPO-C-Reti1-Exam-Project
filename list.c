#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct upo_List_s{
    int size;
    struct upo_listNode_s* head;
}upo_List_s;

typedef upo_List_s* upo_List_t;

typedef struct upo_listNode_s
{
    struct upo_listNode_s *next;
    int data;
}upo_listNode_s;

typedef upo_listNode_s* upo_listNode_t;

upo_List_t upo_ListInit(){
    upo_List_t list = malloc(sizeof(upo_List_s));
    if(list == NULL){
        fprintf(stderr, "Unable to create list\n");
        abort();
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

int upo_ListRemoveHead(upo_List_t l){
    upo_listNode_t n = l->head;
    int val = n->data;
    l->head = n->next;
    l->size--;
    free(n);
    return val;
}

void upo_ListClear(upo_List_t l){
    while(l->size > 0){
        upo_ListRemoveHead(l);
    }
}

void upo_ListDestroy(upo_List_t l){
    upo_ListClear(l);
    free(l);
}

void upo_ListAddImpl(upo_listNode_t *l, int d){
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

void upo_ListAdd(upo_List_t l, int d){
    upo_ListAddImpl(&l->head, d);
    l->size++;
}

int main(int argc, char const *argv[])
{
    upo_List_t lista = upo_ListInit();
    assert(lista != NULL);
    assert(lista->size == 0);
    upo_listNode_t q = lista->head;
    for(int i = 0; i < 10; i++){
        upo_ListAdd(lista, i);
    }
    assert(lista->size == 10);
    upo_listNode_t n = lista->head;

    for(int i = 0; i < 10; i++){
        assert(i == n->data);
        n = n->next;
    }

    upo_ListClear(lista);

    assert(lista->head == NULL);
    assert(lista->size == 0);
    upo_ListDestroy(lista);
    return 0;
}
