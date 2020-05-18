typedef struct upo_List_s{
    int size;
    struct upo_listNode_s* head;
}upo_List_s;

typedef struct upo_listNode_s
{
    struct upo_listNode_s *next;
    int data;
}upo_listNode_s;

typedef upo_List_s* upo_List_t;
typedef upo_listNode_s* upo_listNode_t;

/**
 * @function    :   inizializza la lista
 * @return      :   puntatore alla lista creata
*/
upo_List_t upo_list_init();

/**
 * @function    :   aggiunge un elemento in fondo alla lista
 * @param       :   la lista
 * @param       :   il dato da aggiungere
*/
void upo_list_add(upo_List_t l, int d);

/**
 * @function    :   rimuove la testa della lista
 * @param       :   la lista
 * @return      :   il valore dell'elemento rimosso
*/
int upo_list_remove_head(upo_List_t l);

/**
 * @function    :   rimuove tutti gli elementi dalla lista
 * @param       :   la lista
*/
void upo_list_clear(upo_List_t l);

/**
 * @function    :   rimuove tutti gli elementi dalla lista e la distrugge
 * @param       :   la lista
*/
void upo_list_destroy(upo_List_t l);

/**
 * @function    :   aggiunge un elemento in coda alla lista
 * @param       :   la lista
 * @param       :   l'elemento da aggiungere
*/
void upo_list_add_impl(upo_listNode_t *l, int d);

/**
 * @function    :   restituisce la grandezza della lista
 * @param       :   la lista
 * @return       :   la grandezza della lista
*/
int upo_list_size(upo_List_t l);