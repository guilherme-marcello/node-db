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

    // allocate memory for node
    struct node_t* node = create_dynamic_memory(sizeof(struct node_t));
    if (assert_error(
        node == NULL,
        "node_create",
        ERROR_MALLOC
    )) return NULL;

    // update with given values
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

    // destroy entry
    if (entry_destroy(node->entry) == M_ERROR)
        return M_ERROR;

    // destroy node struct
    destroy_dynamic_memory(node);
    
    return M_OK;
}

struct list_t* list_create() {
    // allocate memory for list
    struct list_t* list = create_dynamic_memory(sizeof(struct list_t));
    if (assert_error(
        list == NULL,
        "list_create",
        ERROR_MALLOC
    )) return NULL;

    // set head to NULL and size to 0
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

    // iterate over nodes (from head) and destroy one by one and return in case of error
    struct node_t* node;
    while ((node = list->head)) {
        list->head = node->next;
        if (node_destroy(node) == M_ERROR)
            return M_ERROR;

    }
    // destroy list struct itself
    destroy_dynamic_memory(list);
    return M_OK;   
}

enum AddOperationStatus insert_node(struct list_t* list, struct node_t* node, struct node_t* node_prev, struct node_t* node_next) {
    if (assert_error(
        list == NULL || node == NULL,
        "insert_node",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    // update node to point its next value to node_next
    node->next = node_next;
    // if there's no prev, node is the new head! otherwise, set node as node_prev next
    if (node_prev == NULL)
        list->head = node;
    else
        node_prev->next = node;

    // increment list size and return ADDED
    list->size++;
    return ADDED;
}

enum AddOperationStatus insert_node_in_ordered_list_aux(struct list_t* list, struct node_t* node, struct node_t* node_prev, struct node_t* node_current) {
    if (node_current == NULL)
        // reached the end of the list, insert_node here
        return insert_node(list, node, node_prev, NULL);

    switch (entry_compare(node_current->entry, node->entry)) {
        case EQUAL:
            // if the entries are equal, replace the current entry with the new entry
            struct entry_t* replaced_entry = node_current->entry;
            node_current->entry = node->entry;
            // destroy the replaced entry if the operation is successful, or return ADD_ERROR
            return entry_destroy(replaced_entry) == M_OK ? REPLACED : ADD_ERROR;
        case LOWER:
            // if the new entry is lower than the current entry, continue inserting in the next node
            return insert_node_in_ordered_list_aux(list, node, node_current, node_current->next);
        case GREATER:
            // if the new entry is greater than the current entry, insert_node here
            return insert_node(list, node, node_prev, node_current);
        default:
            // return ADD_ERROR in case of CMP_ERROR or any unknown state
            return ADD_ERROR;
    }
}

enum AddOperationStatus insert_node_in_ordered_list(struct list_t* list, struct node_t* node) {
    if (assert_error(
        list == NULL || node == NULL,
        "insert_entry_in_order",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    // start the insertion process by calling the auxiliary function with initial parameters (start from head).
    return insert_node_in_ordered_list_aux(list, node, NULL, list->head);
}


int list_add(struct list_t *list, struct entry_t *entry) {
    if (assert_error(
        list == NULL || entry == NULL,
        "list_add",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    struct node_t* new_node = node_create(entry, NULL);
    if (assert_error(
        new_node == NULL,
        "list_add",
        ERROR_CREATE_NODE
    )) return ADD_ERROR;

    // insert the new node into the ordered list and handle the return status
    switch (insert_node_in_ordered_list(list, new_node)) {
        case ADD_ERROR:
            // if insertion failed, destroy node and return ADD_ERROR
            node_destroy(new_node);
            return ADD_ERROR;
        case REPLACED:
            // if the entry was replaced during insertion, destroy new_node struct and return REPLACED
            destroy_dynamic_memory(new_node);
            return REPLACED;
        case ADDED:
            // if the entry was successfully added, return ADDED
            return ADDED;
        default:
            // return ADD_ERROR in other cases
            return ADD_ERROR;
    }
}

enum RemoveOperationStatus list_remove_aux(struct list_t *list, struct node_t* node_prev, struct node_t* node_current, char* key) {
    if (node_current == NULL)
        // if we reached the end of the list without finding the key, return NOT_FOUND
        return NOT_FOUND;

    // compare node_current key with key
    switch (string_compare(node_current->entry->key, key)) {
        case EQUAL:
            // if the keys match, remove the current node and adjust pointers accordingly
            struct node_t* node_current_next = node_current->next;
            if (node_destroy(node_current) == M_ERROR)
                // if node_destroy fails, return CMP_ERROR
                return CMP_ERROR;

            // decrement the list size and update pointers that were pointing to this node
            list->size--;
            if (node_prev == NULL)
                list->head = node_current_next;
            else
                node_prev->next = node_current_next;

            return REMOVED; 
        case GREATER:
            // if the current key is greater than the target key, return NOT_FOUND
            return NOT_FOUND;
        case LOWER:
            // if the current key is lower, continue searching in the next node
            return list_remove_aux(list, node_current, node_current->next, key);
        default:
            // return CMP_ERROR in other cases
            return CMP_ERROR;
    }
}
int list_remove(struct list_t *list, char *key) {
    if (assert_error(
        list == NULL || key == NULL,
        "list_remove",
        ERROR_NULL_POINTER_REFERENCE
    )) return REMOVE_ERROR;

    // start the removal process by calling the auxiliary function with initial parameters (from head)
    return list_remove_aux(list, NULL, list->head, key);
}


struct entry_t *list_get_aux(struct node_t *node, char *key) {
    if (node == NULL || node->entry == NULL)
        // if the current node or its entry is NULL, return NULL
        return NULL;

    switch (string_compare(node->entry->key, key)) {
        case EQUAL:
            // if the keys match, return a pointer to the entry
            return node->entry;
        case CMP_ERROR:
            // if an error occurred during comparison, return NULL
            return NULL;
        default:
            // if the keys do not match, continue searching in the next node
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

    // return list size
    return list->size;
}

enum MemoryOperationStatus list_get_keys_aux(struct node_t* node, char** array, int index, int list_size) {
    // check if the index exceeds the list size
    if (index > list_size)
        return M_ERROR;

    // if the index is equal to the list size or the node is NULL, we've reached the end
    if (index == list_size || node == NULL) {
        // mark the end of keys
        array[index] = NULL;
        return M_OK;
    }

    if (assert_error(
        node->entry == NULL || node->entry->key == NULL,
        "list_get_keys_aux",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    // duplicate the key from the current entry and store it in the array
    array[index] = strdup(node->entry->key);
    if (assert_error(
        array[index] == NULL,
        "list_get_keys_aux",
        ERROR_STRDUP
    )) return M_ERROR;

    // recursively call the function to process the next node in the list
    return list_get_keys_aux(node->next, array, index + 1, list_size);    
}

char **list_get_keys(struct list_t *list) {
    if (assert_error(
        list == NULL,
        "list_get_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    if (assert_error(
        list->head == NULL,
        "list_get_keys",
        ERROR_EMPTY_LIST
    )) return NULL;

    // allocate memory for the array of keys
    char** array = create_dynamic_memory(sizeof(char*) * (list->size + 1));
    if (assert_error(
        array == NULL,
        "list_get_keys",
        ERROR_MALLOC
    )) return NULL;

    // call the auxiliary function to retrieve keys and handle errors if they occur
    if (list_get_keys_aux(list->head, array, 0, list->size) == M_ERROR) {
        // clean up allocated memory and return NULL, in case of error
        for (size_t i = 0; i < list->size; i++)
            destroy_dynamic_memory(array[i]);
        destroy_dynamic_memory(array);
        return NULL;
    }
    
    // return the array containing the retrieved keys
    return array;
}


int list_free_keys(char **keys) {
    if (assert_error(
        keys == NULL,
        "list_free_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    // starting with index 0, iterate over keys, destroying them
    int index = 0;
    char* key;
    while ((key = keys[index])) {
        destroy_dynamic_memory(key);
        index++;
    }
    // destroy array and return M_OK
    destroy_dynamic_memory(keys);
    return M_OK;
}