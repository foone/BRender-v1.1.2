/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: texconv.c 1.6 1995/05/25 13:38:58 sam Exp $
 * $Locker:  $
 *
 * Command line interface for bitmap file format convertion
 *
 * 1.1	1/95	SE	Initial version
 * 1.2	22/2/95	SE	lib update
 *	15/3/95	SE	save as targa
 *			NULL load crash
 *			full paths allowed for load and save
 *			batch file argument length fixed
 *
 */
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "brender.h"
#include "scale.h"
#include "quantize.h"
#include "fmt.h"
#include "datafile.h"

#define SHOW_PICTURES 1

#if SHOW_PICTURES
#include "dosio.h"
#endif

#define VERBOSE 1

#define RED	2
#define GREEN	1
#define BLUE	0

#define TYPE_GUESS -1
#define MAX_ARG_SIZE 256	/* line length in @file */

static br_pixelmap *LoadPIX(char *name,br_uint_32 type);

/*
 * load types
 */
#pragma pack (1)
static br_file_enum_member pixelmap_type_FM[] = {
	_ENUM_MEMBER(BR_PMT_INDEX_1),
	_ENUM_MEMBER(BR_PMT_INDEX_2),
	_ENUM_MEMBER(BR_PMT_INDEX_4),
	_ENUM_MEMBER(BR_PMT_INDEX_8),
	_ENUM_MEMBER(BR_PMT_RGB_555),
	_ENUM_MEMBER(BR_PMT_RGB_565),
	_ENUM_MEMBER(BR_PMT_RGB_888),
	_ENUM_MEMBER(BR_PMT_RGBX_888),
	_ENUM_MEMBER(BR_PMT_RGBA_8888),
	_ENUM_MEMBER(BR_PMT_YUYV_8888),
	_ENUM_MEMBER(BR_PMT_YUV_888),
	_ENUM_MEMBER(BR_PMT_DEPTH_16),
	_ENUM_MEMBER(BR_PMT_DEPTH_32),
	_ENUM_MEMBER(BR_PMT_ALPHA_8),
};

static struct {
	char *name;
	br_pixelmap *(*function)(char *name,br_uint_32 type);
} InputFileTypes[] = {
    	{"image",	LoadPIX},
	{"palette",	LoadPIX},
	{"pixelmap",	LoadPIX},
    	{"pix",		LoadPIX},

	{"bmp",		BrFmtBMPLoad},
	{"rle",		BrFmtBMPLoad},
	{"dib",		BrFmtBMPLoad},
	{"gif",		BrFmtGIFLoad},
	{"tga",		BrFmtTGALoad},
	{"iff",		BrFmtIFFLoad},
	{"lbm",		BrFmtIFFLoad},
};

/*
 * Pixelmap save types
 */
enum {
    	BR_PALETTE,		/* save palette information	*/
	BR_IMAGE,		/* save bitmap information	*/
	BR_PIXELMAP,		/* save pixelmaps		*/
	BR_TARGA,		/* save uncompressed targa	*/
};
static struct {
	char *name;
	br_uint_32 type;
} OutputFileTypes[] = {
    
	{"palette",	BR_PALETTE},	/* save any palette 		: pixelmap->map			*/
	{"pal",		BR_PALETTE},
	
	{"image",	BR_IMAGE},	/* save image without palette	: pixelmap, pixelmap->map=NULL	*/
	
	{"pixelmap",	BR_PIXELMAP},	/* save pixelmap with palette	: pixelmap			*/
	{"pix",		BR_PIXELMAP},	/* save pixelmap with palette	: pixelmap			*/
	
	{"targa",	BR_TARGA},	/* save as (uncompressed) targa					*/
	{"tga",		BR_TARGA},
};

/*
 * get a br_colour from each pixelmap type
 */
br_colour get_pmt_index_8(br_pixelmap *pm,int x,int y,int offset,int byte);
br_colour get_pmt_rgb_555(br_pixelmap *pm,int x,int y,int offset,int byte);
br_colour get_pmt_rgb_565(br_pixelmap *pm,int x,int y,int offset,int byte);
br_colour get_pmt_rgb_888(br_pixelmap *pm,int x,int y,int offset,int byte);
br_colour get_pmt_rgbx_888(br_pixelmap *pm,int x,int y,int offset,int byte);
br_colour get_pmt_rgba_8888(br_pixelmap *pm,int x,int y,int offset,int byte);

/*
 * put a br_colour for each pixelmap type
 */
void put_pmt_rgb_555(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);
void put_pmt_rgb_565(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);
void put_pmt_rgb_888(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);
void put_pmt_rgbx_888(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);
void put_pmt_rgba_8888(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);

/*
 * pixelmap types supported by texconv for gettin' and puttin' br_colours
 */
static struct {
    	int value;
	char *name;
	char *description;
} PixelmapTypes[]={
	{BR_PMT_INDEX_8		,"BR_PMT_INDEX_8","8 bit indexed"},
	{BR_PMT_RGB_555		,"BR_PMT_RGB_555","RGB 16 bit 5 bits per colour"},
	{BR_PMT_RGB_565		,"BR_PMT_RGB_565","RGB 16 bit 5,6,5 bits per colour"},
	{BR_PMT_RGB_888		,"BR_PMT_RGB_888","RGB 24 bit 8 bits per pixel"},
	{BR_PMT_RGBX_888	,"BR_PMT_RGBX_888","RGB 32 bit 8 bits per pixel"},
	{BR_PMT_RGBA_8888	,"BR_PMT_RGBA_8888","RGBA 32 bit 8 bits per component"},
};

static struct {
    	int type;						/* pixelmap type */
    	br_colour (*function)(br_pixelmap *pm,int x,int y,int offset,int byte);	/* function to read pixel from pixelmap */
} GetPixelTypes[]={
	{BR_PMT_INDEX_8	,	get_pmt_index_8,	},
	{BR_PMT_RGB_555	,	get_pmt_rgb_555,	},
	{BR_PMT_RGB_565	,	get_pmt_rgb_565,	},
	{BR_PMT_RGB_888	,	get_pmt_rgb_888,	},
	{BR_PMT_RGBX_888,	get_pmt_rgbx_888,	},
	{BR_PMT_RGBA_8888,	get_pmt_rgba_8888,	},
};

static struct {
    	int type;							/* pixelmap type */
    	void (*function)(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);	/* function to save pixel to pixelmap */
} PutPixelTypes[]={
	{	BR_PMT_INDEX_8	,	NULL,			},
	{	BR_PMT_RGB_555	,	put_pmt_rgb_555,	},
	{	BR_PMT_RGB_565	,	put_pmt_rgb_565,	},
	{	BR_PMT_RGB_888	,	put_pmt_rgb_888,	},
	{	BR_PMT_RGBX_888	,	put_pmt_rgbx_888,	},
	{	BR_PMT_RGBA_8888,	put_pmt_rgba_8888,	},
};

#pragma pack ()

static br_colour (*GetPixel)(br_pixelmap *pm,int x,int y,int offset,int byte);
static void (*PutPixel)(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte);

/*
 * Current import type
 */
static int InputType = TYPE_GUESS;
static int OutputType = TYPE_GUESS;

/*
 * current 32 bit pixelmap type
 */
static br_uint_32 alpha_channel = BR_PMT_RGBX_888;
/*
 * current alpha channel threshold (0-255)
 */
static br_uint_8 alpha_threshold = 127;

/*
 * linked list structure used to process command line args (@file)
 */
static struct list_node {
    	struct list_node *next;
	void *item;
};

