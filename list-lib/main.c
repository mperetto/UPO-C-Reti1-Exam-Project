#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

int main(int argc, char const *argv[])
{
    printf("Test init\n");
    fflush(stdout);

    printf("Test Case: create list\n");
    fflush(stdout);

    upo_List_t lista = upo_list_init();
    assert(lista != NULL);

    printf("OK Test Case: create list\n");
    fflush(stdout);

    printf("Test Case: list size\n");
    fflush(stdout);

    assert(upo_list_size(lista) == 0);

    printf("OK Test Case: list size\n");
    fflush(stdout);

    printf("Test Case: list add item\n");
    fflush(stdout);

    int vet[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    for(int i = 0; i < 10; i++){
        upo_list_add(lista, vet[i]);
    }
    assert(upo_list_size(lista) == 10);

    for(int i = 0; i < 10; i++){
        assert(upo_list_remove_head(lista) == vet[i]);
    }
    assert(upo_list_size(lista) == 0);
    printf("OK Test Case: list add item\n");
    fflush(stdout);

    printf("Test Case: list clear\n");
    fflush(stdout);

    for(int i = 0; i < 10; i++){
        upo_list_add(lista, vet[i]);
    }
    assert(upo_list_size(lista) == 10);

    upo_list_clear(lista);
    assert(upo_list_size(lista) == 0);

    printf("OK Test Case: list clear\n");
    fflush(stdout);
    
    return 0;
}
