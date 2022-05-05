/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: realseg.h 1.3 1995/06/30 15:42:20 sam Exp $
 * $Locker:  $
 */
#ifndef _REALSEG_H_
#define _REALSEG_H_

#ifdef __cplusplus
extern "C" {
#endif

void BR_ASM_CALL _DOSRealSegmentsSetup(void);

extern br_uint_16 BR_ASM_DATA _DOSSegment0000;
extern br_uint_16 BR_ASM_DATA _DOSSegmentA000;

#ifdef __cplusplus
};
#endif
#endif

