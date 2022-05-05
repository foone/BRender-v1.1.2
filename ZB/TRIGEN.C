/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: trigen.c 1.3 1995/02/22 21:53:51 sam Exp $
 * $Locker:  $
 *
 * Generic triangle scan converter
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <argstd.h>
#include <error.h>

#include "zb.h"
#include "fixed.h"
#include "scalar.h"
#include "shortcut.h"

static char rscid[] = "$Id: trigen.c 1.3 1995/02/22 21:53:51 sam Exp $";

#define SCAN_FIXED	0	/* Use fixed(1)/floating(0) point maths for parameter gradients */

#define BR_FRACT(x) ((br_fixed_luf) (x))
#define FIXED_AT_OFFSET(ptr, offset) ( *( ((br_fixed_ls *) ((char *) ptr + offset)) ) )

#if SCAN_FIXED
STATIC br_fixed_ls		g_divisor;
#else
STATIC float			g_divisor;
#endif

STATIC br_fixed_ls pdx, pdy;

/*
 * A pointer per component index to the parameter block to use and the offset of the scalar to use
 */ 
STATIC struct {
	struct scan_parameter *param;
	int	offset;
} parameter_info[] = {
	NULL,		0,											/* C_X */
	NULL,		0,											/* C_Y */
	&zb.pz,		offsetof(struct temp_vertex, v[Z]), 		/* C_Z */
	NULL,		0,											/* C_W */
	&zb.pu,		offsetof(struct temp_vertex, comp[C_U]),	/* C_U */
	&zb.pv,		offsetof(struct temp_vertex, comp[C_V]),	/* C_V */
	&zb.pi,		offsetof(struct temp_vertex, comp[C_I]),	/* C_I */
	&zb.pr,		offsetof(struct temp_vertex, comp[C_R]),	/* C_R */
	&zb.pg,		offsetof(struct temp_vertex, comp[C_G]),	/* C_G */
	&zb.pb,		offsetof(struct temp_vertex, comp[C_B]),	/* C_B */
	&zb.pq,		offsetof(struct temp_vertex, comp[C_Q]),	/* C_Q */
};


/*
 * Set up the per triangle constants for an interpolated parameter
 */

STATIC void ParameterSetup(struct scan_parameter *param,
				int comp,
				struct temp_vertex *v0,
				struct temp_vertex *v1,
				struct temp_vertex *v2)
{

#if SCAN_FIXED

	br_fixed_ls	dp1, dp2;

	/* Work out parameter deltas
	 */

	dp1 = FIXED_AT_OFFSET(v1, comp) - FIXED_AT_OFFSET(v0, comp);
	dp2 = FIXED_AT_OFFSET(v2, comp) - FIXED_AT_OFFSET(v0, comp);

	/* Work out X, Y gradients of parameter
	 */

#if 1
	param->grad_x = BrFixedMul(dp1, zb.main.y) - BrFixedMul(dp2, zb.top.y);
	param->grad_x = BrFixedDiv(param->grad_x, g_divisor);

	param->grad_y = BrFixedMul(dp2, zb.top.x) - BrFixedMul(dp1, zb.main.x);
	param->grad_y = BrFixedDiv(param->grad_y, g_divisor);
#else
	param->grad_x = BrFixedMac2Div(dp1, zb.main.y, -dp2, zb.top.y, g_divisor);

	param->grad_y = BrFixedMac2Div(dp2, zb.top.x, -dp1, zb.main.x, g_divisor);
#endif

#else

	float	dp1, dp2;

	/* Work out parameter deltas
	 */

	dp1 = BrFixedToFloat(FIXED_AT_OFFSET(v1, comp) - FIXED_AT_OFFSET(v0, comp));
	dp2 = BrFixedToFloat(FIXED_AT_OFFSET(v2, comp) - FIXED_AT_OFFSET(v0, comp));

	/* Work out X, Y gradients of parameter
	 */

	param->grad_x = BrFloatToFixed((dp1 * BrFixedToFloat(zb.main.y)
	                              - dp2 * BrFixedToFloat(zb.top.y))  * g_divisor);

	param->grad_y = BrFloatToFixed((dp2 * BrFixedToFloat(zb.top.x)
	                              - dp1 * BrFixedToFloat(zb.main.x)) * g_divisor);
#endif

	/* Initialise starting value of parameter
	 */

	param->current = FIXED_AT_OFFSET(v0, comp) + BrFixedMul(pdx, param->grad_x)
	                                           + BrFixedMul(pdy, param->grad_y);

	/* Initialise per scanline increments
	 */

	param->d_nocarry = BrFixedToInt(zb.main.grad) * param->grad_x + param->grad_y;
	param->d_carry	  = param->d_nocarry + param->grad_x;
}

/*
 * Generic triangle scan converter
 */
#define SWAP(a,b) {t = a; a = b; b = t;}