/*
 * TGA header, this format allows for structure packing by compiler
 */
#pragma pack(1)

static struct {
    	char identsize;			/* number of bytes between header and whatever next 	*/
	char colourmaptype;		/* type of colourmap in file 0=RGB, 1=palette		*/
    	char imagetype;			/* type of storage of image				*/
    	unsigned short colourmapstart;	/* first colour index used in palette			*/
	unsigned short colourmaplength;	/* number of colours in palette from colourmapstart	*/
	char colourmapbits;		/* size of entry in palette table			*/
    	unsigned short xstart;		/* offset between uppr lft of image & uppr lft of screen*/
	unsigned short ystart;
	unsigned short width;		/* width in pixels					*/
	unsigned short depth;		/* depth in pixels					*/
	char bits;			/* number of bits of colour: 1,8,16,24,32		*/
	char descriptor;			/* bit 5 : set, image stored starting with last line	*/
					/* bit 4 : set, image stored pixels right to left	*/
					/* bits 0-3: number of bits available for overlays (n/a)*/
} TGAheader;

enum {
    	BR_TGA_REVERSED_WIDTH		=	0x10,	/* right to left 	*/
	BR_TGA_REVERSED_DEPTH		=	0x20,	/* bottom to top	*/
};

enum {
    	BR_TGA_UNCOMPRESSED_PALETTE	=	0x01,
	BR_TGA_UNCOMPRESSED_RGB		=	0x02,
	BR_TGA_UNCOMPRESSED_MONO	=	0x03,	/* not supported here	*/
	BR_TGA_COMPRESSED_PALETTE	=	0x09,
	BR_TGA_COMPRESSED_RGB		=	0x0a,
	BR_TGA_COMPRESSED_MONO		=	0x0b,	/* not supported here	*/
};

static unsigned short RGB_16;
static struct {
    	char blue,green,red;
} palette_entry;

#pragma pack()
/*
 * Internal functions
 */
static void SaveTarga(br_pixelmap *pm,char *ident)
{
    	/*
	 * save pixelmap as uncompressed targa file
	 */
	char filename[14];
	int i,bytes;
	FILE *fh;
	char *temp_string;

	switch(pm->type)
	{
	    	case BR_PMT_INDEX_8: TGAheader.colourmaptype = 1;
				     TGAheader.imagetype = BR_TGA_UNCOMPRESSED_PALETTE;
				     TGAheader.colourmapstart = 0;
				     TGAheader.colourmaplength = 256;
				     TGAheader.colourmapbits = 24;
				     TGAheader.bits = 8;
				     break;
				     
		case BR_PMT_RGB_555: TGAheader.colourmaptype = 0;
		 		     TGAheader.imagetype = BR_TGA_UNCOMPRESSED_RGB;
				     TGAheader.bits = 15;
				     TGAheader.identsize = 0;
				     break;
				     
		case BR_PMT_RGB_565: TGAheader.colourmaptype = 0;
		 		     TGAheader.imagetype = BR_TGA_UNCOMPRESSED_RGB;
				     TGAheader.bits = 16;
				     TGAheader.identsize = 0;
				     break;
				     
		case BR_PMT_RGB_888: TGAheader.colourmaptype = 0;
		 		     TGAheader.imagetype = BR_TGA_UNCOMPRESSED_RGB;
				     TGAheader.bits = 24;
				     TGAheader.identsize = 0;
				     break;
				     
		case BR_PMT_RGBX_888: TGAheader.colourmaptype = 0;
		 		     TGAheader.imagetype = BR_TGA_UNCOMPRESSED_RGB;
				     TGAheader.bits = 32;
				     TGAheader.identsize = 0;
				     break;
				     
		case BR_PMT_RGBA_8888: TGAheader.colourmaptype = 0;
		 		     TGAheader.imagetype = BR_TGA_UNCOMPRESSED_RGB;
				     TGAheader.bits = 32;
				     TGAheader.identsize = 0;
				     break;
	}

	bytes = TGAheader.bits >> 3;
	
	TGAheader.xstart = 0;
	TGAheader.ystart = 0;
	TGAheader.width = pm->width;
	TGAheader.depth = pm->height;

	TGAheader.descriptor = 0;
	TGAheader.descriptor = 0;
	
	if(ident != NULL) {
	    	strncpy(filename,ident,13);
	}
	else {
		strcpy(filename,pm->identifier);
		strcat(filename,".tga");
	}

	fh = BrFileOpenWrite(filename,BR_FS_MODE_BINARY);
	/*
	 * save TGAheader
	 */
	BrFileWrite(&TGAheader,sizeof(TGAheader),1,fh);

	/*
	 * need to save palette?
	 */
	if(pm->type == BR_PMT_INDEX_8) {
		if(pm->map == NULL)
			BR_ERROR1("Cannot save '%s' as targa file with no palette",pm->identifier);
			
		for(i=0; i<256; i++) {
			palette_entry.red = BR_RED(((br_colour *)(pm->map->pixels))[i]);
			palette_entry.green = BR_GRN(((br_colour *)(pm->map->pixels))[i]);
			palette_entry.blue = BR_BLU(((br_colour *)(pm->map->pixels))[i]);

			BrFileWrite(&palette_entry,sizeof(palette_entry),1,fh);
		}
	}	

	/*
	 * save pixels
	 */
	for(i=pm->height-1; i>=0 ; i--)
		BrFileWrite(((char *)(pm->pixels))+i*pm->row_bytes,pm->width*bytes,1,fh);

	BrFileClose(fh);

#if VERBOSE
	fprintf(stderr,"Output targa '%s'\n",filename);
#endif
}
 
static char *GetLine(void *fh)
{
	char ch,i,*buffer;
	/*
	 * read one line of text from file
	 */
	buffer = BrMemAllocate(MAX_ARG_SIZE,BR_MEMORY_STRING);

	if(BrFileGetLine(buffer,MAX_ARG_SIZE,fh) == 0)
		return NULL;

	return buffer;
}
static void Push(struct list_node **head,void *item)
{
    	struct list_node *new_rec;
	/*
	 * push a pointer to an item onto the head of a list
	 */
	new_rec = BrMemAllocate(sizeof(struct list_node),BR_MEMORY_SCRATCH);
	new_rec->next = (*head);
	new_rec->item = item;

	(*head) = new_rec;
}
static void Pop(struct list_node **head,void **dest)
{
    	struct list_node *curr_node;
	/*
	 * Pop pointer to item from head of list
	 */
	curr_node = (*head);
    	(*dest) = (*head)->item;
	(*head) = (*head)->next;
	
	BrMemFree(curr_node);
}
static void ProcessArg(int *argc,char ***argv,int *n,int *flag)
{
    	void *fh;
	int open_mode = BR_FS_MODE_TEXT;
	int i,count;
	char *next_line,*sub_arg;
	char **new_argv;
	char *delim = "\n\r ";
	/*
	 * Process command line @file
	 */
	struct list_node *head=NULL;

	fh = BrFileOpenRead( (*argv)[(*n)]+1,0,NULL,&open_mode);

	if(fh == NULL) BR_ERROR1("Could not open command line file '%s' for reading",(*argv)[*n]);

	for (count = 0;;)
	{
	    	/*
		 * for each line in @file
		 */
	    	if((next_line = GetLine(fh)) == NULL)
			break;

	    	if(*next_line != '#')
		{
			sub_arg = strtok(next_line,delim);

			while(sub_arg != NULL)
			{
				if(*sub_arg == '#') break;
				/*
				 * store valid entry and get next
				 */	
				Push(&head,sub_arg);
				count++;
				
				sub_arg = strtok(NULL,delim);
			}
			/*
			 * check for comments in @file
			 */
			if (sub_arg != NULL)
				if (*sub_arg == '#') BrMemFree(sub_arg);
		}
		else BrMemFree(next_line);
	}

	new_argv = malloc(sizeof(*new_argv)*(count+(*argc)+2));

	/*
	 * generate new argv table of all command line args processed so far
	 */
	for (i = 0; i<(*n); i++)
		new_argv[i] = (*argv)[i];

	for (i = (*n)+count-1; i >= (*n); i--)
		Pop(&head,&new_argv[i]);

	for (i = (*n)+1; i < ((*argc)+1); i++)
		new_argv[i+count-1] = (*argv)[i];

	(*argc) = count+(*argc)-1;

	if(*flag) BrMemFree(*argv);
		else *flag = 1;

	(*argv) = new_argv;
	(*n)--;
	
	BrFileClose(fh);
}

