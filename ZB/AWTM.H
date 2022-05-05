/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: awtm.h 1.3 1995/02/22 21:53:26 sam Exp $
 * $Locker:  $
 *
 * Global scanline workspace for arbitary width texture mapper
 */
#ifndef _AWTM_H_
#define _AWTM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct scan_edge *edge;

    char *start,*end;
    char *zstart;
    char *source_current;

    short u_int_current;
    short pad;
    unsigned u_current,du,du_carry,du_nocarry;
    int du_int,du_int_nocarry,du_int_carry;

    unsigned v_current,dv,dv_carry,dv_nocarry;

    int dsource,dsource_carry,dsource_nocarry;

    char *texture_start;
    int texture_size,texture_stride,texture_width;
} ArbitraryWidthScanLine;

extern ArbitraryWidthScanLine BR_ASM_DATA awsl;

#ifdef __cplusplus
};
#endif
#endif

