/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesaops.h 1.3 1995/07/28 18:59:05 sam Exp $
 * $Locker:  $
 *
 *
 */
#ifndef _VESAOPS_H_
#define _VESAOPS_H_

#ifdef __cplusplus
extern "C" {
#endif

void BR_ASM_CALL _BrVESACopyTo(br_context *ctx, br_pixelmap *dst, br_pixelmap *src);
void BR_ASM_CALL _BrVESABCopyTo(br_context *ctx, br_pixelmap *dst, br_pixelmap *src);

void BR_ASM_CALL _BrVESACopyFrom(br_context *ctx, br_pixelmap *dst, br_pixelmap *src);
void BR_ASM_CALL _BrVESABCopyFrom(br_context *ctx, br_pixelmap *dst, br_pixelmap *src);

void BR_ASM_CALL _BrVESAClear(br_context *ctx, br_pixelmap *dst, br_colour colour);
void BR_ASM_CALL _BrVESABClear(br_context *ctx, br_pixelmap *dst, br_colour colour);

void BR_ASM_CALL _BrVESARectangleCopyTo(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h);
void BR_ASM_CALL _BrVESABRectangleCopyTo(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h);

void BR_ASM_CALL _BrVESARectangleCopyFrom(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h);
void BR_ASM_CALL _BrVESABRectangleCopyFrom(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h);

void BR_ASM_CALL _BrVESARectangleClear(br_context *ctx, br_pixelmap *dst,
	br_uint_16 x,br_uint_16 y,br_uint_16 w,br_uint_16 h,br_colour colour);
void BR_ASM_CALL _BrVESABRectangleClear(br_context *ctx, br_pixelmap *dst,
	br_uint_16 x,br_uint_16 y,br_uint_16 w,br_uint_16 h,br_colour colour);

void BR_ASM_CALL _BrVESAPixelSet(br_context *ctx, br_pixelmap *dst, br_uint_16 x,br_uint_16 y,br_uint_32 colour);
void BR_ASM_CALL _BrVESABPixelSet(br_context *ctx, br_pixelmap *dst, br_uint_16 x,br_uint_16 y,br_uint_32 colour);

br_uint_32 BR_ASM_CALL _BrVESAPixelGet(br_context *ctx, br_pixelmap *dst, br_uint_16 x,br_uint_16 y);
br_uint_32 BR_ASM_CALL _BrVESABPixelGet(br_context *ctx, br_pixelmap *dst, br_uint_16 x,br_uint_16 y);

void BR_ASM_CALL _BrVESACopyBits(br_context *ctx, br_pixelmap *dst, br_int_16 x, br_int_16 y, br_uint_8 *src,
		br_uint_16 s_stride,br_uint_16 start_bit,br_uint_16 end_bit,
		br_uint_16 nrows,br_uint_32 colour);

void BR_ASM_CALL _BrVESABCopyBits(br_context *ctx, br_pixelmap *dst, br_int_16 x, br_int_16 y, br_uint_8 *src,
		br_uint_16 s_stride,br_uint_16 start_bit,br_uint_16 end_bit,
		br_uint_16 nrows,br_uint_32 colour);


#ifdef __cplusplus
};
#endif
#endif


