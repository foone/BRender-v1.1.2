#ifndef _PCGFXCOM_H_
#define _PCGFXCOM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * details relating screen modes to pixelmap types
 */
typedef struct {
    	int type;
	int bitsperpixel;	/* Displayed bits per pixel */
	int bytesperpixel;	/* Bytes per pixel in frame buffer */
} pixelmap_info;

/*
 * information about current mode
 */
typedef struct {
   	char *display_type;
   	int width;
	int height;
	int bitsperpixel;
	int bytesperpixel;
	int doublebuffertype;
	int mode;
	int set_scanline_length;
} setup_details;

/*
 * svga adapt. info
 */

typedef struct {
	br_uint_32 address;
	br_uint_32 page_num;
	br_uint_32 split;
	br_uint_32 pad;
} screen_table;

/*
 * Default setup string
 */
#define DOSGFX_DEFAULT "MCGA,"

#define DOSGFX_DEFAULT_VESA "VESA,W:640:H:480,B:8,C:0,M:0,L:0"
#define DOSGFX_DEFAULT_MCGA "MCGA,W:320,H:200,B:8,C:0,M:0,L:0"

/*
 * Environemnt variable used to pick up used defined modes
 */
#define DOSGFX_ENVNAME "BRENDER_DOS_GFX"

/*
 * Port addreses of VGA CLUT
 */
#define VGA_PAL_ADDRESS	0x3c8
#define VGA_PAL_DATA	0x3c9


extern char *_DOSGfxDefaultSetupString;
extern char *_DOSGfxEnvironmentVar;
extern int _DOSGfxIndexed;

extern pixelmap_info pixelmap_bit_sizes[];	/* screen mode <-> pixelmap details */
extern int pixelmap_bit_sizes_count;		/* # entries in above table */

extern int old_screen_mode;			/* previous screen mode */
extern int BR_ASM_DATA pc_gfx_set;			/* Flag set true is mode is set */

void _ProcessSetupString(char *string,setup_details *details); /* decode out brender_pc_gfx environment var */
void _PaletteSet(br_pixelmap *palette);
void _PaletteSetEntry(int i,br_colour colour);

#ifdef __cplusplus
};
#endif
#endif

