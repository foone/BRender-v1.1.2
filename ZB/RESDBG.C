/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: resdbg.c 1.1 1995/04/07 19:27:54 sam Exp $
 * $Locker:  $
 *
 */

#include <string.h>

#include <stdio.h>

#include "zb.h"
#include "brassert.h"

/*
 * The granularity of resource sizes
 */
#define BR_RES_GRANULARITY	8
#define BR_RES_MAGIC		0xDEADBEEF

/*
 * The control structure prepended onto resource blocks
 */
struct resource_header {
	br_simple_node node;
	br_simple_list children;
	unsigned int class:8;			/* Class of resource 			 	*/
	int	size:24;					/* Size of resource in 				*/
									/*  BR_RES_GRANULARITY units 		*/
#if BR_RES_TAGGING
	void *magic_ptr;
	int	  magic_num;
#endif
};

#if BR_RES_TAGGING
#define ISRESOURCE(res) ((res->magic_ptr == res) && (res->magic_num == BR_RES_MAGIC))
#else
#define ISRESOURCE(res) 1
#endif

#define RES_BOUNDARY 16
#define RES_ALIGN(x) ((void *)(((br_uint_32)(x)+(RES_BOUNDARY-1)) & ~(RES_BOUNDARY-1)))

/*
 * Align the resource pointer
 */
static void *ResToUser(struct resource_header *r)
{
	/*
	 * Move over the resource_header
	 */
	r++;

	/*
	 * Allocations should always be at least long word aligned
	 */
	ASSERT( (((int)r) & 3)  == 0);

	/*
	 * Bump pointer up to next boundary
	 */
	return RES_ALIGN(r);
}

/*
 * Unalign the resource pointer
 */
static struct resource_header *UserToRes(void *r)
{
	br_uint_32 *l = r;

#if 0
	ASSERT( ((int) r & (RES_BOUNDARY-1)) == 0);
#endif

	/*
	 * the last long-word of the resource will always be non-zero -
	 * class/size or magic_num
	 *
	 * Move pointer back until we hit it
	 */
	while(*(l - 1) == 0)
		l--;

	return ((struct resource_header *)l) - 1;
}

/*
 * Debug function that will dump a resource hierachy
 *
 * Invokes a callback with each text line of the dump
 */
static void InternalResourceDump(struct resource_header *res, void (*putline)(char *str, void *arg), void *arg, int level)
{
#if 1
	int i;
	char *cp = _br_scratch_string;
	struct resource_header *child;
	br_resource_class *rclass;

	/*
	 * Blank out first bit of line
	 */
	for(i=0; i< level*2; i++)
		*cp++ = ' ';

	/*
	 * Describe resource
	 */
 	rclass= fw.resource_class_index[(unsigned char )res->class];

	if(rclass == NULL) {
		sprintf(cp,"0x%08x ??? (%02X)", res+1, res->class);
		putline(_br_scratch_string,arg);
		return;
	}

	sprintf(cp,"0x%08x %-20s %d", ResToUser(res), rclass->identifier, res->size);
	putline(_br_scratch_string,arg);

	/*
	 * Display any children
	 */
	BR_FOR_SIMPLELIST(&res->children,child)
		InternalResourceDump(child, putline, arg, level+1);
#endif
}

void BR_PUBLIC_ENTRY BrResDump(void *vres, void (*putline)(char *str, void *arg), void *arg)
{
	struct resource_header *res = UserToRes(vres);

	InternalResourceDump(res, putline, arg, 0);
}

void BR_PUBLIC_ENTRY BrResDumpAll(char *dump_name)
{
	void *fh;
	
	fh = BrFileOpenWrite(dump_name,1);
	if(fh == NULL)
		return;

	BrFilePutLine("-- FW --\n",fh);
	BrResDump(fw.res,BrFilePutLine,fh);

	BrFilePutLine("\n-- ZB --\n",fh);
	BrResDump(zb.res,BrFilePutLine,fh);

	BrFileClose(fh);
}

