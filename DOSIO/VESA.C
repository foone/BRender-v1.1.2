/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesa.c 1.7 1995/07/28 18:58:56 sam Exp $
 * $Locker:  $
 *
 * VESA screen setup
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>

#include "fw.h"
#include "brassert.h"
#include "syshost.h"

#include "vesa.h"
#include "dosgfxcm.h"
#include "realsupt.h"
#include "vesaops.h"
#include "vesaloop.h"
#include "vesaapi.h"
#include "getenv.h"

static char * this_type="VESA";					/*  this type of display */

/*
 * local function prototypes
 */
static int DOSGfxGetMode_VESA(void);
static void DOSGfxSetMode_VESA(int mode, int set_length);
static int DOSGfxFindMode_VESA(setup_details *details);

static int DOSGfxGetSvgaModeInfo(br_uint_16 mode,br_uint_16 rbuf_seg);
static int DOSGfxGetSvgaInfo_VESA(br_uint_16 rbuf_seg);

/*
 * General device structure
 */
br_device _BrVESADevice = {
	{ 0 },
	"VESA Device",
};

/*
 * Context for general vesa modes
 */
struct br_context vesaContext = {
	"VESA Context (Breaks)",
	&_BrVESADevice,
//	&_FontFixed3x5,
	NULL,			// Hawkeye
	0,

	_BrGenericFree,
	_BrGenericMatch,
	_BrGenericClone,
	_BrGenericDoubleBuffer,

	_BrGenericCopy,
	_BrVESABCopyTo,
	_BrVESABCopyFrom,

	_BrVESABClear,
	_BrGenericRectangle,
	_BrGenericRectangle2,
	_BrGenericRectangleCopy,
	_BrVESABRectangleCopyTo,
	_BrVESABRectangleCopyFrom,
	_BrVESABRectangleClear,
	_BrGenericDirtyRectangleCopy,
	_BrGenericDirtyRectangleFill,
	_BrVESABPixelSet,
	_BrVESABPixelGet,
	_BrGenericLine,
	_BrVESABCopyBits,
};

/*
 * Context for vesa modes with no page breaks in scanlines
 */
struct br_context vesaContextNoBreaks = {
	"VESA Context (No Breaks)",
	&_BrVESADevice,
//	&_FontFixed3x5,
	NULL,			// Hawkeye
	0,
	_BrGenericFree,
	_BrGenericMatch,
	_BrGenericClone,
	_BrGenericDoubleBuffer,

	_BrGenericCopy,
	_BrVESACopyTo,
	_BrVESACopyFrom,

	_BrVESAClear,
	_BrGenericRectangle,
	_BrGenericRectangle2,
	_BrGenericRectangleCopy,
	_BrVESARectangleCopyTo,
	_BrVESARectangleCopyFrom,
	_BrVESARectangleClear,
	_BrGenericDirtyRectangleCopy,
	_BrGenericDirtyRectangleFill,
	_BrVESAPixelSet,
	_BrVESAPixelGet,
	_BrGenericLine,
	_BrVESACopyBits,
};

/*
 * Context for vesa modes with linear mapping
 */