#ifdef VERBOSE
void PrintLoadDetails(br_pixelmap *pm)
{
    	/*
	 * print  details of a pixelmap (and palette)
	 */
	fprintf(stderr,"Loaded '%s' as %s (%d,%d)\n",
		pm->identifier,pixelmap_type_FM[pm->type].name,pm->width,pm->height);
	if(pm->map!=NULL)
		fprintf(stderr,"    Palette '%s' %s (%d)\n",
			pm->map->identifier,pixelmap_type_FM[pm->map->type].name,pm->map->height);
}
#endif

static void CopyIdent(br_pixelmap *dst,br_pixelmap *src)
{
    	/*
	 * make copy of pixelmap->identifier
	 */
	dst->identifier = BrMemStrDup(src->identifier);
}
static int PalettePresent(br_pixelmap *pm)
{
    	int found=0;

	/*
	 * check (BR_PMT_INDEX_8) pixelmap->map is a valid palette (BR_PMT_RGBX_888)
	 */
    	if(pm->type == BR_PMT_INDEX_8)
	{
	    	if(pm->map != NULL)
		{
		    	if((pm->map->type == BR_PMT_RGBX_888) && (pm->map->pixels != NULL))
				found = 1;
		}
	}
	else found = 1;
	
	return found;
}
static int CurrentPixelmaps(br_pixelmap ***mtable)
{
    	int nmaps;
	/*
	 * return table of all current pixelmaps in registry
	 */
	nmaps = BrMapCount(NULL);
	if (nmaps != 0)
	{
		*mtable = BrMemCalloc(sizeof(**mtable),nmaps,BR_MEMORY_APPLICATION);

		nmaps = BrMapFindMany(NULL,*mtable,nmaps);
	}
	else BR_ERROR0("No current data available");

	return nmaps;
}
static void BrQuantAddPixelmap(br_pixelmap *pm)
{
	int i,x,y,threshold;
	br_colour colour;
	br_uint_8 rgb[3];
	int offset,byte;
	
	/*
	 * Add a pixelmap to quantizer
	 */
	if(!PalettePresent(pm))
		BR_ERROR1("Indexed '%s' cannot be quantized without palette",pm->identifier);
		
	for(i=0; i<BR_ASIZE(GetPixelTypes); i++)
		if(GetPixelTypes[i].type == pm->type)
		{
			GetPixel = GetPixelTypes[i].function;
			break;
		}

	/*
	 * for all pixels...
	 */
	byte = BrPixelmapPixelSize(pm) >> 3;
	offset = pm->base_y*pm->row_bytes + pm->base_x*byte;
	
	for(y=0; y<pm->height; y++)
		for(x=0; x<pm->width; x++)
		{
			colour = GetPixel(pm,x,y,offset,byte);
			threshold = colour >> 24;

			/*
			 * if top 8 bits of br_colour is aplha, do not insert colour into quantizer
			 */
			if(threshold < alpha_threshold) threshold = 0;

			if(!threshold)
			{			
				rgb[0] = BR_RED(colour);
				rgb[1] = BR_GRN(colour);
				rgb[2] = BR_BLU(colour);

				/*
				 * insert RGB pixel into quantizer
				 */
				BrQuantAddColours(rgb,1);
			}
		}
}
static br_pixelmap *BrQuantMapPixelmap(int base,br_pixelmap *pm)
{
	br_pixelmap *new_pm;
	int x,y,i,threshold;
	br_colour colour;
	br_uint_8 rgb[3];
	int offset,byte;
	/*
	 * generate new pixelmap from quantizer
	 */
	if(!PalettePresent(pm))
		BR_ERROR1("Indexed '%s' cannot be quantized without palette",pm->identifier);
	
	new_pm = BrPixelmapAllocate(BR_PMT_INDEX_8,pm->width,pm->height,NULL,0);

	for(i=0; i<BR_ASIZE(GetPixelTypes); i++)
		if(GetPixelTypes[i].type == pm->type)
		{
			GetPixel = GetPixelTypes[i].function;
			break;
		}

	/*
	 * for all pixels...
	 */
	byte = BrPixelmapPixelSize(pm) >> 3;
	offset = pm->base_y*pm->row_bytes + pm->base_x*byte;
	
	for(y=0; y<pm->height; y++)
		for(x=0; x<pm->width; x++)
		{
		    	colour = GetPixel(pm,x,y,offset,byte);
			/*
			 * if top 8 bits of br_colour contains threshold, check against alpha_threshold
			 * if greater, set index to zero
			 */
			threshold = colour >> 24;
			
			if(threshold < alpha_threshold) threshold = 0;

			if(!threshold)
			{
				rgb[0] = BR_RED(colour);
				rgb[1] = BR_GRN(colour);
				rgb[2] = BR_BLU(colour);
			
				BrQuantMapColours(base,rgb,(char *)new_pm->pixels+y*new_pm->row_bytes+x,1);
			}
			else
				((char *)(new_pm->pixels))[y*new_pm->row_bytes + x] = 0;
		}
	
	return new_pm;
}
static void DoQuantize(br_pixelmap **mtable,int nmaps,int base,int range,br_pixelmap *palette)
{
    	int i;
	br_pixelmap *new_pm;
	/*
	 * perfrom quantize on all pixelmaps in mtable to base,range of entries in palette
	 */
	BrQuantBegin();

	/*
	 * add all pixel data
	 */
	for(i=0; i<nmaps; i++)
	{
#ifdef VERBOSE
		fprintf(stderr,
			"Quantizing '%s' to palette '%s' using %d colours from the range %d-%d\n",
			mtable[i]->identifier,palette->identifier,range,base,base+range-1);
#endif
		BrQuantAddPixelmap(mtable[i]);
	}

	/*
	 * make internal map to new palette
	 */
	BrQuantPrepareMapping(base,range,palette);

	/*
	 * extract pixel data from map of new palette
	 */
	for(i=0; i<nmaps; i++)
	{
	    	new_pm = BrQuantMapPixelmap(base,mtable[i]);
		CopyIdent(new_pm,mtable[i]);
						
		BrMapRemove(mtable[i]);
		BrPixelmapFree(mtable[i]);

		new_pm->map = palette;
	
		BrMapAdd(new_pm);
	}
						
	BrQuantEnd();
}
static br_pixelmap *Remap(int base,int range,br_pixelmap *pm)
{
    	/*
	 * remap without palette information, just re-index 
	 */
	br_pixelmap *new_pm;
	int x,y,byte,offset;
	char *src,*dst;
	
	byte = BrPixelmapPixelSize(pm) >> 3;
	offset = pm->base_y*pm->row_bytes + pm->base_x*byte;

	new_pm = BrPixelmapAllocate(BR_PMT_INDEX_8,pm->width,pm->height,NULL,0);
	for(y=0, src=pm->pixels, dst=new_pm->pixels ; y<pm->height; y++, src+=pm->row_bytes, dst+=new_pm->row_bytes)
		for(x=0; x<pm->width; x++)
		{
			if((dst[x] = src[x] + base) > base+range-1)
				BR_ERROR1("Unable to reduce '%s' colour range without palette",pm->identifier);
			
		}

	return new_pm;
}
static void DoRemap(br_pixelmap **mtable,int nmaps,int base,int range)
{
    	int i;
	br_pixelmap *palette,*new_pm;
	/*
	 * remap pixelmaps in mtable, quantize if necessary to base,range
	 * cumulative effect, quantize all pixelmaps to one palette
	 */
	BrQuantBegin();
						
	palette = BrPixelmapAllocate(BR_PMT_RGBX_888,1,256,NULL,0);
	palette->identifier = "palette";

	for(i=0; i<nmaps; i++)
	{
#ifdef VERBOSE
		fprintf(stderr,
			"Remapping '%s' to the range %d-%d",
			mtable[i]->identifier,base,base+range-1);
#endif
		/*
		 * only add pixel data if there is a palette to find rgb
		 */
		if(PalettePresent(mtable[i]))
		{
#ifdef VERBOSE
			fprintf(stderr," (colour reduction)");
#endif							    
			BrQuantAddPixelmap(mtable[i]);
		}
#ifdef VERBOSE
		fprintf(stderr,"\n");
#endif							
	}

	BrQuantMakePalette(base,range,palette);
	BrQuantPrepareMapping(base,range,palette);

	for(i=0; i<nmaps; i++)
	{
		if(PalettePresent(mtable[i]))
		{
		    	/*
			 * quantize
			 */
		    	new_pm = BrQuantMapPixelmap(base,mtable[i]);
			new_pm->map = palette;
		}
		else
			/*
			 * if no palette present, just do remap of index
			 */
			new_pm = Remap(base,range,mtable[i]);

		CopyIdent(new_pm,mtable[i]);
								
		BrMapRemove(mtable[i]);
		BrPixelmapFree(mtable[i]);
	
		BrMapAdd(new_pm);
	}
	
	BrQuantEnd();
}
static br_pixelmap *DoConvert(br_pixelmap *src,int new_type)
{
    	int x,y,i;
	br_pixelmap *dst,*palette;
	br_colour pixel;
	int src_offset,src_byte;
	int dst_offset,dst_byte;

	/*
	 * convert from one pixelmap type to another
	 */

	/*
	 * is current source type supported?
	 */
	for(i=0; i<BR_ASIZE(GetPixelTypes); i++)
		if(GetPixelTypes[i].type == src->type)
		{
			GetPixel = GetPixelTypes[i].function;
			break;
		}
	if(i >= BR_ASIZE(GetPixelTypes))
		BR_ERROR1("Unknown pixelmap type",src->type);
	/*
	 * is current destination type supported?
	 */
	for(i=0; i<BR_ASIZE(PutPixelTypes); i++)
		if(PutPixelTypes[i].type == new_type)
		{
			PutPixel = PutPixelTypes[i].function;
			break;
		}
	if(i >= BR_ASIZE(PutPixelTypes))
		BR_ERROR1("Unknown pixelmap type",new_type);

	/*
	 * if source is BR_PMT_INDEX_8, cannot convert without palette information
	 */
	if(!PalettePresent(src))
		BR_ERROR1("Indexed '%s' cannot be converted without palette",src->identifier);

	if(new_type == BR_PMT_INDEX_8)
	{
	    	/*
		 * if converting to BR_PMT_INDEX_8, do a quantize
		 */
	    	BrQuantBegin();
		
		palette = BrPixelmapAllocate(BR_PMT_RGBX_888,1,256,NULL,0);
		palette->identifier = BrMemAllocate(strlen(src->identifier)+4,BR_MEMORY_STRING);
		strcpy(palette->identifier,src->identifier);
		strcat(palette->identifier,"pal");
		
		/*
		 * Add a pixelmap, make palette
		 */
		BrQuantAddPixelmap(src);
		BrQuantMakePalette(0,256,palette);
		BrQuantPrepareMapping(0,256,palette);

		/*
		 * make new pixelmap from quantized palette, assign new palette
		 */
	    	dst = BrQuantMapPixelmap(0,src);
		dst->map = palette;

		BrQuantEnd();
	}
	else
	{
		/*
		 * convert to any other pixelmap type
		 */
		dst = BrPixelmapAllocate(new_type,src->width,src->height,NULL,0);
	
		/*
		 * copy pixels from one to t'other
		 * munch pixels, spit out in other format (without drooling too much on the data)
		 */
		src_byte = BrPixelmapPixelSize(src) >> 3;
		src_offset = src->base_y*src->row_bytes + src->base_x*src_byte;
		dst_byte = BrPixelmapPixelSize(dst) >> 3;
		dst_offset = dst->base_y*dst->row_bytes + dst->base_x*dst_byte;
		
		for(y=0; y<src->height; y++)
			for(x=0; x<src->width; x++)
				PutPixel(dst,GetPixel(src,x,y,src_offset,src_byte),x,y,dst_offset,dst_byte);
	}

	CopyIdent(dst,src);

	return dst;
}				
static int SortMapTable(br_pixelmap **mtable,int nmaps)
{
    	int i,j;
	br_pixelmap *tmp;
	
	/*
	 * regenerate table of pixelmaps if it contains NULL entries
	 */
	for(i=0; i<nmaps; i++)
	    	if(mtable[i]==NULL)
		    	for(j=i; j<nmaps-1; j++)
			{
			    	tmp=mtable[j];
				mtable[j] = mtable[j+1];
				mtable[j+1] = tmp;
			}

	for(i=0;i<nmaps;i++)
	    	if(mtable[i] == NULL)
		    return i;

	return i;
}
static br_pixelmap *LoadPIX(char *filename,br_uint_32 type)
{
	br_pixelmap *pm;

   	/*
	 * Load pixelmap
	 */
	pm = BrPixelmapLoad(filename);

	if(pm == NULL)
		BR_ERROR1("Could not load pixelmap: '%s'",filename);

	return pm;
}
static void OutputPIX(char *filename,br_uint_32 type)
{
	br_pixelmap *pm;
	br_pixelmap **mtable,**oldmtable,**tmpmtable;
	int nmaps,n,i;
	void *fh;
	br_colour colour;
	char buf[15];
	/*
	 * save pixelmap or palette 
	 */
	nmaps = BrMapCount(NULL);

	if (nmaps == 0)
		BR_ERROR0("No current data available");

	mtable = BrMemCalloc(sizeof(*mtable),nmaps,BR_MEMORY_APPLICATION);
	oldmtable = BrMemCalloc(sizeof(*oldmtable),nmaps,BR_MEMORY_APPLICATION);
	tmpmtable = BrMemCalloc(sizeof(*tmpmtable),nmaps,BR_MEMORY_APPLICATION);

	/*
	 * get all registered pixelmaps
	 */
	nmaps = BrMapFindMany(NULL,mtable,nmaps);

	/*
	 * take a copy of this table of pointers
	 */
	memcpy(oldmtable,mtable,nmaps * sizeof(*mtable));

	for(i=0; i<nmaps; i++)
	    	switch(type)
		{
		    	case(BR_PALETTE):tmpmtable[i] = mtable[i];	/* save old pixelmap	*/
					mtable[i] = mtable[i]->map;	/* pixelmap -> palette	*/
					break;
			
			case(BR_IMAGE):tmpmtable[i] = mtable[i]->map;	/* save palette		*/
					mtable[i]->map = NULL;		/* pixelmap->map = NULL */
					break;
		}
	/*
	 * remove any NULL entries in mtable
	 */
	n = SortMapTable(mtable,nmaps);
	
	if(n != 0)
	    	switch(type)
		{
			default:BrPixelmapSaveMany(filename,mtable,n);		/* save all pixelmap data */
				break;
			case(BR_PALETTE):BrPixelmapSave(filename,*mtable);	/* only save first palette in mtable */
				break;
			case(BR_TARGA):if(nmaps == 1) {
						SaveTarga(*mtable,filename);
					}
					else
						for(i=0; i<n; i++)
							SaveTarga(mtable[i],NULL);		/* save all as targa */
				break;
		}
#ifdef VERBOSE
	else
		fprintf(stderr,"Nothing saved\n");
#endif

	/*
	 * restore mtable
	 */
	memcpy(mtable,oldmtable,nmaps * sizeof(*mtable));
	
	/*
	 * restore pixelmap links to palettes if necessary
	 */
	for(i=0; i<nmaps; i++)
	    	switch(type)
		{
		    	case(BR_PALETTE):mtable[i] = tmpmtable[i];
					break;
			case(BR_IMAGE):mtable[i]->map = tmpmtable[i];	/* restore palette	*/
					break;
		}

	BrMemFree(mtable);
	BrMemFree(oldmtable);
	BrMemFree(tmpmtable);

#if VERBOSE
	fprintf(stderr,"Output pixelmap '%s'\n",filename);
#endif

}
void ShowPiccy(br_pixelmap *pm)
{
    	int i,j,high,wide;
	br_colour colour,*palette_entry;
	br_pixelmap *buffer;
	/*
	 * display 320x200 top left of BR_PMT_INDEX_8 pixelmaps
	 */
#if SHOW_PICTURES
	buffer = DOSGfxBegin(NULL);

	if(buffer->type == pm->type)
	{
		wide=(pm->width>buffer->width)?buffer->width:pm->width;
		high=(pm->height>buffer->height)?buffer->height:pm->height;
		
		if(pm->map != NULL)
			DOSGfxPaletteSet(pm->map);
	
		
		BrPixelmapRectangleCopy(buffer,0,0,pm,0,0,wide,high);
		getch();

	}
	DOSGfxEnd();
#else
	fprintf(stderr,"Cannot display image\n");
#endif
}

