/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: readmse.c 1.5 1995/09/07 15:19:41 sam Exp $
 * $Locker: sam $
 *
 * Read mouse position from BIOS
 */
#include <dos.h>

#include "fw.h"
#include "syshost.h"
#include "realsupt.h"
#include "farmem.h"
#include "eventq.h"


static char rscid[] = "$Id: readmse.c 1.5 1995/09/07 15:19:41 sam Exp $";

/*
 * Real mode interrupt handler for mouse
 */
static unsigned char mouse_handler[] = {
#include "mouint.hex"
};

static dosio_real_memory mouse_memory;

extern dosio_real_memory _DOSEventMemory;
extern int _DOSEventQEnabled;

static REGS_TYPE ri,ro;
static SREGS_TYPE sr;

void BR_PUBLIC_ENTRY DOSMouseBegin(void)
{

	/* Allocate a block of DOS memory for handler
	 */
	if(_RealAllocate(&mouse_memory, sizeof(mouse_handler)))
		BR_ERROR0("Could not allocate real mode buffer  (M)\n");

// 	_RealLock(&mouse_memory, sizeof(mouse_handler));

	/*
	 * Copy real mode handler into block
	 */
	FarBlockWrite(mouse_memory.pm_off, mouse_memory.pm_seg, mouse_handler, sizeof(mouse_handler));

	/*
	 * If the event queue is initialised, then hook that into handler
	 */
	if(_DOSEventQEnabled) {
		FarWordWrite(SYM_MouseEventQOff+mouse_memory.pm_off, mouse_memory.pm_seg, _DOSEventMemory.rm_off);
		FarWordWrite(SYM_MouseEventQSeg+mouse_memory.pm_off, mouse_memory.pm_seg, _DOSEventMemory.rm_seg);
	}

	/*
	 * Reset mouse driver
	 */
	ri.WREGS.ax = 0;
	_RealVectorCall(0x33,&ri,&ro,&sr);

	/*
	 * Set real-mode handler
	 */
	ri.WREGS.ax = 0x0C;
	ri.WREGS.cx = 0x7F;
	ri.WREGS.dx = mouse_memory.rm_off;
	sr.es = mouse_memory.rm_seg;

	_RealVectorCall(0x33,&ri,&ro,&sr);

#ifdef __X32z__
	_disable();

	_enable();
#endif
}

void BR_PUBLIC_ENTRY DOSMouseEnd(void)
{
	/*
	 * Reset mouse driver
	 */
	ri.WREGS.ax = 0;
	_RealVectorCall(0x33,&ri,&ro,&sr);
}

void BR_PUBLIC_ENTRY DOSMouseRead(br_int_32 *mouse_x,br_int_32 *mouse_y,br_uint_32 *mouse_buttons)
{
	int mx,my;
	static int ox,oy;
#if 1
	mx = (short)FarWordRead(SYM_MouseX+mouse_memory.pm_off, mouse_memory.pm_seg);
	my = (short)FarWordRead(SYM_MouseY+mouse_memory.pm_off, mouse_memory.pm_seg);
	*mouse_buttons = FarWordRead(SYM_MouseButtons+mouse_memory.pm_off, mouse_memory.pm_seg);
#else
	mx = 0;
	my = 0;
	*mouse_buttons = 0;
#endif

	*mouse_x += mx - ox;
	*mouse_y += my - oy;

	ox = mx;
	oy = my;
}


