/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: loadiff.c 1.1 1995/03/16 12:02:01 sam Exp $
 * $Locker:  $
 *
 * Reading IFF ILBM and PBM files
 *
 * $BC<"make -f ifftopix.mak %s.obj;">
 */
/* static char rscid[] = "$Id: loadiff.c 1.1 1995/03/16 12:02:01 sam Exp $"; */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <argstd.h>
#include <lists.h>
#include <error.h>

#include "bitmap.h"
#include "loadiff.h"

struct BitMapHeader BMhd;
struct Chunk Header;

void ExpandILBM(struct Bitmap * bm, struct BitMapHeader * bmhd, BYTE * sourcebuf);
void ExpandPBM(struct Bitmap * bm, struct BitMapHeader * bmhd, BYTE * sourcebuf);

UDWORD SwapLong(UDWORD l)
{
	union {
		UDWORD l;
		UBYTE c[4];
	} swap;
	UBYTE t;

	swap.l = l;
	t = swap.c[3];
	swap.c[3] = swap.c[0];
	swap.c[0] = t;
	t = swap.c[2];
	swap.c[2] = swap.c[1];
	swap.c[1] = t;
	return swap.l;
}

UWORD SwapWord(UWORD w)
{
	union {
		UWORD w;
		UBYTE c[2];
	} swap;
	UBYTE t;

	swap.w = w;
	t = swap.c[1];
	swap.c[1] = swap.c[0];
	swap.c[0] = t;
	return swap.w;
}

void ReadHeader(FILE * fh, struct Chunk * hp)
{
	fread(hp, sizeof(*hp), 1, fh);

	hp->ckID = SwapLong(hp->ckID);
	hp->ckSize = SwapLong(hp->ckSize);
}

void ReadBMHD(FILE * fh, struct BitMapHeader * bmp)
{
	fread(bmp, sizeof(*bmp), 1, fh);

	bmp->w = SwapWord(bmp->w);
	bmp->h = SwapWord(bmp->h);
	bmp->x = SwapWord(bmp->x);
	bmp->y = SwapWord(bmp->y);
	bmp->transparentColor = SwapWord(bmp->transparentColor);
	bmp->pageWidth = SwapWord(bmp->pageWidth);
	bmp->pageHeight = SwapWord(bmp->pageHeight);

}

void ReadLong(FILE * fh, UDWORD * lp)
{
	fread(lp, sizeof(*lp), 1, fh);

	*lp = SwapLong(*lp);
}

/*
 * Unpack planes
 */
void ExpandILBM(struct Bitmap * bm, struct BitMapHeader * bmhd, BYTE * bodydata)
{
	signed char n, *destbuf;
	int plane, linelen, rowbytes, i, j, k;

	linelen = 2 * ((bmhd->w + 15) >> 4);

	/*
	 * Allocate the plane buffers
	 */
	j = 2 * ((bmhd->w + 15) >> 4) * bmhd->h;

	for (i = 0; i < bmhd->nPlanes; i++)
		bm->planes[i] = CALLOC(j, 1);


	for (i = 0; i < bmhd->h; i++) {
		for (plane = 0; plane < bmhd->nPlanes; plane++) {

			destbuf = (char *) (bm->planes[plane]) + (i * linelen);

			if (bmhd->compression == 1) {
				/*
				 * compressed screen
				 */
				rowbytes = linelen;

				while (rowbytes) {
					/*
					 * unpack until 1 scan-line complete
					 */
					n = *bodydata++;

					/*
					 * uncompressed block - copy n bytes verbatim
					 */
					if (n >= 0) {
						k = (int)n+1;
						memcpy(destbuf, bodydata, k);
						rowbytes -= k;
						destbuf += k;
						bodydata += k;
					} else {
						/*
						 * compressed block - expand n duplicate bytes
						 */
						k = -(int)n + 1;
						rowbytes -= k;
						memset(destbuf, *bodydata++, k);
						destbuf += k;
					}
				}
			} else {
				/*
				 * uncompressed - just copy
				 */
				memcpy(destbuf, bodydata, linelen);
				bodydata += linelen;
				destbuf += linelen;
			}
		}
	}
}

/*
 * Unpack an PBM to byte per pixel
 */
