/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_ll.c
 *
 * Copyright (C) 2003-2004 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_ll
 *
 ****************************************************************/

#include <cyc.h>

/* Treat the linked list as a stack and push data onto it */
int libcyc_push(libcyc_list **stack, void *data)
{
	if ((data == NULL) || (stack == NULL)) return(ERROR);

	if (*stack == NULL) {
		/* We have a brand-new list */
		*stack=malloc(sizeof(libcyc_list));
		(*stack)->active=TRUE;
		(*stack)->data=data;
		(*stack)->next=NULL;
		(*stack)->prev=NULL;
	} else {
		libcyc_list *tmp=NULL;
		tmp=malloc(sizeof(libcyc_list));
		tmp->active=FALSE;
		tmp->data=data;
		tmp->prev=NULL;
		tmp->next=*stack;
		*stack=tmp;
	}

	return(0);
}

/* Treat the linked list as a stack and pop data from it */
void *libcyc_pop(libcyc_list **stack)
{
	void *data=NULL;
	libcyc_list *tmp=NULL;

	if (*stack == NULL) return(NULL);

	if ((*stack)->next) {
		if ((*stack)->data) data=(*stack)->data;
		tmp=*stack;
		*stack=(*stack)->next;
		free(tmp);
		tmp=NULL;
	} else {
		if ((*stack)->data) data=(*stack)->data;
		free(*stack);
		*stack=NULL;
	}

	return(data);
}

/* Append data to the end of the linked list */
libcyc_list *libcyc_ll_append(libcyc_list *list, void *data)
{
	if (data == NULL) return(NULL);

	if (list == NULL) {
		/* We have a brand-new list */
		list=malloc(sizeof(libcyc_list));
		list->active=TRUE;
		list->data=data;
		list->next=NULL;
		list->prev=NULL;
		return(list);
	} else {
		libcyc_list *tmp=NULL;
		/* Walk to the end and add the new data */
		for (tmp=list; tmp != NULL; tmp=tmp->next) {
			if (tmp->next == NULL) {
				tmp->next=malloc(sizeof(libcyc_list));
				tmp->next->active=FALSE;
				tmp->next->data=data;
				tmp->next->prev=tmp;
				tmp->next->next=NULL;
				return(list);
			}
		}
	}

	/* Not reached */
	return(NULL);
}

/* Remove the specified data from the list; return a pointer
   to the start of the new list */
libcyc_list *libcyc_ll_remove(libcyc_list *list, void *data)
{
	libcyc_list *tmp=NULL;

	if ((list == NULL) || (data == NULL)) return(NULL);

	for (tmp=list; tmp != NULL; tmp=tmp->next) {
		if ((tmp->data) && (tmp->data == data)) {
			/* We've found our element to remove */
			if ((tmp->prev == NULL) && tmp->next) {
				libcyc_list *t = NULL;
				/* We're at the beginning of the list;
				   reassign the next element as the first */
				tmp->next->prev=NULL;
				if (tmp->active) {
					tmp->next->active=TRUE;
				}
				t = tmp->next;
				free(tmp);
				tmp = NULL;
				return(t);
			} else if (tmp->prev &&
				   (tmp->next == NULL)) {
				/* We're at the end of the list;
				   reassign the second to last as
				   the last */
				tmp->prev->next=NULL;
				if (tmp->active) {
					tmp->prev->active=TRUE;
				}
				free(tmp);
				tmp = NULL;
				return(list);
			} else if (tmp->prev && tmp->next) {
				/* We're in the middle; lose the
				   specified link and reconnect
				   the ends. */
				tmp->prev->next=tmp->next;
				tmp->next->prev=tmp->prev;
				if (tmp->active) {
					tmp->next->active=TRUE;
				}
				free(tmp);
				tmp = NULL;
				return(list);
			} else {
				/* Only one link in the list, so
				   tmp == list; same as libcyc_ll_free() */
				libcyc_ll_free(list);
				return(NULL);
			}
		}
	}

	return(NULL);
}

/* Free all data in the list */
void libcyc_ll_free(libcyc_list *list)
{
	libcyc_list *tmp=NULL;

	if (list == NULL) return;

	for (tmp=list; tmp != NULL; tmp=tmp->next) {
		if (tmp->next == NULL) {
			/* We're at the end; free the link that
			   we're on and the previous one too.*/
			if (tmp->prev) {
				free(tmp->prev);
				tmp->prev = NULL;
			}
			free(tmp);
			tmp = NULL;
			return;
		} else if (tmp->prev && tmp->next) {
			/* We're in the middle; free the previous
			   link */
			free(tmp->prev);
			tmp->prev = NULL;
		}
	}
}


/* libcyc_list_iterate takes a linked list argument and
returns the data of each successive link */
void *libcyc_ll_iterate(libcyc_list *list)
{
	libcyc_list *tmp=NULL;

	if (list == NULL) return(NULL);

	for (tmp=list; tmp != NULL; tmp=tmp->next) {
		if (tmp->active && tmp->next) {
			tmp->active=FALSE;
			tmp->next->active=TRUE;
			return(tmp->data);
		} else if (tmp->active &&
			   (tmp->next == NULL)) {
			/* We're at the end; single-element list also */
			tmp->active=FALSE;
			return(tmp->data);
		}
	}

	/* Nothing's active; reset. */
	list->active=TRUE;
	return(NULL);
}

/* Returns the size of the list (e.g. number of links) */
int libcyc_ll_get_size(libcyc_list *list)
{
	libcyc_list *tmp=NULL;
	int size = 0;

	if (list == NULL) return(0);

	for (tmp=list; tmp != NULL; tmp=tmp->next) {
		size++;
	}

	return(size);
}