static br_pixelmap *FlipY(br_pixelmap *src)
{
    	br_pixelmap *dst;
	int i;

	/*
	 * flip top/bottom
	 */

	dst = BrPixelmapClone(src);
	if(src->map != NULL) dst->map = BrPixelmapClone(src->map);
	dst->identifier = BrMemStrDup(src->identifier);

	for(i=0; i<src->height; i++)
		memcpy(
		((char *)(dst->pixels)) + (dst->height-i-1)*dst->row_bytes,
		((char *)(src->pixels)) + i*src->row_bytes, src->row_bytes);

	fprintf(stderr,"Flipped '%s' top/bottom\n",dst->identifier);
	
    	return dst;
}

static br_pixelmap *FlipX(br_pixelmap *src)
{
    	br_pixelmap *dst;
	int i;
	int src_byte,src_offset,dst_byte,dst_offset;
	int x,y;

	/*
	 * flip left/right
	 */

	dst = BrPixelmapClone(src);
	if(src->map != NULL) dst->map = BrPixelmapClone(src->map);
	dst->identifier = BrMemStrDup(src->identifier);

	/*
	 * copy pixels from one to t'other
	 * munch pixels, spit out in other format (without drooling too much on the data)
	 */
	src_byte = BrPixelmapPixelSize(src) >> 3;
	src_offset = src->base_y*src->row_bytes + src->base_x*src_byte;
	dst_byte = BrPixelmapPixelSize(dst) >> 3;
	dst_offset = dst->base_y*dst->row_bytes + dst->base_x*dst_byte;
		
	for(y=0; y<src->height; y++)
		for(x=0; x<src->width; x++)
			memcpy(
				((char *)(dst->pixels)) + y*dst->row_bytes + (dst->width-1-x)*dst_byte + dst_offset,
				((char *)(src->pixels)) + y*src->row_bytes + x*src_byte + src_offset,
				src_byte);
			
	fprintf(stderr,"Flipped '%s' left/right\n",dst->identifier);
		
	return dst;
}

