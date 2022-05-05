/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: bitmap.c 1.1 1995/03/22 18:05:22 sam Exp $
 * $Locker:  $
 *
 * Bitmap management and convertion
 *
 * $BC<"make -f ifftopix.mak %s.obj;">
 */
/*static char rscid[] = "$Id: bitmap.c 1.1 1995/03/22 18:05:22 sam Exp $";*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <argstd.h>
#include <lists.h>
#include <error.h>

#include "bitmap.h"


struct Bitmap * NewBitmap(int width, int height, int palette)
{
	struct Bitmap *nbmp;

	NEW_PTR(nbmp);

	/*
	 * Allocate the new bitmap
	 */
	nbmp->type = BMT_BYTEPERPIXEL;
	nbmp->width = width;
	nbmp->height = height;


	if(BYTE_SIZE(nbmp) > UINT_MAX)
		ERROR2("picture is too big - %dx%d",nbmp->width,nbmp->height);

	nbmp->planes[0] = CALLOC(BYTE_SIZE(nbmp), 1);

	nbmp->npalette = palette;
	if(palette)
		nbmp->palette = CALLOC(nbmp->npalette, 3);

	nbmp->background = 0;

	return nbmp;
}

void FreeBitmap(struct Bitmap * bmp)
{
	int i;

	for (i = 0; i < BMP_MAX_PLANES; i++)
		if (bmp->planes[i])
			FREE(bmp->planes[i]);

	if (bmp->palette)
		FREE(bmp->palette);

	if (bmp->mask)
		FREE(bmp->mask);

	FREE(bmp);
}

struct Bitmap *ConvertToBytePerPixel(struct Bitmap * bmp)
{
	int i, x, y;
	int j, mask;

	UBYTE *bpp;
	struct Bitmap *nbmp;

	if (bmp->type != BMT_PLANES)
		return NULL;

	NEW_PTR(nbmp);

	/*
	 * Allocate the new bitmap
	 */
	nbmp->type = BMT_BYTEPERPIXEL;
	nbmp->width = bmp->width;
	nbmp->height = bmp->height;
	nbmp->npalette = bmp->npalette;

	if(BYTE_SIZE(nbmp) > UINT_MAX)
		ERROR2("picture is too big - %dx%d",nbmp->width,nbmp->height);

	nbmp->planes[0] = CALLOC(BYTE_SIZE(bmp), 1);
	nbmp->palette = CALLOC(bmp->npalette, 3);
	memcpy(nbmp->palette, bmp->palette, bmp->npalette * 3);
	nbmp->background = bmp->background;

	/*
	 * Mask each bitplane into bytemap
	 */

	for (y = 0; y < bmp->height; y++) {
		bpp = nbmp->planes[0] + y * BYTE_WIDTH(nbmp);

		/*
		 * work out offset and mask for the current pixel
		 */
		j = y * PLANE_WIDTH(bmp);
		mask = 0x80;
		for (x = 0; x < bmp->width; x++) {
			for (i = 0; i < bmp->nplanes; i++)
				if (bmp->planes[i][j] & mask)
					*bpp |= 1 << i;

			/*
			 * Update mask and offset
			 */
			if (mask == 0x1) {
				j++;
				mask = 0x80;
			} else
				mask >>= 1;
			bpp++;
		}
	}

	return nbmp;
}

struct Bitmap *ConvertToPlanes(struct Bitmap * bmp, int num)
{
	int i, j, size, x, y, mask;
	UBYTE *src;
	struct Bitmap *nbmp;

	if (bmp->type != BMT_BYTEPERPIXEL)
		return NULL;


	/*
	 * Allocate the new bitplanes
	 */
	NEW_PTR(nbmp);

	nbmp->type = BMT_PLANES;
	nbmp->width = bmp->width;
	nbmp->height = bmp->height;
	nbmp->npalette = bmp->npalette;
	nbmp->palette = CALLOC(bmp->npalette, 3);
	memcpy(nbmp->palette, bmp->palette, bmp->npalette * 3);
	nbmp->background = bmp->background;

