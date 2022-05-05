/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: bitmap.h 1.1 1995/03/16 12:05:05 sam Exp $
 * $Locker:  $
 *
 */

/*
 * Needed to get output format for sprites
 */

#define BMP_MAX_PLANES	8

struct Bitmap {
	UWORD type;
	UWORD width;
	UWORD height;
	UWORD nplanes;
	UWORD npalette;
	UWORD background;
	UBYTE *palette;
	UBYTE *mask;
	UBYTE *planes[BMP_MAX_PLANES];
};

#define BMT_PLANES			1
#define BMT_BYTEPERPIXEL	2
#define BMT_NIBBLEPERPIXEL	3
#define BMT_2BITSPERPIXEL	4

#define PLANE_WIDTH(x) (2*(((x)->width+15L) >> 4))


#define PLANE_SIZE(x) (PLANE_WIDTH(x) * (x)->height)

#define BYTE_WIDTH(x) (((x)->width+1L) & ~(1L))
#define BYTE_SIZE(x) (BYTE_WIDTH(x) * (x)->height)

struct Rectangle {
	int x;
	int y;
	int w;
	int h;
};

#define SF_SCALED	1	/* Pad sprite to power of two size */
#define SF_HIGH		2	/* Mario special flag			   */

struct Bitmap * NewBitmap(int width, int height, int palette);
void FreeBitmap(struct Bitmap * bmp);
struct Bitmap *ConvertToBytePerPixel(struct Bitmap * bmp);
struct Bitmap *ConvertToPlanes(struct Bitmap * bmp, int num);
void GenMask_Planes(struct Bitmap * bmp);
void CopyBitmap(struct Bitmap *dst,struct Bitmap *src);
struct Bitmap *ExtractSubBitmap(struct Bitmap * bmp, struct Rectangle * rect);
