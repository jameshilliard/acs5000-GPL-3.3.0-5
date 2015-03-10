/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_malloc.c
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_malloc
 *
 ****************************************************************/

#include <cyc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mem_info {
	void *mem;
	int size;
};

void *libcyc_malloc(struct cyc_info *cyc, size_t size)
{
	void *newmem = NULL;
	struct mem_info *minfo = NULL;

	if (cyc == NULL) return(NULL);

	if ((newmem = malloc(size)) == NULL) return(NULL);
	memset(newmem, '\0', size);

	minfo = malloc(sizeof(struct mem_info));
	minfo->mem = newmem;
	minfo->size = size;

	cyc->mem_ptrs=libcyc_ll_append(cyc->mem_ptrs, minfo);

/*
	if (cyc->flags & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];
		snprintf(debug_str, MAX_STR_LEN,
			 "%s Allocated %d bytes @ %p",
			 __func__, minfo->size, minfo->mem);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/
	
	return(newmem);
}

void libcyc_free(struct cyc_info *cyc, void *ptr)
{
	struct mem_info *minfo = NULL;
	struct mem_info *tmp = NULL;

	if ((ptr == NULL) || (cyc == NULL) || (cyc->mem_ptrs == NULL)) return;

	while ((tmp = libcyc_ll_iterate(cyc->mem_ptrs))) {
		if (tmp->mem == ptr) minfo = tmp;
	}

	if (minfo) {
		cyc->mem_ptrs = libcyc_ll_remove(cyc->mem_ptrs, minfo);
/*
		if (cyc->flags & CYC_DEBUG) {
			char debug_str[MAX_STR_LEN];
			snprintf(debug_str, MAX_STR_LEN,
				 "%s Freeing %d bytes @ %p", __func__,
				 minfo->size, minfo->mem);
			debug_str[MAX_STR_LEN-1] = '\0';
			libcyc_debug(cyc, debug_str);
		}
*/
		free(minfo->mem);
		minfo->mem = NULL;
		minfo->size = -1;
		free(minfo);
		minfo = NULL;
	}
}

void libcyc_free_all(struct cyc_info *cyc)
{
	struct mem_info *tmp = NULL;
	
	if ((cyc == NULL) || (cyc->mem_ptrs == NULL)) return;

	while ((tmp = libcyc_ll_iterate(cyc->mem_ptrs))) {
/*
		if (cyc->flags & CYC_DEBUG) {
			char debug_str[MAX_STR_LEN];
			snprintf(debug_str, MAX_STR_LEN,
				 "%s Freeing %d bytes @ %p", __func__,
				 tmp->size, tmp->mem);
			debug_str[MAX_STR_LEN-1] = '\0';
			libcyc_debug(cyc, debug_str);
		}
*/
		if (tmp->mem) free(tmp->mem);
		tmp->mem = NULL;
		tmp->size = -1;
	}
	
	libcyc_ll_free(cyc->mem_ptrs);
	cyc->mem_ptrs=NULL;
}

char *libcyc_strdup(struct cyc_info *cyc, const char *str)
{
	int len = strlen(str);
	char *s = NULL;
	
	if ((s = libcyc_malloc(cyc, len + 1)) == NULL) return(NULL);

	strncpy(s, str, len);
	s[len] = '\0';

	return(s);
}
