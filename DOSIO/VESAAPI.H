/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesaapi.h 1.3 1995/06/30 15:42:27 sam Exp $
 * $Locker:  $
 */
#ifndef _VESAQRY_H_
#define _VESAQRY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * General VESA information
 */
struct  vesa_info {
    br_uint_8 signature[4];
    br_uint_16 version;
    br_uint_16 oemstring_off;
    br_uint_16 oemstring_seg;
    br_uint_8 capabilities[4];
    br_uint_16 videomode_off;
    br_uint_16 videomode_seg;
    br_uint_16 memory;
    br_uint_8 reserved[242];
};

#define VESA_MODES_MAX	128

/*
 * Per mode VESA information
 */
struct vesa_modeinfo {
    br_uint_16 attributes;
    br_uint_8 win_attr_a;
    br_uint_8 win_attr_b;
    br_uint_16 granularity;
    br_uint_16 win_size;
    br_uint_16 win_seg_a;
    br_uint_16 win_seg_b;
    br_uint_16 windowptr_off;
    br_uint_16 windowptr_seg;
    br_uint_16 bytesperline;
    br_uint_16 xres;
    br_uint_16 yres;
    br_uint_8 xcharsize;
    br_uint_8 ycharsize;
    br_uint_8 numberofplanes;
    br_uint_8 bitsperpixel;
    br_uint_8 banks;
    br_uint_8 memorymodel;
    br_uint_8 banksize;
    br_uint_8 imagepages;
    br_uint_8 reserved1;
    br_uint_8 redmask;
    br_uint_8 redfield;
    br_uint_8 greenmask;
    br_uint_8 greenfield;
    br_uint_8 bluemask;
    br_uint_8 bluefield;
    br_uint_8 resmask;
    br_uint_8 resfield;
    br_uint_8 directcolor;
    br_uint_8 reserved2[216];
};


int BR_ASM_CALL _VESABegin(void);
void BR_ASM_CALL _VESAEnd(void);

int BR_ASM_CALL _VESAInfo(struct vesa_info *vip, br_uint_16 *modes);
int BR_ASM_CALL _VESAModeInfo(struct vesa_modeinfo *vmip, br_uint_32 mode);

int BR_ASM_CALL _VESAModeSet(br_uint_32 mode);
int BR_ASM_CALL _VESAModeGet(br_uint_16 *mode);

int BR_ASM_CALL _VESAScanlineLengthSet(br_uint_32 width, br_uint_16 * bytes, br_uint_16 * pixels, br_uint_16 * scanlines);
int BR_ASM_CALL _VESAScanlineLengthGet(br_uint_16 * bytes, br_uint_16 * pixels, br_uint_16 * scanlines);

int BR_ASM_CALL _VESADisplayStartSet(br_uint_32 x, br_uint_32 y);
int BR_ASM_CALL _VESADisplayStartGet(br_uint_32 *px, br_uint_32 *py);

int BR_ASM_CALL _VESAWindowSet(br_uint_32 window, br_uint_32 position);
int BR_ASM_CALL _VESAWindowGet(br_uint_32 window, br_uint_32 *position);

#ifdef __cplusplus
};
#endif
#endif



