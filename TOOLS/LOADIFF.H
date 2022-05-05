/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: loadiff.h 1.1 1995/03/16 12:05:07 sam Exp $
 * $Locker:  $
 *
 */
#define MakeID(a,b,c,d) ((a)<<24L | (b)<<16L | (c)<<8 | (d))

#define ID_FORM MakeID('F','O','R','M')
#define ID_ILBM MakeID('I','L','B','M')
#define ID_PBM MakeID('P','B','M',' ')
#define ID_BMHD MakeID('B','M','H','D')
#define ID_CAMG MakeID('C','A','M','G')
#define ID_CMAP MakeID('C','M','A','P')
#define ID_BODY MakeID('B','O','D','Y')

#define MAXCOLOURS 256

#define ROUNDODDUP(a) (((a)+1)&(~1L))


struct Chunk {
	UDWORD ckID, ckSize;
};

struct BitMapHeader {
	UWORD w, h, x, y;
	UBYTE nPlanes, masking, compression, pad1;
	UWORD transparentColor;
	UBYTE xAspect, yAspect;
	UWORD pageWidth, pageHeight;
};

struct Bitmap *ReadPicture_IFF(char *filename);
int ReadPalette_IFF(struct Bitmap * bmp, char *filename, int num, int from, int to);
