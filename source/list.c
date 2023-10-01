#include "list.h"
#include "list-private.h"
#include "utils.h"

#include <stddef.h>
#include <string.h>

struct node_t* node_create(struct entry_t *entry, struct node_t *next) {
    if (assert_error(
        entry == NULL,
        "node_create",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    struct node_t* node = create_dynamic_memory(sizeof(struct node_t));
    if (assert_error(
        node == NULL,
        "node_create",
        ERROR_MALLOC
    )) return NULL;

    node->entry = entry;
    node->next = next;
    return node;
}

enum MemoryOperationStatus node_destroy(struct node_t *node) {
    if (assert_error(
        node == NULL,
        "node_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;
    
    return entry_destroy(node->entry);
}

struct list_t* list_create() {
    struct list_t* list = create_dynamic_memory(sizeof(struct list_t));
    if (assert_error(
        list == NULL,
        "list_create",
        ERROR_MALLOC
    )) return NULL;

    list->head = NULL;
    list->size = 0;
    return list;
}

int list_destroy(struct list_t *list) {
    if (assert_error(
        list == NULL,
        "list_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    struct node_t* node;
    while ((node = list->head)) {
        if (node_destroy(node) == M_ERROR)
            return M_ERROR;
        list->head = node->next;
    }
    return M_OK;   
}

enum AddOperationStatus insert_node(struct list_t* list, struct node_t* node, struct node_t* node_prev, struct node_t* node_next) {
    if (assert_error(
        list == NULL || node == NULL,
        "insert_node",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    node->next = node_next;
    if (node_prev == NULL)
        list->head = node;
    else
        node_prev->next = node;

    list->size++;
    return ADDED;
}

int is_list_empty(struct list_t* list) {
    return list == NULL || list->head == NULL || list->size == 0;
}

enum AddOperationStatus insert_to_head(struct list_t* list, struct node_t* node) {
    if (assert_error(
        list == NULL || node == NULL,
        "insert_to_head",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    struct node_t* head = list->head;
    list->head = node;
    node->next = head;
    return ADDED;
}

enum AddOperationStatus insert_node_in_ordered_list_aux(struct list_t* list, struct node_t* node, struct node_t* node_prev, struct node_t* node_current) {
    if (node_current == NULL)
        return insert_node(list, node, node_prev, NULL);

    switch (entry_compare(node_current->entry, node->entry)) {
        case EQUAL:
            node_current->entry = node->entry;
            return REPLACED;
        case LOWER:
            return insert_node_in_ordered_list_aux(list, node, node_current, node_current->next);
        case GREATER:
            return insert_node(list, node, node_prev, node_current);
    default:
        return ADD_ERROR;
    }
}

enum AddOperationStatus insert_node_in_ordered_list(struct list_t* list, struct node_t* node) {
    if (assert_error(
        list == NULL || node == NULL,
        "insert_entry_in_order",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    return insert_node_in_ordered_list_aux(list, node, NULL, list->head);
}


int list_add(struct list_t *list, struct entry_t *entry) {
    if (assert_error(
        list == NULL,
        "list_add",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    struct node_t* new_node = node_create(entry, NULL);
    if (assert_error(
        new_node == NULL,
        "list_add",
        ERROR_CREATE_NODE
    )) return ADD_ERROR;

    switch (insert_node_in_ordered_list(list, new_node)) {
        case ADD_ERROR:
            destroy_dynamic_memory(new_node->entry);
            destroy_dynamic_memory(new_node);
            return ADD_ERROR;
        case REPLACED:
            destroy_dynamic_memory(new_node);
            return REPLACED;
        case ADDED:
            return ADDED;
        default:
            return ADD_ERROR;
    }
}

enum RemoveOperationStatus list_remove_aux(struct list_t *list, struct node_t* node_prev, struct node_t* node_current, char* key) {
    if (node_current == NULL)
        return NOT_FOUND;

    switch (string_compare(node_current->entry->key, key)) {
        case EQUAL:
            if (node_prev == NULL)
                list->head = node_current->next;
            else
                node_prev->next = node_current->next;
            list->size--;
            return REMOVED; 
        case GREATER:
            return NOT_FOUND;
        case LOWER:
            return list_remove_aux(list, node_current, node_current->next, key);
        default:
            return CMP_ERROR;
    }

}

int list_remove(struct list_t *list, char *key) {
    if (assert_error(
        list == NULL || key == NULL,
        "list_remove",
        ERROR_NULL_POINTER_REFERENCE
    )) return REMOVE_ERROR;

    return list_remove_aux(list, NULL, list->head, key);
}


struct entry_t *list_get_aux(struct node_t *node, char *key) {
    if (node == NULL || node->entry == NULL)
        return NULL;

    switch (string_compare(node->entry->key, key)) {
        case EQUAL:
            return node->entry;
        case CMP_ERROR:
            return NULL;
        default:
            return list_get_aux(node->next, key);
        }

}

struct entry_t *list_get(struct list_t *list, char *key) {
    if (assert_error(
        list == NULL || key == NULL,
        "list_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    return list_get_aux(list->head, key);
}

int list_size(struct list_t *list) {
    if (assert_error(
        list == NULL,
        "list_size",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    return list->size;
}

enum MemoryOperationStatus list_get_keys_aux(struct node_t* node, char** array, int index, int list_size) {
    if (index > list_size)
        return M_ERROR;

    if (index == list_size || node == NULL) {
        array[index] = NULL;
        return M_OK;
    }

    if (assert_error(
        node->entry == NULL || node->entry->key,
        "list_get_keys_aux",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    array[index] = strdup(node->entry->key);
    if (assert_error(
        array[index] == NULL,
        "list_get_keys_aux",
        ERROR_STRDUP
    )) return M_ERROR;
    return list_get_keys_aux(node->next, array, index + 1, list_size);    
}

char **list_get_keys(struct list_t *list) {
    if (assert_error(
        list == NULL,
        "list_get_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    char** array = create_dynamic_memory((sizeof(char*) * list->size) + 1);
    if (assert_error(
        array == NULL,
        "list_get_keys",
        ERROR_MALLOC
    )) return NULL;

    if (list_get_keys_aux(list->head, array, 0, list->size) == M_ERROR) {
        for (size_t i = 0; i < list->size; i++)
            destroy_dynamic_memory(array[i]);
        destroy_dynamic_memory(array);
        return NULL;
    }
    return array;
}

int list_free_keys(char **keys) {
    if (assert_error(
        keys == NULL,
        "list_free_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    int index = 0;
    char* key;
    while ((key = keys[index])) {
        destroy_dynamic_memory(key);
        index++;
    }
    destroy_dynamic_memory(keys);
    return M_OK;
}