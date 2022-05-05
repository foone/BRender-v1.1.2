/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: keyboard.c 1.6 1995/09/07 15:19:40 sam Exp $
 * $Locker: sam $
 *
 * keyboard interrupt interface
 */
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>

#include "fw.h"
#include "syshost.h"
#include "keyboard.h"
#include "realsupt.h"
#include "farmem.h"
#include "eventq.h"

static char rscid[] = "$Id: keyboard.c 1.6 1995/09/07 15:19:40 sam Exp $";

/*
 * Real mode interrupt handler for INT15
 */
static unsigned char keyboard_handler[] = {
#include "kbdint.hex"
};

static dosio_real_memory keyboard_memory;

static unsigned short keyboard_old_seg;
static unsigned short keyboard_old_off;

extern dosio_real_memory _DOSEventMemory;
extern int _DOSEventQEnabled;

/*
 * Install a real mode interrupt handler on INT 15
 */
void BR_PUBLIC_ENTRY DOSKeyBegin(void)
{
	int i;

	/* Allocate a block of DOS memory for handler
	 */
	if(_RealAllocate(&keyboard_memory, sizeof(keyboard_handler)))
		BR_ERROR0("Could not allocate real mode buffer (K)\n");

// 	_RealLock(&keyboard_memory, sizeof(keyboard_handler));

	/*
	 * Copy real mode handler into block
	 */
	FarBlockWrite(keyboard_memory.pm_off, keyboard_memory.pm_seg, keyboard_handler, sizeof(keyboard_handler));

	/*
	 * Get old real mode vector
	 */
	_RealVectorGet(0x15,&keyboard_old_seg,&keyboard_old_off);

	/*
	 * Put chain address in realmode code
	 */
	FarWordWrite(SYM_KeyboardOldOff+keyboard_memory.pm_off, keyboard_memory.pm_seg, keyboard_old_off);
	FarWordWrite(SYM_KeyboardOldSeg+keyboard_memory.pm_off, keyboard_memory.pm_seg, keyboard_old_seg);

	/*
	 * If the event queue is initialised, then hook that
	 * into handler
	 */
	if(_DOSEventQEnabled) {
		FarWordWrite(SYM_KeyboardEventQOff+keyboard_memory.pm_off, keyboard_memory.pm_seg, _DOSEventMemory.rm_off);
		FarWordWrite(SYM_KeyboardEventQSeg+keyboard_memory.pm_off, keyboard_memory.pm_seg, _DOSEventMemory.rm_seg);
	}

	/* Set new real mode vector
	 */
	_RealVectorSet(0x15,keyboard_memory.rm_seg,keyboard_memory.rm_off+SYM_KeyboardHandler);

#ifdef __X32z__
	_disable();
	_enable();
#endif
}

/*
 * Clear down the custom keyboard handler
 */
void BR_PUBLIC_ENTRY DOSKeyEnd(void)
{
	/* Set new real mode vector
	 */
	_RealVectorSet(0x15,keyboard_old_seg,keyboard_old_off);

	/*
	 * Release memeory block
	 */
	_RealFree(&keyboard_memory);
}

/*
 * Enable or diable BIOS keystrokes, if enabled, then the
 * keyboard handler passes all scancode on to the BIOS
 */
void BR_PUBLIC_ENTRY DOSKeyEnableBIOS(br_uint_16 flag)
{
	FarWordWrite(SYM_KeyboardEnableBIOS+keyboard_memory.pm_off, keyboard_memory.pm_seg, flag != 0);
}

/*
 *  Test if a key on keyboard is depressed
 */
unsigned char BR_PUBLIC_ENTRY DOSKeyTest(unsigned char scancode,unsigned char qualifiers, unsigned repeats)
{
	unsigned char r;
	unsigned char m;

	m = FarByteRead(scancode + SYM_KeyboardMap + keyboard_memory.pm_off,keyboard_memory.pm_seg);

	if(qualifiers == 0)
		r = (m & (QUAL_NONE | repeats)) == (QUAL_NONE | repeats);
	else
		r = (m & (QUAL_ALL | repeats)) == (QUAL_NONE | qualifiers | repeats);

	m &= ~repeats;

	FarByteWrite(scancode + SYM_KeyboardMap + keyboard_memory.pm_off,keyboard_memory.pm_seg,m);

	return r;
}

char BR_PUBLIC_ENTRY DOSKeyMap(unsigned char scancode)
{
	return FarByteRead(scancode + SYM_KeyboardMap + keyboard_memory.pm_off,keyboard_memory.pm_seg);
}