void ExpandPBM(struct Bitmap * bm, struct BitMapHeader * bmhd, BYTE * bodydata)
{
	UBYTE *destbuf, *databuf;
	int i, linelen, rowbytes, n;
	long planebytes = 0;

	/*
	 * Allocate the picture data buffer
	 */
	linelen = (bmhd->w + 1) & ~1;

	/*
	 * Allocate a byte per pixel data area
	 */
	planebytes = (long) linelen * (long) bmhd->h;

	if(planebytes > UINT_MAX)
		ERROR2("picture is too big - %dx%d",bmhd->w,bmhd->h);

	databuf = CALLOC(planebytes, 1);

	/*
	 * Fill in the bitmap elements
	 */
	bm->width = bmhd->w;
	bm->height = bmhd->h;
	bm->background = bmhd->transparentColor;

	/*
	 * Unpack the file data into bitplanes
	 */

	for (i = 0; i < bmhd->h; i++) {
		destbuf = databuf + (i * linelen);

		if (bmhd->compression == 1) {
			/*
			 * compressed screen
			 */
			rowbytes = linelen;

			while (rowbytes) {
				/*
				 * unpack until 1 scan-line complete
				 */
				n = *bodydata++;

				/*
				 * uncompressed block - copy n bytes verbatim
				 */
				if (n >= 0) {
					memcpy(destbuf, bodydata, ++n);
					rowbytes -= n;
					destbuf += n;
					bodydata += n;
				} else {
					/*
					 * compressed block - expand n duplicate bytes
					 */
					n = -n + 1;
					rowbytes -= n;
					memset(destbuf, *bodydata++, n);
					destbuf += n;
				}
			}
		} else {
			/*
			 * uncompressed - just copy
			 */
			memcpy(destbuf, bodydata, linelen);
			bodydata += linelen;
			destbuf += linelen;
		}
	}

	bm->planes[0] = databuf;
}

struct Bitmap *ReadPicture_IFF(char *filename)
{
	UDWORD id;
	BYTE *bodydata = NULL;
	struct Bitmap *bm = NULL;
	struct Bitmap *nbm = NULL;

	FILE *fh = FOPEN(filename,"rb");

	/*
	 * Allocate the bitmap structure
	 */
	bm = CALLOC(sizeof(struct Bitmap), 1);

	ReadHeader(fh, &Header);
	if (Header.ckID != ID_FORM)
		goto badiff;

	ReadLong(fh, &id);
	switch (id) {
	case ID_ILBM:
		bm->type = BMT_PLANES;
		break;

	case ID_PBM:
		bm->type = BMT_BYTEPERPIXEL;
		break;

	default:
		goto badiff;
	};

	for (;;) {
		ReadHeader(fh, &Header);

		if (feof(fh))
			return NULL;

		if (Header.ckID == ID_BODY)
			break;

		switch (Header.ckID) {
		case ID_BMHD:
			ReadBMHD(fh, &BMhd);
			break;
		case ID_CMAP:
			bm->palette = MALLOC(Header.ckSize);
			bm->npalette = Header.ckSize / 3;

			fread(bm->palette, 1, Header.ckSize, fh);
			break;
		default:
			fseek(fh, ROUNDODDUP(Header.ckSize), SEEK_CUR);
		}
	}

	/*
	 * Fill in the bitmap elements
	 */
	bm->width = BMhd.w;
	bm->height = BMhd.h;
	bm->nplanes = BMhd.nPlanes;
	bm->background = BMhd.transparentColor;

	/*
	 * Read data into RAM for ease of decompression
	 */
	bodydata = MALLOC(Header.ckSize);

	fread(bodydata, 1, Header.ckSize, fh);

	switch (bm->type) {

	case BMT_PLANES:
		ExpandILBM(bm, &BMhd, bodydata);
		break;
	case BMT_BYTEPERPIXEL:
		ExpandPBM(bm, &BMhd, bodydata);
		break;
	}

	/*
	 * Close the file
	 */
	FCLOSE(fh);

	/*
	 * Get rid of the original file data
	 */
	FREE(bodydata);

	/*
	 * Convert a bitplane picture to byte per pixel
	 */
	if (bm->type == BMT_PLANES) {
		nbm = ConvertToBytePerPixel(bm);
		FreeBitmap(bm);
		return nbm;
	} else {
		return bm;
	}

badiff:
	FREE(bm);
	FCLOSE(fh);
	return NULL;
}


int ReadPalette_IFF(struct Bitmap * bmp, char *filename, int num, int from, int to)
{
	UDWORD id;
	UBYTE *new_palette;
	int num_colours, i;
	FILE *fh = FOPEN(filename,"rb");

	/*
	 * Dig the palette out of the input IFF file
	 */
	ReadHeader(fh, &Header);
	if (Header.ckID != ID_FORM)
		return 1;

	ReadLong(fh, &id);
	switch (id) {
	case ID_ILBM:
		break;

	case ID_PBM:
		break;

	default:
		return 1;
	};

	for (;;) {
		ReadHeader(fh, &Header);

		if (feof(fh))
			return 1;

		if (Header.ckID == ID_CMAP) {
			new_palette = MALLOC(Header.ckSize);
			num_colours = Header.ckSize / 3;
			fread(new_palette, 1, Header.ckSize, fh);
			break;

		} else {
			fseek(fh, ROUNDODDUP(Header.ckSize), SEEK_CUR);
		}
	}

	/*
	 * Assume default if no subrange is given
	 */
	if (num == 0) {
		from = to = 0;
		num = num_colours;
	}
	/*
	 * Paste the new palette into the given bitmap
	 */
	if (from + num > num_colours)
		return 1;

	if (to + num > bmp->npalette)
		return 1;

	for (i = 0; i < num * 3; i++)
		bmp->palette[to * 3 + i] = new_palette[from * 3 + i];

	FREE(new_palette);
	FCLOSE(fh);
	return 0;
}
