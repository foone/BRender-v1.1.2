/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: dosproto.h 1.4 1995/06/30 15:42:07 sam Exp $
 * $Locker:  $
 *
 * Function prototypes for Brender DOS IO library
 */
#ifndef _DOSPROTO_H_
#define _DOSPROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Graphics mode
 */
br_pixelmap * BR_PUBLIC_ENTRY DOSGfxBegin(char *new_setup_string);
void BR_PUBLIC_ENTRY DOSGfxEnd(void);

void BR_PUBLIC_ENTRY DOSGfxPaletteSet(br_pixelmap *pm);
void BR_PUBLIC_ENTRY DOSGfxPaletteSetEntry(int i,br_colour colour);

/*
 * Mouse reading
 */
void BR_PUBLIC_ENTRY DOSMouseBegin(void);
void BR_PUBLIC_ENTRY DOSMouseEnd(void);
void BR_PUBLIC_ENTRY DOSMouseRead(br_int_32 *mouse_x,br_int_32 *mouse_y,br_uint_32 *mouse_buttons);

/*
 * Reading system clock
 */

void BR_ASM_CALL DOSClockBegin(void);
void BR_ASM_CALL DOSClockEnd(void);
br_uint_32 BR_ASM_CALL DOSClockRead(void);

/*
 * Keyboard UP/DOWN handling
 */
void BR_PUBLIC_ENTRY DOSKeyBegin(void);
void BR_PUBLIC_ENTRY DOSKeyEnd(void);
br_uint_8 BR_PUBLIC_ENTRY DOSKeyTest(br_uint_8 scancode,br_uint_8 qualifiers, br_uint_8 repeats);
void BR_PUBLIC_ENTRY DOSKeyEnableBIOS(br_uint_16 flag);

/*
 * Divide overflow suppressor
 */
int BR_ASM_CALL DOSDivTrapBegin(void);
int BR_ASM_CALL DOSDivTrapEnd(void);
int BR_ASM_CALL DOSDivTrapCount(int reset);

/*
 * Event queue for mouse and keyboard
 */
void BR_PUBLIC_ENTRY DOSEventBegin(void);
void BR_PUBLIC_ENTRY DOSEventEnd(void);
br_uint_16 BR_PUBLIC_ENTRY DOSEventWait(struct dosio_event *event, int block);

#endif /* _NO_PROTOTYPES */
#ifdef __cplusplus
};
#endif
#endif