int main(int argc, char **argv)
{
	char	*cp;
	int n,new_cm;

	BR_BANNER("TEXCONV","1994","$Revision: 1.6 $");

	/*
	 * Initialise framework
	 */
	BrBegin();

	/*
	 * Process command line to expand @files
	 */

	for (n = 1, new_cm = 0; n<argc; n++)
	{
		if (argv[n][0] == '@')
			ProcessArg(&argc,&argv,&n,&new_cm);
	}

	/*
 	 * Process argument list in order
	 */
	while (argv++, --argc)
	{
		if (**argv == '-')
		{
			/*
			 * Got one or more flags
			 */
			for (cp = *argv + 1; *cp; cp++)
			{

				if (strchr("IOPQRScnor", *cp))
				{
					argv++;
					if(--argc == 0)
						BR_ERROR1("The %c option requires an argument", *cp);
				}

				switch (*cp)
				{
					default:
						BR_ERROR1("unknown option '%c'\n", *cp);
						break;
					/*
					 * Usage
					 */
					case '?':{
					    	int i;
						
						fprintf(stderr,
							"\n"
							"Usage: texconv {options}\n"
							"\n"
							"Options are treated as commands executed in left-to-right order -\n"
							"\n"
							"   <input-file>             Load a file into current data\n"
							"   -I <input-type>          Set input file type\n"
							"   -O <output-type>         Set output file type\n"
							"\n"
							"   -a                       Toggle current 32 bit pixelmaps to exclude/include\n"
							"			     alpha data (default exclude)\n"
							"   -c <pixelmap-type>[,t]   Convert to pixelmap type, may involve quantizing.\n"
							"                            t is the alpha channel threshold (0-255) for\n"
							"                            conversions involving 32 bit pixelmaps.\n"
							"   -f                       'Forget' all current data\n"
							"   -n <name>                Assign identifier 'name' to data\n"
							"   -o <file>                Generate output file from current data\n"
							"   -r <file>,<pixelmap-type>,offset,x,y[,P]\n"
							"                            Load raw data file as pixelmap-type,\n"
							"                            with pixel dimensions x,y, from offset into file\n"
							"                            P is specified to load as a palette\n"
							"   -v                       view snapshot of current data (only BR_PMT_INDEX_8)\n"
							"   -x                       flip left/right\n"
							"   -y                       flip top/bottom\n"
							"\n"
							"   -P <name>[,RAW]          Apply palette from a file to all current indexed\n"
							"                            pixelmaps. If RAW is specified, the palette file\n"
							"                            is 768 byte rgb, otherwise pixelmap format\n"
							"   -Q <name>[,b,r]          Quantize to palette supplied (pixelmap format)\n"
							"                            using (b)ase and (r)ange palette entries\n"
							"   -R b,r                   Quantize and remap to (b)ase,(r)ange colours.\n"
							"                            (0 <= b,r <256)\n"
							"   -S x,y[,<float>]         Scale to new x,y dimensions using optional filter\n"
							"                            size f (default 3.0)\n"
							"   @file                    perform all operation contained within <file>"
							);
	
						fprintf(stderr,
							"\n"
							"   <input-type> =\n"
							"            pix    - BRender Pixelmap format\n"
							"            bmp    - Windows BMP format   (4,8,24,RLE4,RLE8)\n"
							"            gif    - CompuServ GIF format (1< x <=8 bit)\n"
							"            tga    - Targa TGA format     (8,15,16,24,32 bit)\n"
							"            iff    - Amiga IFF/LBM format (1< x <=8 bit)\n"
							"\n"
							"   If a type is not specified, it will be guessed from the extension\n"
							"   of the filename.\n");
							
						fprintf(stderr,
							"\n"
							"   <output-type> =\n"
							"            palette         Palette information stripped from bitmap (.pix)\n"
							"            image           Pixelmap without palette (.pix)\n"
							"            pixelmap        Image with any palette information (.pix)\n"
							"            targa           Output as .tga file\n"
							"\n"
							"   If a type is not specified, the default is pixelmap\n");
						fprintf(stderr,
							"\n"
							"   <pixelmap-type> =\n");

						for(i=0; i<BR_ASIZE(PixelmapTypes); i++)
						{
						    	fprintf(stderr,
								"            %s   (%s)\n",
								PixelmapTypes[i].name,
								PixelmapTypes[i].description);
						}
						break;
					}

					/*
					 * -I <input-type>, set input file type
					 */
					case 'I': {
						int i;

						for(i=0; i < BR_ASIZE(InputFileTypes) ; i++)
							if(!stricmp(*argv,InputFileTypes[i].name))
								break;

						if(i >= BR_ASIZE(InputFileTypes))
							BR_ERROR1("Unknown input type \"%s\"",*argv);
	
						InputType = i;
						break;
						}
					/*
					 * -O <output-type>, set output file type
					 */
					case 'O': {
						int i;
	
						for(i=0; i < BR_ASIZE(OutputFileTypes) ; i++)
							if(!stricmp(*argv,OutputFileTypes[i].name))
								break;
	
						if(i >= BR_ASIZE(OutputFileTypes))
							BR_ERROR1("Unknown output type \"%s\"",*argv);
	
						OutputType = i;
						break;
					}
					/*
					 * -P <name>[,RAW], set indexed maps to have specified [raw] palette
					 */
					case 'P': {
						br_pixelmap *pm,*palette,**mtable;
						int nmaps,i,y,r = 0,open_mode = BR_FS_MODE_BINARY;
						char *filename,*raw,p[3],*delim = ",";
						br_colour *palette_entry,colour;
						void *fh;
						

						filename = strtok(*argv,delim);
						raw = strtok(NULL,delim);

						if (raw!=NULL) if(!stricmp(raw,"RAW")) r = 1;

						if(r)
						{
						    	/*
							 * Load palette as 768 byte raw RGB
							 */
						    	palette = BrPixelmapAllocate(BR_PMT_RGBX_888,1,256,NULL,0);
							palette->identifier = BrMemStrDup(filename);
							palette->identifier = strtok(palette->identifier,".");

							if((fh = BrFileOpenRead(filename,0,NULL,&open_mode)) == NULL)
								BR_ERROR1("Unable to open raw palette file '%s'",filename);
							
							for(y=0, palette_entry = palette->pixels; y<256; y++,palette_entry++)
							{
								if(BrFileRead(p,1,3,fh) != 3)
									BR_ERROR1("Unable to read raw palette from '%s'",filename);
									
								colour = BR_COLOUR_RGB(p[0],p[1],p[2]);
								*palette_entry = colour;
							}
							BrFileClose(fh);
						}
						else
						{
						    	/*
							 * load palette as BR_PMT_RGBX_888
							 */
							palette = BrPixelmapLoad(*argv);

							if(palette == NULL)
								BR_ERROR1("Could not load palette: '%s'",*argv);

							palette = (palette->map == NULL)?palette:palette->map;
							if(palette->type != BR_PMT_RGBX_888)
								BR_ERROR1("'%s' does not contain a palette",*argv);
						}

						nmaps = CurrentPixelmaps(&mtable);

						for(i=0; i<nmaps; i++)
							/*
							 * apply new palette to all BR_PMT_INDEX_8
							 */
						    	if(mtable[i]->type == BR_PMT_INDEX_8)
							{
								pm = BrPixelmapClone(mtable[i]);
								BrPixelmapCopy(pm,mtable[i]);
								pm->map = palette;
								
								CopyIdent(pm,mtable[i]);
							    
#ifdef VERBOSE
								fprintf(stderr,"'%s' assigned palette '%s'\n",
									pm->identifier,
									palette->identifier);
#endif
								BrMapRemove(mtable[i]);
								BrPixelmapFree(mtable[i]);
								BrMapAdd(pm);
							}

						BrMemFree(mtable);
						
					    	break;
					}
					/*
					 * -Q <name>[,b,r], quantize to base,range in palette
					 */
					case 'Q': {
					    	br_pixelmap *palette,**mtable;
						int nmaps,i,base,range;
						char *pal_name,*next;
						char *delim = ",";

						pal_name = strtok(*argv,delim);
						next = strtok(NULL,delim);
						if(next != NULL)
						{
							i = sscanf(next,"%5d",&base);
							next = strtok(NULL,delim);
							if (next != NULL)
								i += sscanf(next,"%5d",&range);
						}
						else i = 0;
						
						if (i < 2)
						{
							base = 0;
							range = 256;
						}
						if ((base < 0 ) || (base >255))
							BR_ERROR0("Base value out of range 0-255");
						if ((range < 1) || (range >256))
							BR_ERROR0("Range value out of range 1-256");
						if (base + range - 1 > 255)
							BR_ERROR0("Too many colours specified for -Q");
						
						palette = BrPixelmapLoad(pal_name);

						if(palette == NULL)
							BR_ERROR1("Could not load palette: '%s'",pal_name);

						if(palette->map != NULL) palette = palette->map;
						
						if(palette->type != BR_PMT_RGBX_888)
							BR_ERROR1("'%s' does not contain a valid palette",pal_name);
						
						nmaps = CurrentPixelmaps(&mtable);

						/*
						 * quantize all entries in mtable to given palette
						 */
						DoQuantize(mtable,nmaps,base,range,palette);

						BrMemFree(mtable);

					    	break;
					}
					/*
					 * -R b,r , remap to base,range.
					 * may invlove quantizing.
					 */
					case 'R': {
					    	br_pixelmap *new_palette,**mtable,*new_pm;
						int nmaps,i,base,range;

						i = sscanf(*argv,"%5d,%5d",&base,&range);
						
						if (i != 2)
							BR_ERROR0("Incorrect number of parameters for -R");
						if ((base < 0 ) || (base >255))
							BR_ERROR0("Base value out of range 0-255");
						if ((range < 1) || (range >256))
							BR_ERROR0("Range value out of range 1-256");
						if (base + range - 1 > 255)
							BR_ERROR0("Too many colours specified for -R");

						nmaps = CurrentPixelmaps(&mtable);

						DoRemap(mtable,nmaps,base,range);

						BrMemFree(mtable);
						
					    	break;
					}
					/*
					 * -S x,y[,f]  scale to new x,y with filter size (float) f
					 */
					case 'S': {
					    	br_pixelmap *new_palette,**mtable,*pm,*new_pm;
						int nmaps,i,new_x,new_y;
						float fwidth = 3.0;

						i = sscanf(*argv,"%5d,%5d,%f",&new_x,&new_y,&fwidth);
						
						if (i < 2)
							BR_ERROR0("Incorrect number of parameters for -S");
						if ((new_x < 0) || (new_y < 0))
							BR_ERROR0("Cannot scale to negative size");

						BrScaleBegin();

						nmaps = CurrentPixelmaps(&mtable);

						for(i=0; i<nmaps; i++)
						{
							if(!PalettePresent(mtable[i]))
								BR_ERROR1("Indexed '%s' cannot be scaled without palette",
									mtable[i]->identifier);

							/*
							 * convert pixelmap to RGB_888 (24 bit rgb)
							 */
						    	pm = DoConvert(mtable[i],BR_PMT_RGB_888);

							/*
							 * scale rgb image
							 */
						    	new_pm = BrPixelmapScale(pm,new_x,new_y,fwidth);

							BrPixelmapFree(pm);

							/*
							 * convert to old type (may involve quantize)
							 */
							pm = DoConvert(new_pm,mtable[i]->type);

							BrPixelmapFree(new_pm);
							
#ifdef VERBOSE
							fprintf(stderr,"Scaled '%s' from %d,%d to %d,%d\n",
								mtable[i]->identifier,
								mtable[i]->width,
								mtable[i]->height,
								new_x,
								new_y);
#endif

							BrMapRemove(mtable[i]);
							BrPixelmapFree(mtable[i]);

							BrMapAdd(pm);
						}

						BrScaleEnd();

					    	break;
					}
					/*
					 * -a  toggle 32 bit bitmaps between BR_PMT_RGBA_8888 or BR_PMT_RGBX_888
					 */
					case 'a': {
						br_pixelmap **mtable;
						int nmaps,i;
						
					    	alpha_channel = (BR_PMT_RGBX_888)?BR_PMT_RGBA_8888:BR_PMT_RGBX_888;

						nmaps = CurrentPixelmaps(&mtable);

						for(i=0; i<nmaps; i++)
							if(mtable[i]->type != alpha_channel)
							{
							    	mtable[i]->type = alpha_channel;							
#ifdef VERBOSE
								fprintf(stderr,"'%s' type changed to %s\n",
								mtable[i]->identifier,
								pixelmap_type_FM[mtable[i]->type].name);
#endif
							}

						BrMemFree(mtable);
						
					    	break;
					}
					/*
					 * -c <pixelmap-type>[,threshold]
					 * convert pixelmap from one type to th'other, may involve quantize
					 * with alpha threshold
					 */
					case 'c':{
						br_pixelmap **mtable,*pm;
						int nmaps,i,new_type,t;
						char *conv_type,*next;
						char *delim=",";

						conv_type = strtok(*argv,delim);
						next = strtok(NULL,delim);
						if(next!=NULL)
							i = sscanf((next),"%5d",&t);
						else i=0;

						if(i == 1)
						{
						    	if((t>=0) && (t<256))
							    	alpha_threshold = t;
							else
								BR_ERROR1("Alpha channel threshold %d too large (0-255)",t);
						}
						else
							alpha_threshold = 127;

						for(i=0; i<BR_ASIZE(PixelmapTypes); i++)
							if(!stricmp(*argv,PixelmapTypes[i].name))
							{
							    	new_type = PixelmapTypes[i].value;
							    	break;
							}
						if(i >= BR_ASIZE(PixelmapTypes))
							BR_ERROR1("Unknown pixelmap type '%s'",*argv);
						nmaps = CurrentPixelmaps(&mtable);
						for(i=0; i<nmaps; i++)
						{							
							pm = DoConvert(mtable[i],new_type);
#ifdef VERBOSE
							fprintf(stderr,"Converted '%s' %s (%d bit) to %s (%d bit)\n",
								mtable[i]->identifier,
								pixelmap_type_FM[mtable[i]->type].name,
								BrPixelmapPixelSize(mtable[i]),
								pixelmap_type_FM[new_type].name,
								BrPixelmapPixelSize(pm));
#endif
							BrMapRemove(mtable[i]);
							BrPixelmapFree(mtable[i]);

							BrMapAdd(pm);
						}

						BrMemFree(mtable);
						
					    	break;
					}
					/*
					 * -f   forget all current data
					 */
					case 'f': {
						br_pixelmap **mtable;
						int nmaps,i;
						
					    	alpha_channel = BR_PMT_RGBX_888;
						
						InputType = TYPE_GUESS;
						OutputType = TYPE_GUESS;
						
						nmaps = CurrentPixelmaps(&mtable);

						for(i=0; i<nmaps; i++)
						{
						    	BrMapRemove(mtable[i]);
							BrPixelmapFree(mtable[i]);
						}
						BrMemFree(mtable);
#ifdef VERBOSE
						fprintf(stderr,"Current data 'forgotten'\n");
#endif
						
					    	break;
					}
					/*
					 * -n <name>   set pixelmap identifiers to 'name'
					 */
					case 'n':{
					    	br_pixelmap **mtable;
						int nmaps,i;

						nmaps = CurrentPixelmaps(&mtable);

						for(i=0; i<nmaps; i++)
						{
#ifdef VERBOSE
							fprintf(stderr,"'%s' assigned new identifier '%s'\n",
								mtable[i]->identifier,
								*argv);
#endif
							mtable[i]->identifier = *argv;
						}

						BrMemFree(mtable);
						
					    	break;
					}
					/*
					 * -r <name>,<pixelmap-type>,offset,x,y  load raw file as pixelmap-type size x,y
					 */
					case 'r':{
					    	br_pixelmap *pm;
						char *filename,*conv_type,pixel_buffer[4];
						char *delim = ",",*next;
						int i,x,y,type,byte,offset,open_mode = BR_FS_MODE_BINARY;
						void *fh;

						filename = strtok(*argv,delim);
						conv_type = strtok(NULL,delim);
						next = strtok(NULL,delim);
						if (next != NULL)
						    	i = sscanf(next,"%5d",&offset);
						else i = 0;
						if (i==0) BR_ERROR0("Offset required for -r");
						
						next = strtok(NULL,delim);
						if (next != NULL)
						{
							i = sscanf(next,"%5d",&x);
							next = strtok(NULL,delim);
							if(next != NULL)
								i += sscanf(next,"%5d",&y);
						}
						else i = 0;

						if(filename == NULL | conv_type == NULL | i!=2)
						   BR_ERROR0("Incorrect number of arguments for -r");

						for(i=0; i<BR_ASIZE(PixelmapTypes); i++)
							if(!stricmp(conv_type,PixelmapTypes[i].name))
							{
							    	type = PixelmapTypes[i].value;
							    	break;
							}
						if(i >= BR_ASIZE(PixelmapTypes))
							BR_ERROR1("Unknown pixelmap type '%s'",conv_type);

						if((fh = BrFileOpenRead(filename,0,NULL,&open_mode)) == NULL)
							BR_ERROR1("Unable to open '%s'",filename);

						BrFileAdvance(offset,fh);

						pm = BrPixelmapAllocate(type,x,y,NULL,0);
						
						pm->identifier = BrMemStrDup(filename);
						pm->identifier = strtok(pm->identifier,".");

						byte = BrPixelmapPixelSize(pm) >> 3;

						for(y=0; y<pm->height; y++)
							for(x=0; x<pm->width; x++)
							{
							    	if (BrFileRead(pixel_buffer,1,byte,fh) != byte)
									BR_ERROR4("Unable to load raw data '%s' as %s (%d,%d)",
										filename,
										conv_type,
										pm->width,
										pm->height);
								for(i=1; i<=byte; i++)
									((char *)(pm->pixels))[y*pm->row_bytes+x*byte+(byte-i)] = pixel_buffer[i-1];
							}

						BrFileClose(fh);
						BrMapAdd(pm);
#ifdef VERBOSE
						fprintf(stderr,"'%s' loaded as type %s (%d,%d)\n",
							pm->identifier,
							pixelmap_type_FM[pm->type].name,
							pm->width,pm->height);
#endif
						
					    	break;
					}
					/*
					 * -v   view current BR_PMT_INDEX_8 entries in register
					 */
					case 'v':{
						br_pixelmap **mtable;
						int nmaps,i;

						nmaps = CurrentPixelmaps(&mtable);
						
						for(i=0; i<nmaps; i++)
						    	ShowPiccy(mtable[i]);

						BrMemFree(mtable);
						
					    	break;
					}
					
					/*
					 * -x flip left/right
					 */
					case 'x':{
						br_pixelmap **mtable,*pm;
						int nmaps,i;

						nmaps = CurrentPixelmaps(&mtable);
						
						for(i=0; i<nmaps; i++) {
						    
						    	pm = FlipX(mtable[i]);

							BrMapRemove(mtable[i]);
							BrPixelmapFree(mtable[i]);

							BrMapAdd(pm);
						}
						
						BrMemFree(mtable);
						
					    	 break;
					}
					
					/*
					 * -y flip top/bottom
					 */
					case 'y':{
						br_pixelmap **mtable,*pm;
						int nmaps,i;

						nmaps = CurrentPixelmaps(&mtable);
						
						for(i=0; i<nmaps; i++) {
						    
						    	pm = FlipY(mtable[i]);

							BrMapRemove(mtable[i]);
							BrPixelmapFree(mtable[i]);

							BrMapAdd(pm);
						}
						
						BrMemFree(mtable);
						
					    	 break;
					}
					/*
					 * -o <output_file>
					 */
					case 'o':{
						int i,ot = OutputType;
						char *cp;

						if(ot == TYPE_GUESS && (cp = strrchr(*argv,'.'))) {
						/*
						 * Guess import type based on extension
						 */
						for(i=0; i < BR_ASIZE(OutputFileTypes); i++)
							if(!stricmp(OutputFileTypes[i].name,cp+1))
								ot = i;
					}
					/*
					 * Complain if, by this point, we don't know how to process file
					 */
					if(ot == TYPE_GUESS)
						BR_ERROR0("Unknown output file format");

					OutputPIX(*argv,OutputFileTypes[ot].type);
					
					break;
					}
				}
			}
		}
		else
		{
			int i,it = InputType;
			char *cp,*input_name = *argv;
			br_pixelmap *pm;

			/**
			 ** process input file
			 **/

			if(it == TYPE_GUESS && (cp = strrchr(input_name,'.')))
			{
			/*
			 * Guess import type based on extension
			 */
				for(i=0; i < BR_ASIZE(InputFileTypes); i++)
				if(!stricmp(InputFileTypes[i].name,cp+1))
					it = i;
			}

			/*
			 * Complain if, by this point, we don't know how to process file
			 */
			if(it == TYPE_GUESS)
				BR_ERROR0("Unknown input file format");

			pm = InputFileTypes[it].function(input_name,alpha_channel);

			#ifdef VERBOSE
				PrintLoadDetails(pm);
			#endif

			if(pm != NULL) BrMapAdd(pm);
		}
	}
	/*
	 * Close down and go home
	 */
	BrEnd();

	return 0;
}

