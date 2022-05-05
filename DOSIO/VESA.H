/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vesa.h 1.3 1995/02/22 22:09:24 sam Exp $
 * $Locker:  $
 *
 *
 */
#ifndef _VESA_H_
#define _VESA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * setup VESA video display mode
 */
br_pixelmap * BR_PUBLIC_ENTRY DOSGfxBegin_VESA(char *new_setup_string);

/*
 * reset VESA video mode
 */
void BR_PUBLIC_ENTRY DOSGfxEnd_VESA(void);

#ifdef __cplusplus
};
#endif
#endif

