/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: realsupt.h 1.2 1995/09/07 15:19:43 sam Exp $
 * $Locker:  $
 */
#ifndef _REALSUPT_H_
#define _REALSUPT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dosio_real_memory {
	br_uint_32 pm_off;
	br_uint_16 pm_seg;
	br_uint_16 rm_off;
	br_uint_16 rm_seg;
} dosio_real_memory;

br_uint_32 BR_ASM_CALL _RealAllocate(dosio_real_memory *mem, br_uint_32 size);
void BR_ASM_CALL _RealFree(dosio_real_memory *mem);
void BR_ASM_CALL _RealVectorGet(br_uint_8 vector, br_uint_16 *vsegp, br_uint_16 *voffp);
void BR_ASM_CALL _RealVectorSet(br_uint_8 vector, br_uint_16 vseg, br_uint_16 voff);
void BR_ASM_CALL _RealVectorCall(br_uint_8 vector, union BR_REGS *ri, union BR_REGS *ro, struct BR_SREGS *sr);

void BR_ASM_CALL _RealLock(dosio_real_memory *mem, br_uint_32 size);
void BR_ASM_CALL _RealUnlock(dosio_real_memory *mem, br_uint_32 size);

void BR_ASM_CALL _RealSegmentsSetup(void);

extern br_uint_16 BR_ASM_DATA _RealSegment0000;
extern br_uint_16 BR_ASM_DATA _RealSegmentA000;

#ifdef __cplusplus
};
#endif
#endif

