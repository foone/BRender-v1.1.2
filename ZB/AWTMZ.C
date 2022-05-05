/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: awtmz.c 1.1 1995/08/31 16:52:48 sam Exp $
 * $Locker:  $
 *
 * Arbitary width texture mapper
 */
#include <math.h>

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

extern int BR_ASM_CALL SafeFixedMac2Div(int,int,int,int,int);

typedef void BR_ASM_CALL ScanLineCallFn(void);
typedef ScanLineCallFn *ScanLineCall;

#ifdef __WATCOMC__

extern int sar16(int);

#pragma aux sar16 = "sar eax,16" parm [eax]

#else

#define sar16 _sar16

#endif

typedef void BR_ASM_CALL TrapezoidRenderCall(void);

//TrapezoidRenderCall TrapezoidRenderPIZ2TA; 	// Hawkeye
TrapezoidRenderCall TrapezoidRenderPIZ2TIA;
TrapezoidRenderCall TrapezoidRenderPIZ2TIANT;

//TrapezoidRenderCall TrapezoidRenderPIZ2TA15;
//TrapezoidRenderCall TrapezoidRenderPIZ2TA24;

//TrapezoidRenderCall TrapezoidRenderPIZ2TAN;

#define ZB 1

#define BPP 1

#define FNAME TriangleRenderPIZ2TIA
#define TNAME TrapezoidRenderPIZ2TIA
#define BUMP 0
#define LIGHT 1
#include "awtmi.h"
#undef LIGHT
#undef BUMP
#undef FNAME
#undef TNAME

#define FNAME TriangleRenderPIZ2TIANT
#define TNAME TrapezoidRenderPIZ2TIANT
#define BUMP 0
#define LIGHT 1
#include "awtmi.h"
#undef LIGHT
#undef BUMP
#undef FNAME
#undef TNAME

#if 0											// Hawkeye

#define FNAME TriangleRenderPIZ2TA
#define TNAME TrapezoidRenderPIZ2TA
#define BUMP 0
#define LIGHT 0
#include "awtmi.h"
#undef LIGHT
#undef BUMP
#undef FNAME
#undef TNAME

#define FNAME TriangleRenderPIZ2TAN
#define TNAME TrapezoidRenderPIZ2TAN
#define BUMP 1
#define LIGHT 0
#include "awtmi.h"
#undef LIGHT
#undef FNAME
#undef TNAME
#undef BUMP

#undef BPP
#define BPP 3

#define FNAME TriangleRenderPIZ2TA24
#define TNAME TrapezoidRenderPIZ2TA24
#define BUMP 0
#define LIGHT 0
#include "awtmi.h"
#undef LIGHT
#undef BUMP
#undef FNAME
#undef TNAME

#undef BPP
#define BPP 2

#define FNAME TriangleRenderPIZ2TA15
#define TNAME TrapezoidRenderPIZ2TA15
#define BUMP 0
#define LIGHT 0
#include "awtmi.h"
#undef LIGHT
#undef BUMP
#undef FNAME
#undef TNAME

#undef BPP

#endif

