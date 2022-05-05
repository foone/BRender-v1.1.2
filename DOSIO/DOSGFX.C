/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: dosgfx.c 1.6 1995/03/20 21:31:03 sam Exp $
 * $Locker:  $
 *
 * Mode independant graphics support
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>

#include "fw.h"
#include "brassert.h"
#include "dosio.h"

#include "syshost.h"
#include "dosgfxcm.h"
#include "vesa.h"
#include "mcga.h"
#include "getenv.h"

static char rscid[] = "$Id: dosgfx.c 1.6 1995/03/20 21:31:03 sam Exp $";

typedef br_pixelmap * BR_PUBLIC_ENTRY dosgfx_begin_cbfn(char *new_setup_string);
typedef void BR_PUBLIC_ENTRY dosgfx_end_cbfn(void);

static struct {
   	char *identifier;
	dosgfx_begin_cbfn *begin;
	dosgfx_end_cbfn *end;
} *functions,function_defs[]={
	{
	    "MCGA",
	    DOSGfxBegin_MCGA,
	    DOSGfxEnd_MCGA,
	},
	{
	    "VESA",
	    DOSGfxBegin_VESA,
	    DOSGfxEnd_VESA,
	}
};

br_pixelmap * BR_PUBLIC_ENTRY DOSGfxBegin(char *new_setup_string)
{
	char *display_type;
	char tmp[32];
	char *setup_string;
	char *env_string;
	int i;

	env_string = DOSGetEnv(_DOSGfxEnvironmentVar);

	if(new_setup_string == NULL)
		setup_string = (env_string != NULL)?env_string:NULL;
	else
		setup_string = new_setup_string;

	if(setup_string == NULL)
		setup_string = _DOSGfxDefaultSetupString;
	
	strncpy(tmp,setup_string,BR_ASIZE(tmp)-1);

	display_type = strtok(tmp,",");

	/*
	 * get function table for display type
	 */
	for(i=0; i<BR_ASIZE(function_defs); i++)
		if(!stricmp(function_defs[i].identifier,display_type))
			break;

	if(i >= BR_ASIZE(function_defs))
		BR_ERROR1("Unknown display type %s",display_type);

	functions = &function_defs[i];

	/*
	 * setup for display type
	 */
	return functions->begin(new_setup_string);
}

void BR_PUBLIC_ENTRY DOSGfxEnd(void)
{
    	functions->end();
}

void BR_PUBLIC_ENTRY DOSGfxPaletteSet(br_pixelmap *pm)
{
	_PaletteSet(pm);
}

void BR_PUBLIC_ENTRY DOSGfxPaletteSetEntry(int i,br_colour colour)
{
	_PaletteSetEntry(i,colour);
}

