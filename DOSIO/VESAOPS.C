 /*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesaops.c 1.6 1995/08/31 17:20:11 sam Exp $
 * $Locker:  $
 *
 * VESA pixelmap operations
 */
#include <stdio.h>
#include <dos.h>

#include "fw.h"
#include "brassert.h"
#include "syshost.h"

#include "vesa.h"
#include "dosgfxcm.h"
#include "realsupt.h"
#include "vesaloop.h"
#include "vesaops.h"

void BR_ASM_CALL _BrVESACopyTo(br_context *ctx,
	br_pixelmap *dst, br_pixelmap *src)
{
	_PagedScreenCopyToNoBreaks(dst->pixels, ctx->qualifier,
		src->pixels,src->row_bytes);
}

void BR_ASM_CALL _BrVESACopyFrom(br_context *ctx,
	br_pixelmap *dst, br_pixelmap *src)
{
	_PagedScreenCopyFromNoBreaks(dst->pixels, ctx->qualifier,
		src->pixels,src->row_bytes);
}

void BR_ASM_CALL _BrVESABCopyTo(br_context *ctx,
	br_pixelmap *dst, br_pixelmap *src)
{
#if 0
	_PagedScreenCopyToBreaks(dst->pixels, ctx->qualifier,
		src->pixels,src->row_bytes);
#endif

	_BrVESABRectangleCopyTo(ctx,dst,0,0,src,0,0,dst->width,dst->height);
}

void BR_ASM_CALL _BrVESABCopyFrom(br_context *ctx,
	br_pixelmap *dst, br_pixelmap *src)
{
#if 0
	_PagedScreenCopyFromBreaks(dst->pixels, ctx->qualifier,
		src->pixels,src->row_bytes);
#endif

	_BrVESABRectangleCopyFrom(ctx,dst,0,0,src,0,0,dst->width,dst->height);
}

void BR_ASM_CALL _BrVESAClear(br_context *ctx, br_pixelmap *dst,
	br_colour colour)
{
	/*
	 * clear screen
	 */
	_PagedRectangleClearNoBreaks(dst->pixels, ctx->qualifier,
		0, 0, dst->width, dst->height, colour);
}

void BR_ASM_CALL _BrVESABClear(br_context *ctx,
	br_pixelmap *dst ,br_colour colour)
{
	/*
	 * copy to screen
	 */
	_PagedRectangleClearBreaks(dst->pixels, ctx->qualifier,
		0, 0, dst->width, dst->height, colour);
}

void BR_ASM_CALL _BrVESARectangleCopyTo(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h)
{
   	char *mem;
	/*
	 * copy to screen
	 */
	mem = (char *)(src->pixels) +
		(sy+src->base_y)*src->row_bytes +
		(sx+src->base_x)*pixel_size;

	_PagedRectangleCopyToNoBreaks(dst->pixels, ctx->qualifier,
		mem, src->row_bytes, dx, dy, w, h);
}

void BR_ASM_CALL _BrVESABRectangleCopyTo(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h)
{
   	char *mem;
	/*
	 * copy to screen
	 */
	mem = (char *)(src->pixels) +
		(sy+src->base_y)*src->row_bytes +
		(sx+src->base_x)*pixel_size;

	_PagedRectangleCopyToBreaks(dst->pixels, ctx->qualifier,
		mem, src->row_bytes, dx, dy, w, h);
}

void BR_ASM_CALL _BrVESARectangleCopyFrom(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h)
{
   	char *mem;
	/*
	 * copy from screen
	 */
	mem = (char *)(dst->pixels) +
		(dy+dst->base_y)*dst->row_bytes +
		(dx+dst->base_x)*pixel_size;

	_PagedRectangleCopyFromNoBreaks(src->pixels, ctx->qualifier,
		mem, dst->row_bytes, sx, sy, w, h);
}

void BR_ASM_CALL _BrVESABRectangleCopyFrom(br_context *ctx,
	br_pixelmap *dst, br_uint_16 dx,br_uint_16 dy,
	br_pixelmap *src,br_uint_16 sx,br_uint_16 sy,br_uint_16 w,br_uint_16 h)
{
   	char *mem;
	/*
	 * copy from screen
	 */
	mem = (char *)(dst->pixels) +
		(dy+dst->base_y)*dst->row_bytes +
		(dx+dst->base_x)*pixel_size;

	_PagedRectangleCopyFromBreaks(src->pixels, ctx->qualifier,
		mem, dst->row_bytes, sx, sy, w, h);
}

void BR_ASM_CALL _BrVESARectangleClear(br_context *ctx, br_pixelmap *dst,
	br_uint_16 x,br_uint_16 y,br_uint_16 w,br_uint_16 h,br_colour colour)
{
	/*
	 * copy to screen
	 */
	_PagedRectangleClearNoBreaks(dst->pixels, ctx->qualifier,
		x, y, w, h, colour);
}

void BR_ASM_CALL _BrVESABRectangleClear(br_context *ctx, br_pixelmap *dst,
	br_uint_16 x,br_uint_16 y,br_uint_16 w,br_uint_16 h,br_colour colour)
{
	/*
	 * copy to screen
	 */
	_PagedRectangleClearBreaks(dst->pixels, ctx->qualifier,
		x, y, w, h, colour);
}


void BR_ASM_CALL _BrVESAPixelSet(br_context *ctx, br_pixelmap *dst,
	br_uint_16 x,br_uint_16 y,br_uint_32 colour)
{
   	_PagedPixelSetNoBreaks(dst->pixels, ctx->qualifier, x,y,colour);
}

void BR_ASM_CALL _BrVESABPixelSet(br_context *ctx, br_pixelmap *dst,
	br_uint_16 x,br_uint_16 y,br_uint_32 colour)
{
   	_PagedPixelSetBreaks(dst->pixels, ctx->qualifier, x,y,colour);
}

br_uint_32 BR_ASM_CALL _BrVESAPixelGet(br_context *ctx,
	br_pixelmap *src, br_uint_16 x,br_uint_16 y)
{
	return _PagedPixelGetNoBreaks(src->pixels, ctx->qualifier, x,y);
}

br_uint_32 BR_ASM_CALL _BrVESABPixelGet(br_context *ctx,
	br_pixelmap *src, br_uint_16 x,br_uint_16 y)
{
	return _PagedPixelGetBreaks(src->pixels, ctx->qualifier, x,y);
}

void BR_ASM_CALL _BrVESACopyBits(br_context *ctx,  br_pixelmap *dst,
		br_int_16 x, br_int_16 y, br_uint_8 *src,
		br_uint_16 s_stride,br_uint_16 start_bit,br_uint_16 end_bit,
		br_uint_16 nrows,br_uint_32 colour)
{
	_PagedCopyBitsNoBreaks(dst->pixels, ctx->qualifier,
		x,y, src, s_stride, start_bit, end_bit, nrows, colour);
}

void BR_ASM_CALL _BrVESABCopyBits(br_context *ctx,  br_pixelmap *dst,
		br_int_16 x, br_int_16 y, br_uint_8 *src,
		br_uint_16 s_stride,br_uint_16 start_bit,br_uint_16 end_bit,
		br_uint_16 nrows,br_uint_32 colour)
{
	_PagedCopyBitsBreaks(dst->pixels, ctx->qualifier,
		x,y, src, s_stride, start_bit, end_bit, nrows, colour);
}
