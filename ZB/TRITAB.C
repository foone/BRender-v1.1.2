/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: tritab.c 1.4 1995/02/22 21:53:53 sam Exp $
 * $Locker:  $
 *
 * Prototype table driven small triangle scan converter
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "zb.h"
#include "fixed.h"
#include "scalar.h"
#include "shortcut.h"
#include "trigen.h"

static char rscid[] = "$Id: tritab.c 1.4 1995/02/22 21:53:53 sam Exp $";

void dprintf(int x, int y, char *fmt,...);
extern int ToggleFlag;
extern int DebugCount0;
extern int DebugCount1;

#define ST_MIN_DX	-4
#define ST_MIN_DY	0

#define ST_MAX_DX	3
#define ST_MAX_DY	7

/*
 * Edge indexes
 */
#define ET 0
#define EB 1
#define EM 2

struct small_triangle {
	br_fixed_ls		edge_gradients[3];
	br_int_32		divisor;

	br_fixed_ls		divisor_x0;
	br_fixed_ls		divisor_x1;

	br_fixed_ls		divisor_y0;
	br_fixed_ls		divisor_y1;
};

struct small_triangle SmallTriLUT
	[ST_MAX_DX-ST_MIN_DX+1]
	[ST_MAX_DX-ST_MIN_DX+1]
	[ST_MAX_DY-ST_MIN_DY+1]
	[ST_MAX_DY-ST_MIN_DY+1];

/*
 * Setup lookup table
 */
void SetupSmallTriLUT(void)
{
	int dx[3],dy[3];
	struct small_triangle *stp;

	/*
	 * For each possible triangle in the LUT...
	 */
	for(dx[ET] = ST_MAX_DX; dx[ET] < ST_MAX_DX; dx[ET]++)
	for(dx[EM] = ST_MAX_DX; dx[EM] < ST_MAX_DX; dx[EM]++)
	for(dy[ET] = ST_MAX_DY; dy[ET] < ST_MAX_DY; dy[ET]++)
	for(dy[EB] = ST_MAX_DY; dy[EB] < ST_MAX_DY; dy[EB]++) {

		/*
		 * Find deltas for third edge
		 */
		dx[EB] = dx[EM]-dx[ET];
		dy[EM] = dy[ET]+dy[EB];
	
		stp = &SmallTriLUT[dx[ET]][dx[EB]][dy[ET]][dy[EB]];

		/*
		 * Work out edge gradients
		 */
		stp->edge_gradients[ET] = BrFixedDiv(BrIntToFixed(dx[ET]),BrIntToFixed(dy[ET]));
		stp->edge_gradients[EB] = BrFixedDiv(BrIntToFixed(dx[EB]),BrIntToFixed(dy[EB]));
		stp->edge_gradients[EM] = BrFixedDiv(BrIntToFixed(dx[EM]),BrIntToFixed(dy[EM]));

		/*
		 * Work out divisor for parameters (area)
		 */
		stp->divisor = dx[ET] * dy[EM] - dx[EM] * dy[ET];

		stp->divisor_x0 = BrFixedDiv(BrIntToFixed(dy[EM]),BrIntToFixed(stp->divisor));
		stp->divisor_x1 = BrFixedDiv(BrIntToFixed(dy[ET]),BrIntToFixed(stp->divisor));

		stp->divisor_y1 = BrFixedDiv(BrIntToFixed(dx[EM]),BrIntToFixed(stp->divisor));
		stp->divisor_y0 = BrFixedDiv(BrIntToFixed(dx[ET]),BrIntToFixed(stp->divisor));

		/*
		 * Render triangle outline into a scratch buffer
		 */

		/*
		 * Mark the vertices of the triangle
		 */

		/*
		 * Classify triangle type
		 */
	}
}

/*
 * Override smooth shaded 16 bit Z buffered triangles
 */
void BR_ASM_CALL TestPixelRender_Z2I(br_int_32 x, br_int_32 y)
{
	SETUP_OFFSET;

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = CURRENT_INT(pi);
	}
}

void BR_ASM_CALL TestPixelRender_Z2IX(br_int_32 x, br_int_32 y)
{
	SETUP_OFFSET;

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = CURRENT_INT(pi)+32;
	}
}

#define SWAPV(a,b) {t = a; a = b; b = t;}

void BR_ASM_CALL XTriangleRenderPIZ2I(struct temp_vertex *v0, struct temp_vertex *v1,struct temp_vertex *v2)
{
	struct temp_vertex tv[3],*t;
	int x[3],y[3],dx[3],dy[3];
	int i;

	/*
	 * Sort vertices in Y
	 */
	if ((v0->v[Y]) > (v1->v[Y]))
		SWAPV(v0, v1);
	if ((v0->v[Y]) > (v2->v[Y]))
		SWAPV(v0, v2);
	if ((v1->v[Y]) > (v2->v[Y]))
		SWAPV(v1, v2);

	tv[0] = *v0;
	tv[1] = *v1;
	tv[2] = *v2;

	zb.component_mask = CM_Z | CM_I;
	zb.correct_mask = 0;

	zb.trapezoid_render = GenericTrapezoidRender;
	zb.pixel_render = TestPixelRender_Z2I;

	/*
	 * Truncate to integer coordinates
	 */
	for(i = 0; i < 3; i++) {
		x[i] = tv[i].v[X] >> 16;
		y[i] = tv[i].v[Y] >> 16;
		tv[i].v[X] &= 0xffff0000;
		tv[i].v[Y] &= 0xffff0000;
	}

	/*
	 * Work out edge deltas
	 */
	dx[ET] = x[1] - x[0];
	dx[EM] = x[2] - x[0];

	dy[ET] = y[1] - y[0];
	dy[EB] = y[2] - y[1];

#if 0
	dprintf(0,0,"Top dx = %d  dy = %d",dx[ET],dy[ET]);
	dprintf(0,1,"Bot dx = %d  dy = %d",dx[EB],dy[EB]);
#endif

	/*
	 * Do deltas fit into LUT?
	 */
	if(dx[ET] < ST_MIN_DX || dx[ET] > ST_MAX_DX ||
	   dx[EM] < ST_MIN_DX || dx[EM] > ST_MAX_DX ||
	   dy[ET] < ST_MIN_DY || dy[ET] > ST_MAX_DY ||
	   dy[EB] < ST_MIN_DY || dy[EB] > ST_MAX_DY) {
		/*
		 * Use generic scan converter
		 */
		GenericTriangleRender(tv+0,tv+1,tv+2);
		DebugCount0++;
	} else {
		zb.pixel_render = TestPixelRender_Z2IX;
		GenericTriangleRender(tv+0,tv+1,tv+2);
		DebugCount1++;
	}
}