struct br_context vesaContextLinear = {
	"VESA Context (Linear)",
	&_BrVESADevice,
//	&_FontFixed3x5,
	NULL,			// Hawkeye
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

extern screen_table * BR_ASM_DATA screen_offset_table;

/*
 * Local copies of vesa information
 */
static struct vesa_info vesa_info;
static struct vesa_modeinfo vesa_modeinfo;
static br_uint_16 modes_table[VESA_MODES_MAX];

br_uint_8 page_breaks;			/* scanline page break flag */

br_pixelmap * BR_PUBLIC_ENTRY DOSGfxBegin_VESA(char *new_setup_string)
{
	setup_details details;
	char *setup_string;
	char *env_string;
	int pmtype;
	br_pixelmap *pm;
	int i;
	int screenmode;
	char *screen;
	br_context *ctx;

	env_string = DOSGetEnv(_DOSGfxEnvironmentVar);

	/*
	 * setup defaults
	 */
	_ProcessSetupString(DOSGFX_DEFAULT_VESA,&details);

	/*
	 * parse new setup string
	 */
	if(new_setup_string == NULL)
		setup_string = (env_string != NULL)?env_string:NULL;
	else
		setup_string = new_setup_string;

	if(setup_string == NULL)
		setup_string = _DOSGfxDefaultSetupString;

	if(setup_string)
		_ProcessSetupString(setup_string,&details);

	if(stricmp(details.display_type,this_type))
		BR_ERROR1("Unknown display type %s",details.display_type);

	/*
	 * set the window address for video memory
	 */
	screen = (br_uint_8 *)(window_start << 4);

	/*
	 * Make sure real mode descriptors are setup
	 */
	_RealSegmentsSetup();

	/*
	 * set screen mode
	 */
	if(_VESABegin())
		BR_ERROR0("Could not setup real mode buffer for VESA query");

	old_screen_mode = DOSGfxGetMode_VESA();

	if((screenmode = DOSGfxFindMode_VESA(&details)) != -1)
		DOSGfxSetMode_VESA(screenmode, details.set_scanline_length);
	else
		BR_ERROR3("Unable to set %d VESA mode %d,%d",details.bitsperpixel,details.width,details.height);
	_VESAEnd();

	pc_gfx_set = 1;
	
	/*
	 * get pixelmap type
	 */
	for(i=0; i<pixelmap_bit_sizes_count; i++)
		if(details.bitsperpixel == pixelmap_bit_sizes[i].bitsperpixel &&
		   details.bytesperpixel == pixelmap_bit_sizes[i].bytesperpixel )
			break;

	if(i >= pixelmap_bit_sizes_count)
		BR_ERROR1("Don't know how to handle %d bit pixelmaps",details.bitsperpixel);

	pmtype = pixelmap_bit_sizes[i].type;

	/*
	 * Flag whether this is an indexed mode or not
	 */
	_DOSGfxIndexed = (pmtype == BR_PMT_INDEX_8);

	/*
	 * allocate screen pixelmap
	 * !! VESA don't allow access to screen
	 */
	pm = BrPixelmapAllocate(pmtype,details.width,details.height,screen,0);
	pm->row_bytes = scanline_size;
	pm->identifier = this_type;
	pm->flags |= BR_PMF_NO_ACCESS;

	/*
	 * set misc thingy-bobs for screen copying,rectangle etc
	 */

	colour_buffer_size = pm->row_bytes;

	if(page_breaks)
		pm->context = ctx = &vesaContext;
	else
		pm->context = ctx = &vesaContextNoBreaks;

#if defined(__DPMI__)
	/*
	 * Use the screen segment
	 */
	ctx->qualifier = _RealSegmentA000;
	pm->pixels = 0x0000;
#else
	ctx->qualifier = _RealSegment0000;
#endif

	window_end = (br_uint_32)((char *)pm->pixels + page_size);

	return pm;
}

void BR_PUBLIC_ENTRY DOSGfxEnd_VESA(void)
{
	REGS_TYPE regs;

   	/*
	 * clean up th' mess
	 */
	regs.WREGS.ax = old_screen_mode;
	INT86(0x10,&regs,&regs);
	pc_gfx_set = 0;
}

static int DOSGfxGetMode_VESA(void)
{
	REGS_TYPE regs;
	
	regs.HREGS.ah = 0x0F;
	INT86(0x10,&regs,&regs);
	return (int)regs.HREGS.al;
}

static void DOSGfxSetMode_VESA(int mode, int set_length)
{
	int power,new_length;
	int i;
	br_uint_16 bytes,pixels,scanlines;
	br_uint_16 bytesperline;

	/*
	 * set video mode
	 */
	_VESAModeSet(mode);

	bytesperline = vesa_modeinfo.bytesperline;
	page_breaks = 1;

	/*
	 * set logical scanline length to next pow(2), so no page breaks in scanline
	 */
	for(power=5, new_length = 2 << power; new_length < vesa_modeinfo.xres * ((vesa_modeinfo.bitsperpixel+1) >> 3); ++power)
		new_length = 2 << power;

	if(((vesa_info.memory*64*1024) > (new_length*vesa_modeinfo.yres)) &&
        (new_length > vesa_modeinfo.bytesperline) && set_length) {
		/*
		 * Attempt to set a more useful stride
		 */
#if 0
	 	if(_VESAScanlineLengthSet(new_length,&bytes,&pixels,&scanlines)) {
			/*
			 * The set failed - set it back to old value, just in case
			 */
			_VESAScanlineLengthSet(vesa_modeinfo.bytesperline,&bytes,&pixels,&scanlines);
		} else {
				bytesperline = new_length;
		}
#else
	 	_VESAScanlineLengthSet(new_length,&bytes,&pixels,&scanlines);
		bytesperline = new_length;
		page_breaks = 0;
#endif
	} else if (new_length == vesa_modeinfo.bytesperline)
    	page_breaks = 0;

	/*
	 * set display start x,y
	 */
	_VESADisplayStartSet(0,0);

	/*
	 * set bankshift for correct paging
	 */
	switch(vesa_modeinfo.win_size/vesa_modeinfo.granularity)
	{
		case(0x0040):bank_shift = 6;
			break;
		case(0x0020):bank_shift = 5;
			 break;
		case(0x0010):bank_shift = 4;
			 break;
		case(0x0008):bank_shift = 3;
			 break;
		case(0x0004):bank_shift = 2;
			 break;
		case(0x0002):bank_shift = 1;
			 break;
		case(0x0000):bank_shift = 0;
			 break;
	}

	bank_increment = 1 << bank_shift;

	/*
	 * setup misc constants for double buffering
	 */
	page_size = vesa_modeinfo.win_size * 1024;

	pixel_size = (vesa_modeinfo.bitsperpixel + 1) >> 3;
	full_banks = vesa_modeinfo.yres*bytesperline / page_size;
	scanline_size = bytesperline;

	if(bytesperline) /* if graphics mode */
		scanlines_per_page = page_size / bytesperline;

	lines_left = vesa_modeinfo.yres-(full_banks*scanlines_per_page);

	/*
	 * set the screen_offset_table
	 * addr and page for each scanline in video memory
	 */
	if(screen_offset_table != NULL) BrMemFree(screen_offset_table);
	screen_offset_table = BrMemCalloc(sizeof(screen_table),vesa_modeinfo.yres,BR_MEMORY_SCRATCH);

	for(i=0; i<vesa_modeinfo.yres; i++)
	{
		screen_offset_table[i].address = (bytesperline*i) % page_size;
		screen_offset_table[i].page_num = ((bytesperline*i) / page_size) << bank_shift;
		screen_offset_table[i].split =
			((1 + page_size - ((bytesperline*i) % page_size)) < bytesperline)?1:0;
	}

	pixelwidth = vesa_modeinfo.xres;
	pixelheight = vesa_modeinfo.yres;
}

static int vesa_bits(struct vesa_modeinfo *mip)
{
	/*
	 * Work out bits per pixel in direct mode by adding the mask sizes
	 */
	if(mip->memorymodel != 6) {
		return mip->bitsperpixel;
	} else {
		return 	mip->redmask + mip->greenmask + mip->bluemask;
	}
}

static int DOSGfxFindMode_VESA(setup_details *details)
{
	int mode_bits;
	br_uint_16 mode;
	int m;

	/*
	 * get svga_info
	 */
	if(_VESAInfo(&vesa_info,modes_table))
		BR_ERROR0("VESA driver not installed");

	/*
	 * check VESA signature
	 */
	if(strncmp((char *)vesa_info.signature,"VESA",4))
		BR_ERROR0("No VESA signature");

	/*
	 * is mode supported?
	 */

	if(details->mode == 0) {
    	/* search for mode */
		for(m=0; modes_table[m] != 0xFFFF; m++) {
			mode = modes_table[m];

			if(_VESAModeInfo(&vesa_modeinfo,mode))
				BR_ERROR0("Unable to read VESA mode information");
	
			mode_bits = vesa_bits(&vesa_modeinfo);

			if((vesa_modeinfo.xres == details->width) &&
				(vesa_modeinfo.yres == details->height) &&
				(mode_bits == details->bitsperpixel)) {
					details->bytesperpixel = (vesa_modeinfo.bitsperpixel + 7)>>3;
					return mode;
					break;
				}
		}

		/*
		 * Could not find mode
		 */
		return -1;

	} else {
	   	/* force mode */
		mode = details->mode;

		if(_VESAModeInfo(&vesa_modeinfo,mode))
			BR_ERROR1("Unable to query VESA mode %d",mode);

		details->width = vesa_modeinfo.xres;
		details->height = vesa_modeinfo.yres;
		details->bitsperpixel = vesa_bits(&vesa_modeinfo);
		details->bytesperpixel = (vesa_modeinfo.bitsperpixel + 7)>>3;
	}

	return mode;
}



