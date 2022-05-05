/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Header: p:/tech/q3d/rcs/tools/rawtopix.c 1.2 1995/03/01 16:08:41 sam Exp $
 *
 * $BC<"make -f rawtopix.mak %s.obj;">
 *
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "brender.h"

br_pixelmap *MakeTexture(char *file_name, char *name);

int main(int argc, char **argv)
{
	br_pixelmap *pm;

	BrBegin();

	if(argc != 4) {
		fprintf(stderr,"Converts a 256x256 raw byte per pixel image to a .PIX file\n");
		fprintf(stderr,"Usage: rawtopix <input> <identifier> <output>\n");
		return 0;
	}

	pm = MakeTexture(argv[1],argv[2]);

	BrPixelmapSave(argv[3],pm);

	BrEnd();
	return 0;
}


br_pixelmap *MakeTexture(char *file_name, char *name)
{
 	br_pixelmap *texmap;
 	FILE *fh;

	texmap= BrPixelmapAllocate(BR_PMT_INDEX_8,256,256,NULL,0);
	texmap->identifier = BrResStrDup(texmap,name);

 	fh = BrFileOpenRead(file_name,0,NULL,NULL);

	if(fh == NULL)
		BR_ERROR1("could not open '%s'",file_name);

	BrFileRead(texmap->pixels,256 * 256,1,fh);
	BrFileClose(fh);

	BrMapAdd(texmap);

	return texmap;
}
