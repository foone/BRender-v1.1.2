/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: trigen.h 1.2 1995/02/22 21:53:52 sam Exp $
 * $Locker:  $
 *
 * Some macros for making writing generic triangle scan converters
 * erasier
 */
#ifndef _TRIGEN_H_
#define _TRIGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Find the current value of a parameter parameter
 */
#define CURRENT_INT(param) (((short *)&zb.param.currentpix)[1])
#define CURRENT_UINT(param) (((unsigned short *)&zb.param.currentpix)[1])

#define CURRENT(param) (zb.param.currentpix)

#define CURRENT_INT_CORRECT(param) (BrFixedDiv(zb.param.currentpix, zb.pq.currentpix) >> 16)
#define CURRENT_UINT_CORRECT(param) ((unsigned short)(BrFixedDiv(zb.param.currentpix, zb.pq.currentpix) >> 16))

#define CURRENT_CORRECT(param) (BrFixedDiv(zb.param.currentpix, zb.pq.currentpix)

/*
 * Setup a local variable for accessing pixels
 */
#define SETUP_OFFSET int offset = (y * zb.row_width + x)

/*
 * lvalues for reading and wring pixels of different forms
 */
#define DEPTH_2			(((unsigned short *)zb.depth_buffer)[offset])
#define DEPTH_4			(((unsigned long *)zb.depth_buffer)[offset])
#define COLOUR_INDEX	(((unsigned char *)zb.colour_buffer)[offset])

#define COLOUR_TRUE16	(((unsigned short *)zb.colour_buffer)[offset])

#define COLOUR_TRUE24_R	(((unsigned char *)zb.colour_buffer)[offset*3+0])
#define COLOUR_TRUE24_G	(((unsigned char *)zb.colour_buffer)[offset*3+1])
#define COLOUR_TRUE24_B	(((unsigned char *)zb.colour_buffer)[offset*3+2])

#define COLOUR_TRUE32_R	(((unsigned char *)zb.colour_buffer)[offset*4+0])
#define COLOUR_TRUE32_G	(((unsigned char *)zb.colour_buffer)[offset*4+1])
#define COLOUR_TRUE32_B	(((unsigned char *)zb.colour_buffer)[offset*4+2])

#define SHADE_LOOKUP(i, pix) (zb.shade_table[((i) << 8) + (pix)])
#define TEXTURE_LOOKUP(u, v) (zb.texture_buffer[(((v) << 8)  & 0xff00) + ((u) & 0x00ff)])

#ifdef __cplusplus
};
#endif

#endif
