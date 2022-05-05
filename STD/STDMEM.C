/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdmem.c 1.6 1995/08/31 16:38:18 sam Exp $
 * $Locker: sam $
 *
 * Default memory handler that uses malloc()/free() from C library
 */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "brender.h"

/*
 * Glue functions for malloc()/free()
 */
static void * BR_CALLBACK BrStdlibAllocate(br_size_t size, br_uint_8 type)
{
	void *m;

	m = malloc(size);

	if(m == NULL)
		BR_ERROR2("BrStdlibAllocate: failed with size=%d, type=%d",size,type);

	return m;
}

static void BR_CALLBACK BrStdlibFree(void *mem)
{
	free(mem);
}

static br_size_t BR_CALLBACK BrStdlibInquire(br_uint_8 type)
{
	return MEMAVL;
}

/*
 * Allocator structure
 */
br_allocator BrStdlibAllocator = {
	"malloc",
	BrStdlibAllocate,
	BrStdlibFree,
	BrStdlibInquire,
};

/*
 * Override global variable s.t. this is the default allocator
 */
br_allocator *_BrDefaultAllocator = &BrStdlibAllocator;