/*
 * functions to read a pixel from a pixelmap type
 */

br_colour get_pmt_index_8(br_pixelmap *pm,int x,int y,int offset,int byte)
{
	char *src;
	br_colour *palette_entry;
	alpha_channel = 0;

	src = pm->pixels;
	palette_entry = pm->map->pixels;

	if (palette_entry == 0)
		alpha_channel = 255 << 24;

	return alpha_channel + palette_entry[*(src+offset+y*pm->row_bytes+x)];
}
br_colour get_pmt_rgb_555(br_pixelmap *pm,int x,int y,int offset,int byte)
{
    	char *src;
	short pixel;

	src = pm->pixels;
	pixel = *(short *)(src+y*pm->row_bytes+x*byte+offset);

	return (BR_COLOUR_RGB((pixel >> 7) & 0xf8,(pixel >> 2) & 0xf8,(pixel << 3) & 0xf8)) & 0xffffff;
}
br_colour get_pmt_rgb_565(br_pixelmap *pm,int x,int y,int offset,int byte)
{
    	char *src;
	short pixel;

	src = pm->pixels;
	pixel = *(short *)(src+y*pm->row_bytes+x*byte+offset);

	return (BR_COLOUR_RGB((pixel >> 8) & 0xf8,(pixel >> 3) & 0xf8,(pixel << 3) & 0xf8)) & 0xffffff;
}
br_colour get_pmt_rgb_888(br_pixelmap *pm,int x,int y,int offset,int byte)
{
    	char *src;

	src = pm->pixels;
	src += offset+y*pm->row_bytes+x*byte;

	return (BR_COLOUR_RGB(src[2],src[1],src[0])& 0xffffff);
}
br_colour get_pmt_rgbx_888(br_pixelmap *pm,int x,int y,int offset,int byte)
{
    	char *src;

	src = pm->pixels;
	src += offset+y*pm->row_bytes+x*byte;

	return	(src[3]<<24) + BR_COLOUR_RGB(src[2],src[1],src[0]);
}
br_colour get_pmt_rgba_8888(br_pixelmap *pm,int x,int y,int offset,int byte)
{
    	char *src;

	src = pm->pixels;
	src += offset+y*pm->row_bytes+x*byte;

	return  (src[3]<<24) + BR_COLOUR_RGB(src[2],src[1],src[0]);
}

