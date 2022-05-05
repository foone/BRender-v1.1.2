/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: savescr.c 1.2 1995/08/31 16:28:21 sam Exp $
 * $Locker:  $
 *
 * Save a Material Script file
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "brender.h"
#include "fmt.h"

/*
 * The default material used to load a script
 */
extern br_material _DefaultScriptMaterial;

STATIC struct {
	char *name;
	int value;
} MaterialFlagNames[] = {
	{ "light"				, BR_MATF_LIGHT				},
	{ "prelit"				, BR_MATF_PRELIT			},
	{ "smooth"				, BR_MATF_SMOOTH			},
	{ "environment"			, BR_MATF_ENVIRONMENT_I		},
	{ "environment_local"	, BR_MATF_ENVIRONMENT_L		},
	{ "perspective"			, BR_MATF_PERSPECTIVE		},
	{ "decal"				, BR_MATF_DECAL				},
	{ "always_visible"		, BR_MATF_ALWAYS_VISIBLE	},
	{ "two_sided"			, BR_MATF_TWO_SIDED			},
	{ "force_z_0"			, BR_MATF_FORCE_Z_0			},
	{ "dither"				, BR_MATF_DITHER		 	},
};

STATIC WriteScriptMaterial(br_material *mat, void *df)
{
	int i,j;

	BrFilePrintf(df,"\nmaterial = [\n");
	
	if(mat->identifier)
		BrFilePrintf(df,"    identifier = \"%s\";\n",mat->identifier);

	/*
	 * Flags
	 */
	if(mat->flags != _DefaultScriptMaterial.flags) {
		BrFilePrintf(df,"    flags = [");

		for(i=0,j=0; i < BR_ASIZE(MaterialFlagNames); i++)
			if(mat->flags & MaterialFlagNames[i].value) {
				BrFilePrintf(df,"%s%s",j?", ":"",MaterialFlagNames[i].name);
				j++;
			}

		BrFilePrintf(df,"];\n");
	}

	/*
	 * Lighting parameters
	 */
	if(mat->colour != _DefaultScriptMaterial.colour)
		BrFilePrintf(df,"    colour = [%d,%d,%d];\n",
			BR_RED(mat->colour),
			BR_GRN(mat->colour),
			BR_BLU(mat->colour));

	if(mat->opacity != _DefaultScriptMaterial.opacity)
		BrFilePrintf(df,"    opacity = %d;\n",mat->opacity);

	if(mat->ka != _DefaultScriptMaterial.ka)
		BrFilePrintf(df,"    ambient = %f;\n",BrScalarToFloat(BrUFractionToScalar(mat->ka)));
		
	if(mat->kd != _DefaultScriptMaterial.kd)
		BrFilePrintf(df,"    diffuse = %f;\n",BrScalarToFloat(BrUFractionToScalar(mat->ka)));

	if(mat->ks != _DefaultScriptMaterial.ks)
		BrFilePrintf(df,"    specular = %f;\n",BrScalarToFloat(BrUFractionToScalar(mat->ks)));

	if(mat->power != _DefaultScriptMaterial.power)
		BrFilePrintf(df,"    power = %f;\n",BrScalarToFloat(mat->power));

	if(mat->index_base != _DefaultScriptMaterial.index_base)
		BrFilePrintf(df,"    index_base = %d;\n",mat->index_base);

	if(mat->index_range != _DefaultScriptMaterial.index_range)
		BrFilePrintf(df,"    index_range = %d;\n",mat->index_range);

	/*
	 * Map Transform
	 */
	for(i=0; i < 3; i++)
		if(mat->map_transform.m[i][0] != _DefaultScriptMaterial.map_transform.m[i][0] ||
		   mat->map_transform.m[i][1] != _DefaultScriptMaterial.map_transform.m[i][1])
				break;

	if( i < 3) {
		/*
		 * Matrices were different
		 */
		BrFilePrintf(df,"    map_transform = [\n");
		for(i=0; i < 3; i ++)
			BrFilePrintf(df,
					 "                     [%f,%f]%s\n",
					 BrScalarToFloat(mat->map_transform.m[i][0]),
					 BrScalarToFloat(mat->map_transform.m[i][1]),
					 i != 2?",":"");
		BrFilePrintf(df,"                    ];\n");
	}

	/*
	 * Maps and Tables 
	 */
	if(mat->colour_map && mat->colour_map->identifier)
		BrFilePrintf(df,"    colour_map = \"%s\";\n",mat->colour_map->identifier);

	if(mat->screendoor && mat->screendoor->identifier)
		BrFilePrintf(df,"    screendoor = \"%s\";\n",mat->screendoor->identifier);

	if(mat->index_shade && mat->index_shade->identifier)
		BrFilePrintf(df,"    index_shade = \"%s\";\n",mat->index_shade->identifier);

	if(mat->index_blend && mat->index_blend->identifier)
		BrFilePrintf(df,"    index_blend = \"%s\";\n",mat->index_blend->identifier);

	BrFilePrintf(df,"];\n");
}

/*
 * Write out a material script
 */
br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSaveMany(char *filename,br_material **materials,br_uint_16 num)
{
	void *df;
	int i,count;

	/*
	 * Open file and write header
	 */
	df = BrFileOpenWrite(filename,1);

	if(df == NULL)
		return 0;

	/*
	 * Write a header
	 */
	BrFilePutLine("# BRender Material Script",df);
	BrFilePutLine("#",df);

	if(materials) { 
		for(i=0; i<num; i++)
			WriteScriptMaterial(materials[i],df);
		count = num;
	} else {
		BrMaterialEnum(NULL,(br_material_enum_cbfn *)WriteScriptMaterial,df); 
		count = BrMaterialCount(NULL);
	}

	BrFileClose(df);

	return count;
}

br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSave(char *filename, br_material *ptr)
{
	return BrFmtScriptMaterialSaveMany(filename,&ptr,1);
}
