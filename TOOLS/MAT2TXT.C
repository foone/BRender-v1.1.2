/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 */
#include <stdio.h>

#include "brender.h"


br_pixelmap * BR_CALLBACK LoadMapFFHook(char *name)
{
	br_pixelmap *pm;

	pm = BrPixelmapAllocate(BR_PMT_INDEX_8,1,1,NULL,0);
	pm->identifier = BrMemStrDup(name);

	return pm;
}

br_pixelmap * BR_CALLBACK LoadTableFFHook(char *name)
{
	br_pixelmap *pm;

	pm = BrPixelmapAllocate(BR_PMT_INDEX_8,1,1,NULL,0);
	pm->identifier = BrMemStrDup(name);

	return pm;
}

int main(int argc, char ** argv)
{
	br_material *mats[100];
	int n;

	BrBegin();

	BrTableFindHook(LoadTableFFHook);
	BrMapFindHook(LoadMapFFHook);

	n = BrMaterialLoadMany(argv[1],mats,100);

	BrWriteModeSet(BR_FS_MODE_TEXT);

	n = BrMaterialSaveMany(argv[2],mats,n);

	BrEnd();

	return 0;
}
