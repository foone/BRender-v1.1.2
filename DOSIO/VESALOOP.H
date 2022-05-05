/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesaloop.h 1.2 1995/07/28 18:59:01 sam Exp $
 * $Locker:  $
 *
 * VESA specific pixelmap loops
 */
#ifndef _VESALOOPS_H_
#define _VESALOOPS_H_

#ifdef __cplusplus
extern "C" {
#endif

extern br_uint_32 BR_ASM_DATA window_start;
extern br_uint_32 BR_ASM_DATA window_end;
extern br_uint_32 BR_ASM_DATA pixelwidth;
extern br_uint_32 BR_ASM_DATA pixelheight;
extern br_uint_32 BR_ASM_DATA scanlines_per_page;
extern br_uint_32 BR_ASM_DATA page_size;

extern br_uint_32 BR_ASM_DATA pixel_size;
extern br_uint_32 BR_ASM_DATA full_banks;
extern br_uint_32 BR_ASM_DATA lines_left;
extern br_uint_32 BR_ASM_DATA bank_shift;
extern br_uint_32 BR_ASM_DATA bank_increment;
extern br_uint_32 BR_ASM_DATA scanline_size;
extern br_uint_32 BR_ASM_DATA source_scanline_size;
extern br_uint_32 BR_ASM_DATA colour_buffer_size;

void BR_ASM_CALL _PagedScreenCopyToNoBreaks(br_uint_8 *dest, br_uint_32 dest_qual, br_uint_8 *src, br_uint_32 src_stride);
void BR_ASM_CALL _PagedScreenCopyToBreaks(br_uint_8 *dest, br_uint_32 dest_qual, br_uint_8 *src, br_uint_32 src_stride);

void BR_ASM_CALL _PagedScreenCopyFromNoBreaks(br_uint_8 *dest, br_uint_32 dest_qual, br_uint_8 *src, br_uint_32 src_stride);
void BR_ASM_CALL _PagedScreenCopyFromBreaks(br_uint_8 *dest, br_uint_32 dest_qual, br_uint_8 *src, br_uint_32 src_stride);

void BR_ASM_CALL _PagedRectangleCopyToNoBreaks(
	br_uint_8 *dest, br_uint_32 dest_qual,
	br_uint_8 *src, br_uint_32 src_stride,
	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height);

void BR_ASM_CALL _PagedRectangleCopyToBreaks(
	br_uint_8 *dest, br_uint_32 dest_qual,
	br_uint_8 *src, br_uint_32 src_stride,
	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height);

void BR_ASM_CALL _PagedRectangleCopyFromNoBreaks(
	br_uint_8 *dest, br_uint_32 dest_qual,
	br_uint_8 *src, br_uint_32 src_stride,
	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height);

void BR_ASM_CALL _PagedRectangleCopyFromBreaks(
	br_uint_8 *dest, br_uint_32 dest_qual,
	br_uint_8 *src, br_uint_32 src_stride,
	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height);

void BR_ASM_CALL _PagedRectangleClearNoBreaks(
	br_uint_8 *dest, br_uint_32 dest_qual,
	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height,
	br_uint_32 colour);

void BR_ASM_CALL _PagedRectangleClearBreaks(
	br_uint_8 *dest, br_uint_32 dest_qual,
	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height,
	br_uint_32 colour);

void BR_ASM_CALL _PagedPixelSetNoBreaks(br_uint_8 *dest, br_uint_32 qual, br_uint_32 px, br_uint_32 py, br_uint_32 colour);
void BR_ASM_CALL _PagedPixelSetBreaks(br_uint_8 *dest, br_uint_32 qual, br_uint_32 px, br_uint_32 py, br_uint_32 colour);

br_uint_32 BR_ASM_CALL _PagedPixelGetNoBreaks(br_uint_8 *src, br_uint_32 qual, br_uint_32 px, br_uint_32 py);
br_uint_32 BR_ASM_CALL _PagedPixelGetBreaks(br_uint_8 *src, br_uint_32 qual, br_uint_32 px, br_uint_32 py);

void BR_ASM_CALL _PagedCopyBitsNoBreaks(
	br_uint_8 *dest,
	br_uint_32 dest_qual,
	br_int_32 x,
	br_int_32 y,
	br_uint_8 *src,
	br_uint_32 s_stride,
	br_uint_32 start_bit,
	br_uint_32 end_bit,
	br_uint_32 n_rows,
	br_uint_32 colour);

void BR_ASM_CALL _PagedCopyBitsBreaks(
	br_uint_8 *dest,
	br_uint_32 dest_qual,
	br_int_32 x,
	br_int_32 y,
	br_uint_8 *src,
	br_uint_32 s_stride,
	br_uint_32 start_bit,
	br_uint_32 end_bit,
    br_uint_32 n_rows,
	br_uint_32 colour);


#ifdef __cplusplus
};
#endif
#endif

