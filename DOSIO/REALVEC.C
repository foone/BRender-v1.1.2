/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: readmse.c 1.5 1995/09/07 15:19:41 sam Exp $
 * $Locker: sam $
 *
 * Get and set real mode vectors
 */
#include <dos.h>

#include "fw.h"
#include "syshost.h"
#include "realsupt.h"

#if 0
static char rscid[] = "$Id: $";

static REGS_TYPE regs;
static SREGS_TYPE sregs;

void BR_ASM_CALL _RealVectorGet(br_uint_8 vector, br_uint_16 *vsegp, br_uint_16 *voffp)
{
	regs.HREGS.ah = 0x35;
	regs.HREGS.al = vector;
	_RealVectorCall(0x21,&regs,&regs,&sregs);

	*vsegp = sregs.es;
	*voffp = regs.WREGS.bx;
}

void BR_ASM_CALL _RealVectorSet(br_uint_8 vector, br_uint_16 vseg, br_uint_16 voff)
{
	regs.HREGS.ah = 0x25;
	regs.HREGS.al = vector;
	sregs.ds = vseg;
	regs.WREGS.dx = voff;
	_RealVectorCall(0x21,&regs,&regs,&sregs);

#ifdef __X32__
	/*
	 * X32 with OS/2 needs this !?
	 */

	_disable();	
	_enable();
#endif
}
#endif