	for (i = 0; i < 8; i++)
		nbmp->planes[i] = 0;

	size = PLANE_SIZE(bmp);

	for (i = 0; i < num; i++)
		nbmp->planes[i] = CALLOC(size, 1);

	/*
	 * Unpack each byte into the new bitplanes
	 */


	for (y = 0; y < bmp->height; y++) {
		src = bmp->planes[0] + y * BYTE_WIDTH(bmp);

		/*
		 * work out offset and mask for the current pixel
		 */
		j = y * PLANE_WIDTH(bmp);
		mask = 0x80;

		for (x = 0; x < bmp->width; x++) {

			for (i = 0; i < num; i++)
				if (*src & (1 << i))
					nbmp->planes[i][j] |= mask;

			/*
			 * Update mask and offset
			 */
			if (mask == 0x1) {
				j++;
				mask = 0x80;
			} else {
				mask >>= 1;
			}
			src++;
		}
	}

	return nbmp;
}

void GenMask_Planes(struct Bitmap * bmp)
{
	int i, j, k, mask;
	int bgbits[BMP_MAX_PLANES];

	if (bmp->type != BMT_PLANES)
		return;

	for (k = 0; k < bmp->nplanes; k++)
		bgbits[k] = (bmp->background & (1 << k)) ? -1 : 0;

	/*
	 * Allocate the mask plane
	 */
	j = PLANE_SIZE(bmp);
	bmp->mask = CALLOC(j, 1);

	/*
	 * generate the mask plane
	 */
	for (i = 0; i < j; i++) {
		mask = 0;
		for (k = 0; k < bmp->nplanes; k++)
			mask |= bmp->planes[k][i] ^ bgbits[k];
		/*
		 * Clear the background colour out to zero in all planes
		 */
		for (k = 0; k < bmp->nplanes; k++)
			bmp->planes[k][i] &= mask;

		bmp->mask[i] = ~mask;
	}
}

void CopyBitmap(struct Bitmap *dst,struct Bitmap *src)
{
	int r,width;
	UBYTE *src_p,*dst_p;

	if(src->width > dst->width) {
		width = dst->width;
	} else {
		width = src->width;
	}

	if(src->height > dst->height) {
		r = dst->height;
	} else {
		r = src->height;
	}

	src_p = src->planes[0];
	dst_p = dst->planes[0];


//	fprintf(stderr,"%d %d %d %d\n",width,r,dmod,smod);

	while(r--) {
		memcpy(dst_p, src_p, width);
		dst_p += dst->width;
		src_p += src->width;
	}
}

struct Bitmap *ExtractSubBitmap(struct Bitmap * bmp, struct Rectangle * rect)
{
	struct Bitmap *nbmp;
	UBYTE *src, *dst;
	int y;

	if (bmp->type != BMT_BYTEPERPIXEL)
		return NULL;

	NEW_PTR(nbmp);

	nbmp->type = BMT_BYTEPERPIXEL;

	nbmp->palette = CALLOC(bmp->npalette * 3, 1);
	memcpy(nbmp->palette, bmp->palette, bmp->npalette * 3);
	nbmp->npalette = bmp->npalette;

	nbmp->background = bmp->background;

	nbmp->width = rect->w;
	nbmp->height = rect->h;

	nbmp->planes[0] = CALLOC(BYTE_SIZE(nbmp), 1);
	dst = nbmp->planes[0];

	src = bmp->planes[0] + rect->y * BYTE_WIDTH(bmp) + rect->x;

	for (y = 0; y < nbmp->height; y++) {
		memcpy(dst, src, nbmp->width);
		dst += BYTE_WIDTH(nbmp);
		src += BYTE_WIDTH(bmp);
	}

	return nbmp;
}
