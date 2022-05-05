/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Header: p:/tech/q3d/rcs/tools/ifftopix.c 1.3 1995/03/01 16:08:37 sam Exp $
 *
 * $BC<"make -f ifftopix.mak %s.obj;">
 *
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "brender.h"
#include "argstd.h"

#include "bitmap.h"
#include "loadiff.h"

main(int argc, char **argv)
{
	struct Bitmap *bm;
	br_pixelmap *pm;

	BrBegin();

	if(argc != 4) {
		fprintf(stderr,"Converts an IFF/ILBM of PBM image to a INDEX_8 .PIX file of the same size\n");
		printf("Usage: ifftopix <input> <identifier> <output>\n");
		exit(0);
	}

	/*
	 * Read the input picture to a bitmap
	 */
	bm = ReadPicture_IFF(argv[1]);
	
	if(bm->type != BMT_BYTEPERPIXEL)
		BR_ERROR0("Bitmap is not byte per pixel");

	/*
	 * Construct a pixelmap that refers to the same pixels
	 */
	pm = BrPixelmapAllocate(BR_PMT_INDEX_8, BYTE_WIDTH(bm), bm->height, bm->planes[0], 0);
	pm->width = bm->width;
	pm->identifier = argv[2];

	/*
	 * Save the map out with filename
	 */
	BrPixelmapSave(argv[3],pm);

	BrEnd();
	return 0;
}

br_pixelmap *MakeTexture(char *file_name, char *name)
{
 	br_pixelmap *texmap;
 	FILE *fh;

	texmap= BrPixelmapAllocate(BR_PMT_INDEX_8,256,256,NULL,0);
	texmap->identifier = strdup(name);

 	fh = BrFileOpenRead(file_name,0,NULL,NULL);

	if(fh == NULL)
		BR_ERROR1("could not open '%s'",file_name);

	BrFileRead(texmap->pixels,256 * 256,1,fh);
	BrFileClose(fh);

	BrMapAdd(texmap);

	return texmap;
}

