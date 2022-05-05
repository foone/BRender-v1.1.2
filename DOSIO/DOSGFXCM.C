#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <ctype.h>

#include "brender.h"
#include "brassert.h"
#include "syshost.h"
#include "dosio.h"

#include "dosgfxcm.h"

#if HAS_ENV_H
#include <env.h>
#endif

/*
 * Pointers to default strings
 */
char *_DOSGfxDefaultSetupString = DOSGFX_DEFAULT;
char *_DOSGfxEnvironmentVar = DOSGFX_ENVNAME;

int old_screen_mode;				/* last screen mode */
int _DOSGfxIndexed;

pixelmap_info pixelmap_bit_sizes[]={
   	{
	    	BR_PMT_INDEX_8,
		8,1,
	},
	{
	    	BR_PMT_RGB_555,
		15,2,
	},
	{
	    	BR_PMT_RGB_565,
		16,2,
	},
	{
	    	BR_PMT_RGB_888,
		24,3,
	},
	{
	    	BR_PMT_RGBX_888,
		24,4,
	},
	{
	    	BR_PMT_RGBA_8888,
		32,4,
	},
};

int pixelmap_bit_sizes_count = sizeof(pixelmap_bit_sizes)/sizeof(pixelmap_bit_sizes[0]);

void _ProcessSetupString(char *string,setup_details *details)
{
	char *current_tok;
	int current_value;
	char search_string[80];

	strncpy(search_string,string,BR_ASIZE(search_string)-1);
    
	/*
	 * setup defaults
	 */
	details->display_type = strtok(search_string,",");
	
	for(;;) {
    	current_tok = strtok(NULL,":");

		if(current_tok == NULL)
			break;

		if(isdigit(*current_tok))
		{

			current_value = strtol(current_tok,NULL,0);

			switch(toupper(*(current_tok-2)))
			{
			    	case'W':details->width = current_value;
					break;
				case'H':details->height = current_value;
					break;
				case'B':details->bitsperpixel = current_value;
					break;
				case'C':details->doublebuffertype = current_value;
					break;
				case'M':details->mode = current_value;
					break;
				case'L':details->set_scanline_length = current_value;
					break;
				default:BR_ERROR1("Unknown argument '%c' in setup string",*(current_tok-2));
					break;
			}
		}
	}
}


void _PaletteSet(br_pixelmap *pm)
{
	char *entries;
	int i;

	/*
	 * Don't set palette if in true colour mode
	 * (Some cards use CLUT for gamma corection ramps)
	 */
	if(!_DOSGfxIndexed)
		return;

   	/*
  	 * only one way to set a palette
	 */

	UASSERT(pm->type == BR_PMT_RGBX_888);

	for(i=0, entries=(void *)pm->pixels; i<256; i++, entries += pm->row_bytes)
		_PaletteSetEntry(i,*(br_colour *)entries);
}

void _PaletteSetEntry(int i,br_colour colour)
{
#if 1
	/*
	 * Setup the palette by writing the DAC registers directly
	 */
	outp(VGA_PAL_ADDRESS,i);
	
	outp(VGA_PAL_DATA,BR_RED(colour) >> 2);
	outp(VGA_PAL_DATA,BR_GRN(colour) >> 2);
	outp(VGA_PAL_DATA,BR_BLU(colour) >> 2);
#endif
}



