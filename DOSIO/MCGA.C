#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>

#include "fw.h"
#include "brassert.h"

#include "syshost.h"

#include "mcga.h"
#include "dosgfxcm.h"
#include "realsupt.h"
#include "segregs.h"
#include "getenv.h"

static char *this_type="MCGA";					/*  this type of display */

/*
 * table relating screen modes to pixelmap types
 */
static struct {
	int width;
	int height;
	int bitsperpixel;
	int bytesperpixel;
	int mode;
} MCGA_modes[]={
	{
	    	320,200,8,1,0x13,
	},
};

/*
 * local function prototypes
 */
static int DOSGfxGetMode_MCGA(void);
static void DOSGfxSetMode_MCGA(int mode);
static int DOSGfxFindMode_MCGA(setup_details *details);


/*
 * Device structure
 */

br_device _BrMCGADevice = {
	{ 0 },
	"MCGA Device",
};

/*
 * Default context to use for MCGA pixelmaps
 */
br_context mcgaContext = {
	"MCGA Device",
	&_BrMCGADevice,
//	&_FontFixed3x5,
	NULL,				// Hawkeye
	0,

	_BrGenericFree,
	_BrGenericMatch,
	_BrGenericClone,
	_BrGenericDoubleBuffer,

	_BrPmMemCopy,
	_BrPmMemCopy,
	_BrPmMemCopy,

	_BrPmMemFill,
	_BrGenericRectangle,
	_BrGenericRectangle2,
	_BrPmMemRectangleCopyTo,
	_BrPmMemRectangleCopyTo,
	_BrPmMemRectangleCopyFrom,
	_BrPmMemRectangleFill,
	_BrPmMemDirtyRectangleCopy,
	_BrPmMemDirtyRectangleFill,
	_BrPmMemPixelSet,
	_BrPmMemPixelGet,
	_BrPmMemLine,
	_BrPmMemCopyBits,
};

br_pixelmap * BR_PUBLIC_ENTRY DOSGfxBegin_MCGA(char *new_setup_string)
{
	setup_details details;
	char *setup_string;
	char *env_string;
	int pmtype;
	br_pixelmap *pm;
	int i;
	int screenmode;
	unsigned char *screen;

	env_string = DOSGetEnv(_DOSGfxEnvironmentVar);

	/*
	 * setup defaults
	 */
	_ProcessSetupString(DOSGFX_DEFAULT_MCGA,&details);


	/*
	 * parse new setup string
	 */
	if(new_setup_string == NULL)
		setup_string = (env_string != NULL)?env_string:NULL;
	else
		setup_string = new_setup_string;

	if(setup_string)
		_ProcessSetupString(setup_string,&details);


	if(stricmp(details.display_type,this_type))
		BR_ERROR1("Unknown display type %s",details.display_type);
	
	/*
	 * get pixelmap type
	 */
	for(i=0; i<pixelmap_bit_sizes_count; i++)
		if(details.bitsperpixel == pixelmap_bit_sizes[i].bitsperpixel)
			break;

	if(i >= pixelmap_bit_sizes_count)
		BR_ERROR1("Don't know how to handle %d bit pixelmaps",details.bitsperpixel);

	pmtype = pixelmap_bit_sizes[i].type;
	 
	/*
	 * set the window address for video memory
	 */
	screen = (br_uint_8 *)0xA0000;

	/*
	 * Make sure real mode descriptors are setup
	 */
	_RealSegmentsSetup();

	/*
	 * allocate screen pixelmap
	 * MCGA allowa access to screen
	 */
	pm = BrPixelmapAllocate(pmtype,details.width,details.height,screen,0);
	pm->row_bytes = pm->width * pixelmap_bit_sizes[i].bytesperpixel;
	pm->identifier = this_type;
	pm->flags |= BR_PMF_ROW_WHOLEPIXELS | BR_PMF_LINEAR;

#if defined(__DPMI__)
	/*
	 * Use the screen segment
	 */
	mcgaContext.qualifier = _RealSegmentA000;
	pm->pixels = 0x0000;
#else
	mcgaContext.qualifier = _RealSegment0000;
#endif

	/*
	 * set pixelmap functions
	 */
	pm->context = &mcgaContext;

	/*
	 * Set indexed flag
	 */
	_DOSGfxIndexed = 1;

	/*
	 * set screen mode
	 */
	old_screen_mode = DOSGfxGetMode_MCGA();

	if((screenmode = DOSGfxFindMode_MCGA(&details)) != -1) {
		DOSGfxSetMode_MCGA(screenmode);
	} else
		BR_ERROR3("Unable to set %d MCGA mode %d,%d",details.bitsperpixel,details.width,details.height);

	pc_gfx_set = 1;

	return pm;
}

void BR_PUBLIC_ENTRY DOSGfxEnd_MCGA(void)
{
    /*
	 * clean up th' mess
	 */
   	DOSGfxSetMode_MCGA(old_screen_mode);
	pc_gfx_set = 0;
}

static int DOSGfxGetMode_MCGA(void)
{
	REGS_TYPE regs;
	SREGS_TYPE sregs;

	regs.HREGS.ah = 0x0F;
	INT86(0x10,&regs,&regs);
	return (int)regs.HREGS.al;
}
static void DOSGfxSetMode_MCGA(int mode)
{
	REGS_TYPE regs;
	SREGS_TYPE sregs;
	
	regs.WREGS.ax = mode;
#if 1
	INT86(0x10,&regs,&regs);
#endif
}

static int DOSGfxFindMode_MCGA(setup_details *details)
{
    	int i;
	
	for (i=0; i<BR_ASIZE(MCGA_modes); i++)
		if(details->width == MCGA_modes[i].width &&
		  details->height == MCGA_modes[i].height &&
		  details->bitsperpixel == MCGA_modes[i].bitsperpixel)
			break;
			
	if(i >= BR_ASIZE(MCGA_modes))
		return -1;
	else
		return MCGA_modes[i].mode;
}

