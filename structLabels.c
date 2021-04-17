#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utilities.h"
#include "externs.h"
#include "structs.h"


/* handles entry field activities in symbol table */
int edit_entry(labelPtr lbl, char *name) {

    labelPtr label = get_label(lbl, name);
    if (label != NULL) {
        if (label->external) {
            err = EXTERN_INVALID_LABEL;
            return FALSE;
        }
        label->entry = TRUE;
        entry_existence = TRUE; /* Global variable that holds that there was at least one entry in the program */
        return TRUE;
    } else {
        err = ENTRY_LABEL_DOES_NOT_EXIST;
    }
    return FALSE;
}

/* This function checks if a given name is a name of a label in the list */
boolean is_existing_label(labelPtr h, char *name)
{
    return get_label(h, name) != NULL;
}

/* This function checks if a given label name is in the list if so return 1 else return 0. */
labelPtr get_label(labelPtr label, char *name)
{

    while(label)
    {
        if(strcmp(label->name, name) == 0) /* we found a label with the name given */
            return label;
        label=label->next;
    }
    return NULL;
}

/* This function adds a new label to the linked list of labels given its info. */
labelPtr add_label(labelPtr *hptr, char *name, unsigned int address, boolean external, ...)
{
    va_list p;

    labelPtr t=*hptr;
    labelPtr temp = {0}; /* Auxiliary variable to store the info of the label and add to the list */

    if(is_existing_label(*hptr, name))
    {
        err = LABEL_ALREADY_EXISTS;
        return NULL;
    }
    temp=(labelPtr) malloc(sizeof(Labels));
    if(!temp) /*if we couldn't allocate memory to temp then print an error massage and exit the program*/
    {
        printf("\nerror, cannot allocate memory\n");
        exit(ERROR);
    }

    /* Storing the info of the label in temp */
    strcpy(temp->name, name);
    temp -> entry = FALSE;
    temp -> address = address;
    temp -> external = external;

    if(!external) /* An external label can't be in an action statement */
    {
        va_start(p,external);
        temp -> inActionStatement = va_arg(p,boolean);
    } else
    {
        extern_existence = TRUE;
    }

    /* If the list is empty then we set the head of the list to be temp */
    if(!(*hptr))
    {
        *hptr = temp;
        temp -> next = NULL;
        return temp;
    }

    /* Setting a pointer to go over the list until he points on the last label and then setting temp to be the new last label */
    while(t -> next != NULL)
        t = t->next;
    temp -> next = NULL;
    t -> next = temp;

    va_end(p);
    return temp;
}

/* This function gets a label's name, searches the list for it and deletes the label.
 * If it managed to delete the label return 1 else return 0
 */
int delete_label(labelPtr *hptr, char *name)
{
    /* Goes over the label list and checking if a label by a given name is in the list if it is then deletes it by
    free its space and change the previous label's pointer to point to the next label */
    labelPtr temp = *hptr;
    labelPtr prevtemp = NULL;
    while (temp) {
        if (strcmp(temp->name, name) == 0) {
            if (strcmp(temp->name, (*hptr)->name) == 0) {
                *hptr = (*hptr)->next;
                free(temp);
            } else {
                prevtemp->next = temp->next;
                free(temp);
            }
            return 1;
        }
        prevtemp = temp;
        temp = temp->next;
    }
    return 0;

}

/* This function adds a node to a the next word */
void add_new_inst_node(){
    wordPtr tmpWord = (wordPtr) malloc(sizeof(wordMeta));
    word_list->next = tmpWord;
    word_list->next->prev = word_list;
    word_list = word_list->next;
}

/* This function adds a node to a the next word */
void add_new_data_node(){
    wordPtr tmpWord = (wordPtr) malloc(sizeof(wordMeta));
    tmpWord->next = NULL;
    data_word_list->next = tmpWord;
    data_word_list->next->prev = data_word_list;
    data_word_list = data_word_list->next;
}

/* This function frees the allocated memory for the symbols table */
void free_labels(labelPtr *hptr)
{
    /* Free the label list by going over each label and free it */
    labelPtr temp;
    while(*hptr)
    {
        temp=*hptr;
        *hptr=(*hptr)->next;
        free(temp);
    }
}

/* This function offsets the symbols addresses according to the IC values */
labelPtr update_data_symbol_address(){
    labelPtr head_symbol = symbols_table;
    while (symbols_table) {
        if (symbols_table->address_type == DATA_IMG) {
            symbols_table->address += ic;
        }
        symbols_table = symbols_table->next;
    }
    return head_symbol;
}

/* This function frees the allocated memory for the externals table*/
void free_ext(extPtr *hptr)
{
    /* Free the ext list by going over each ext and free it */
    extPtr temp;
    while(*hptr)
    {
        temp = *hptr;
        *hptr=(*hptr)->next;
        free(temp);
    }
}

/* This function frees the allocated memory for the word list*/
void free_words(wordPtr *hptr)
{
    /* Free the ext list by going over each ext and free it */
    wordPtr temp;
    while(*hptr)
    {
        temp = *hptr;
        *hptr=(*hptr)->next;
        free(temp);
    }
}

/* This function offsets the data image addresses according to the IC values */
wordPtr update_dataimg_address(wordPtr *data_list) {
    wordPtr head = *data_list;
    wordPtr follower = *data_list;

    while((*data_list)->next) {
        (*data_list)->address += ic; /* its dc value plus the final ic */
        follower = follower->next;
        if (!(follower->next)) { /* deletes the last node of the linked list*/
            free((*data_list)->next);
            (*data_list)->next = NULL;
        }
        else (*data_list) = (*data_list)->next;
    }
    return head;
}
