/*
 *    Copyright Voxar Limited 1995
 *
 *    basetype.h : a useful set of basic types.
 *
 *    $Id: basetype.h 1.2 1995/05/02 15:51:28 sam Exp $
 *
 */

#ifndef _BASETYPES_H_
#define _BASETYPES_H_

typedef br_int_32 Int_t;        /* useful general purpose integer */
typedef float     Float_t;      /* four byte floating point number */
typedef int       Bool_t;       /* just a boring boolean */

#ifdef TRUE
#undef TRUE
#endif
#define TRUE  (1)

#ifdef FALSE
#undef FALSE
#endif
#define FALSE (0)

#define ASSERT_TRAP(x) ((void)((x)||(BR_FATAL("Internal error: assertion failed.\n"),0)))

#endif