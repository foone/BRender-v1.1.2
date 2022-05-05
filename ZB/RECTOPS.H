/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: rectops.h 1.2 1995/02/22 21:53:39 sam Exp $
 * $Locker:  $
 *
 */

#ifndef _RECTOPS_H_
#define _RECTOPS_H_

#ifdef __cplusplus
extern "C" {
#endif

void BR_ASM_CALL GfxRectangleClear(br_pixelmap *dest,
									br_uint_16 x0, br_uint_16 y0,
									br_uint_16 x1, br_uint_16 y1,
									br_uint_32 pvalue);


void BR_ASM_CALL GfxRectangleTransfer(br_pixelmap *dest,
										br_uint_16 x0, br_uint_16 y0,
										br_uint_16 x1, br_uint_16 y1,
										br_pixelmap *src);

#ifdef __cplusplus
};
#endif
#endif