void put_pmt_rgb_555(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte)
{
    	char *src;
	short pixel;

	src = pm->pixels;

	pixel = ((BR_RED(colour) >> 3) << 10) +
		((BR_GRN(colour) >> 3) << 5) +
		((BR_BLU(colour) >> 3));

	*(short *)(src+y*pm->row_bytes+x*byte+offset) = pixel;
}
void put_pmt_rgb_565(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte)
{
    	char *src;
	short pixel;

	src = pm->pixels;

	pixel = ((BR_RED(colour) >> 3) << 11) +
		((BR_GRN(colour) >> 2) << 5) +
		((BR_BLU(colour) >> 3));

	*(short *)(src+y*pm->row_bytes+x*byte+offset) = pixel;
}
void put_pmt_rgb_888(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte)
{
    	char *src;

	src = pm->pixels;
	src += offset + y*pm->row_bytes+(x*byte);

	src[2] = BR_RED(colour);
	src[1] = BR_GRN(colour);
	src[0] = BR_BLU(colour);
}
void put_pmt_rgbx_888(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte)
{
    	char *src;

	src = pm->pixels;
	src += offset+y*pm->row_bytes+x*byte;

	src[3] = colour >> 24;
	src[2] = BR_RED(colour);
	src[1] = BR_GRN(colour);
	src[0] = BR_BLU(colour);
}
void put_pmt_rgba_8888(br_pixelmap *pm,br_colour colour,int x,int y,int offset,int byte)
{
    	char *src;

	src = pm->pixels;
	src += offset+y*pm->row_bytes+x*byte;

	src[3] = colour >> 24;
	src[2] = BR_RED(colour);
	src[1] = BR_GRN(colour);
	src[0] = BR_BLU(colour);
}