void BR_ASM_CALL GenericTriangleRender(struct temp_vertex *v0, struct temp_vertex *v1, struct temp_vertex *v2)
{
	struct temp_vertex tv[3];
	struct temp_vertex *t;
	br_int_32	i,j,m;
	char	flag;

	/*
	 * General setup -
	 */

	/* Sort vertices in Y order
	 * XXX Optimise?
	 */

	if ((v0->v[Y]) > (v1->v[Y]))
		SWAP(v0, v1);
	if ((v0->v[Y]) > (v2->v[Y]))
		SWAP(v0, v2);
	if ((v1->v[Y]) > (v2->v[Y]))
		SWAP(v1, v2);

	/*
	 * Work out starts of each edge
	 */
	zb.main.start = zb.top.start = BrFixedToInt(v0->v[Y]);
	zb.bot.start = BrFixedToInt(v1->v[Y]);

	/* Work out top and bottom trapezoid heights
	 */
	zb.main.count = BrFixedToInt(v2->v[Y]) - zb.main.start;
	zb.top.count = zb.bot.start - zb.top.start;
	zb.bot.count = BrFixedToInt(v2->v[Y]) - zb.bot.start;

	/*
	 * Ignore zero height triangles
	 */

	if ((zb.top.count == 0) && (zb.bot.count == 0)) return;

	/* Work out deltas and gradients along...
	 */

	/* Top short edge
	 */
	zb.top.x = v1->v[X] - v0->v[X];
	zb.top.y = v1->v[Y] - v0->v[Y];

	if (zb.top.count)
		zb.top.grad = BrFixedDiv(zb.top.x, zb.top.y);

	/* Bottom short edge
	 */
	zb.bot.x = v2->v[X] - v1->v[X];
	zb.bot.y = v2->v[Y] - v1->v[Y];

	if (zb.bot.count)
		zb.bot.grad = BrFixedDiv(zb.bot.x, zb.bot.y);

	/* Long edge
	 */
	zb.main.x = v2->v[X] - v0->v[X];
	zb.main.y = v2->v[Y] - v0->v[Y];
	zb.main.grad = BrFixedDiv(zb.main.x, zb.main.y);

	/* Work out divisor for parameter gradient calcs.
	 */

#if SCAN_FIXED
	g_divisor = BrFixedMul(zb.top.x, zb.main.y) - BrFixedMul(zb.main.x, zb.top.y);
	if (g_divisor == 0) return;
#else
	g_divisor = (BrFixedToFloat(zb.top.x)  * BrFixedToFloat(zb.main.y)
	           - BrFixedToFloat(zb.main.x) * BrFixedToFloat(zb.top.y));
	if (g_divisor == 0.0) return;
	g_divisor = 1.0 / g_divisor;
#endif

	/* Initialise bottom X
	 */

	pdy = S1 - BR_FRACT(v1->v[Y]);
	zb.bot.i = v1->v[X] + BrFixedMul(pdy, zb.bot.grad);

	/* Initialise main X and top X and remember offset so that params can
	 * be initialised as well
	 */

	pdy = S1 - BR_FRACT(v0->v[Y]);
	zb.main.i = v0->v[X] + BrFixedMul(pdy, zb.main.grad);
	zb.top.i  = v0->v[X] + BrFixedMul(pdy, zb.top.grad);
	pdx = (zb.main.i & 0xffff0000) + S1 - v0->v[X];

	/*
	 * Prepare any parameters for perspective correct interpolation
	 */
	if(zb.correct_mask) {
	 	/*
		 * Make temporary copies of the vertices
		 */
 		tv[0] = *v0; tv[1] = *v1; tv[2] = *v2;
		v0 = tv+0;	v1 = tv+1;	v2 = tv+2;

	 	for(i=0; i<3; i++) {

	 		tv[i].comp[C_Q] = BrFixedDiv(S1, tv[i].comp[C_W]);

			for(j = C_U, m = zb.correct_mask >> C_U ; m ;  j++, m = m >> 1 )
				if(m & 1)
			 		tv[i].comp[j] = BrFixedMul(tv[i].comp[j], tv[i].comp[C_Q]);
	 	}
	}

	/*
 	 * Set up parameters for scan convertions
	 */
	for(i = C_Z, m = zb.component_mask >> C_Z ; m ;  i++, m = m >> 1 )
		if(m & 1)
			ParameterSetup(parameter_info[i].param, parameter_info[i].offset, v0, v1, v2);

	/*
	 * Scan convert the two trapezoids
	 */

	zb.trapezoid_render(&zb.top);
	zb.trapezoid_render(&zb.bot);
}

/*
 * Scan the trapezoid betwen two edges, calling zb.pixel_plot for each pixel
 */

/* This macro needs extending if NUM_COMPONENTS in zb.h changes */
#define SET_PARAMS(a, op, b)\
{\
		zb.pz.a op zb.pz.b;\
		zb.pu.a op zb.pu.b;\
		zb.pv.a op zb.pv.b;\
		zb.pi.a op zb.pi.b;\
		zb.pr.a op zb.pr.b;\
		zb.pg.a op zb.pg.b;\
		zb.pb.a op zb.pb.b;\
		zb.pq.a op zb.pq.b;\
}

void BR_ASM_CALL GenericTrapezoidRender(struct scan_edge *minor)
{
	char flag;
	int i;
	int y = minor->start;

	while (minor->count-- > 0) {
		SET_PARAMS(currentpix, =, current)

		if (g_divisor >= 0) {
			for(i=BrFixedToInt(zb.main.i); i < BrFixedToInt(minor->i); i++) {
				zb.pixel_render(i, y);
				SET_PARAMS(currentpix, +=, grad_x)
			}
		} else {
			for(i=BrFixedToInt(zb.main.i)-1; i > BrFixedToInt(minor->i)-1; i--) {
				SET_PARAMS(currentpix, -=, grad_x)
				zb.pixel_render(i, y);
			}
		}

		zb.main.i = BrFixedAddCarry(zb.main.i, zb.main.grad, &flag);
		minor->i += minor->grad;

		y++;

		if (flag)
			SET_PARAMS(current, +=, d_carry)
		else
			SET_PARAMS(current, +=, d_nocarry)
	}
}


