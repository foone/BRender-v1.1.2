/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: awtm.c 1.7 1995/08/31 16:47:30 sam Exp $
 * $Locker:  $
 *
 * Arbitary width texture mapper
 */
#include <math.h>

#define WRAP 1
#define FIX 1

#include "brender.h"
#include "zb.h"
#include "shortcut.h"
#include "brassert.h"

#define swap(type,a,b) { type _; _ = a; a = b; b = _; }

#define high8(a) (*((unsigned char *)&a+3))
#define high16(a) (*((unsigned short *)&a+1))
#define cvsl(a) ((unsigned int)(unsigned short)(a))
#define mod2n(x,y) ((x)>=0 ? (x) & (y-1) : (x) | ~(y-1))
#define powerof2(x) (!((x) & (x)-1))

int Orig = 0;

extern int BR_ASM_CALL SafeFixedMac2Div(int,int,int,int,int);
BR_ASM_CALL_EXTRA(SafeFixedMac2Div)

typedef void BR_ASM_CALL ScanLineCallFn(void);
typedef ScanLineCallFn *ScanLineCall;

ArbitraryWidthScanLine BR_ASM_DATA awsl;
BR_ASM_DATA_EXTRA(awsl)

#ifdef __WATCOMC__

extern int sar16(int);

#pragma aux sar16 = "sar eax,16" parm [eax]

#else

#define sar16 _sar16

#endif

typedef void BR_ASM_CALL TrapezoidRenderCall(void);

TrapezoidRenderCall TrapezoidRenderPIZ2TIA;
TrapezoidRenderCall TrapezoidRenderPIZ2TA;
TrapezoidRenderCall TrapezoidRenderPIZ2TA15;
TrapezoidRenderCall TrapezoidRenderPIZ2TA24;

BR_ASM_CALL_EXTRA(TrapezoidRenderPIZ2TIA)
BR_ASM_CALL_EXTRA(TrapezoidRenderPIZ2TA)
BR_ASM_CALL_EXTRA(TrapezoidRenderPIZ2TA15)
BR_ASM_CALL_EXTRA(TrapezoidRenderPIZ2TA24)

#define BPP 1

#define FNAME TriangleRenderPIZ2TIA
#define LIGHT 1
#include "innera.h"
#undef LIGHT
#undef FNAME

#define FNAME TriangleRenderPIZ2TA
#define LIGHT 0
#include "innera.h"
#undef LIGHT
#undef FNAME

#undef BPP
#define BPP 3

#define FNAME TriangleRenderPIZ2TA24
#define LIGHT 0
#include "innera.h"
#undef LIGHT
#undef FNAME

#undef BPP
#define BPP 2

#define FNAME TriangleRenderPIZ2TA15
#define LIGHT 0
#include "innera.h"
#undef LIGHT
#undef FNAME

#undef BPP
