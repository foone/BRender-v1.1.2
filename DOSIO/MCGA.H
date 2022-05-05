/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: mcga.h 1.3 1995/02/22 22:09:22 sam Exp $
 * $Locker:  $
 *
 *
 */
#ifndef _MCGA_H_
#define _MCGA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * setup mcga video display mode
 */
br_pixelmap * BR_PUBLIC_ENTRY DOSGfxBegin_MCGA(char *new_setup_string);

/*
 * reset mcga video mode
 */
void BR_PUBLIC_ENTRY DOSGfxEnd_MCGA(void);

#ifdef __cplusplus
};
#endif
#endif

