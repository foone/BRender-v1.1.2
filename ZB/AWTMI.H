/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: awtmi.h 1.1 1995/08/31 16:52:47 sam Exp $
 *
 * Triangle renderer for arbitrary width linear texture mapping
 */
void BR_ASM_CALL
FNAME(struct temp_vertex_fixed *a,struct temp_vertex_fixed *b,struct temp_vertex_fixed *c)
{
    /* v[0] and v[1] contain the (fractional) screen coordinates */
    /* And comp[C_W] contain the 3-space z-value */

    /* (v[0]-width/2)*comp[C_W] gives the 3-space x-values */
    /* Similarly for y */

    int sxa,sxb,sxc,sya,syb,syc;

    int dv_int;
    int offset;
    int grad;

    br_fixed_ls g_divisor,dp1,dp2;

    int dv_int_nocarry;
    int dv_int_carry;
    int v_int_current;
    int direction;

    int texture_height;
    int colour_width;

    texture_height = zb.material->colour_map->height;
    zb.awsl.texture_stride = zb.material->colour_map->row_bytes;
    zb.awsl.texture_width = zb.material->colour_map->width;

    /* Optimal sort of 3 elements? */

    if (a->v[1]>b->v[1]) {
		if (b->v[1]>c->v[1]) {
	    	swap(struct temp_vertex_fixed *,a,c);
		} else {
		    if (a->v[1]>c->v[1]) {
				swap(struct temp_vertex_fixed *,a,b);
				swap(struct temp_vertex_fixed *,b,c);
	    	} else {
				swap(struct temp_vertex_fixed *,a,b);
	    	}
		}
    } else {
		if (b->v[1]>c->v[1]) {
	    	if (a->v[1]>c->v[1]) {
				swap(struct temp_vertex_fixed *,b,c);
				swap(struct temp_vertex_fixed *,a,b);
	    	} else {
				swap(struct temp_vertex_fixed *,b,c);
		    }
		}
    }

    sxa = sar16(a->v[0]);
    sya = sar16(a->v[1]);
    sxb = sar16(b->v[0]);
    syb = sar16(b->v[1]);
    sxc = sar16(c->v[0]);
    syc = sar16(c->v[1]);

    zb.top.x = sxb-sxa;
    zb.top.y = syb-sya;
    zb.bot.x = sxc-sxb;
    zb.bot.y = syc-syb;
    zb.main.x = sxc-sxa;
    zb.main.y = syc-sya;

    zb.awsl.texture_stride = zb.material->colour_map->row_bytes;
    zb.awsl.texture_width = zb.material->colour_map->width;
    texture_height = zb.material->colour_map->height;
    zb.awsl.texture_size = zb.awsl.texture_stride*texture_height;
    zb.awsl.texture_start = zb.texture_buffer;

    colour_width = zb.row_width/BPP;

    zb.main.grad = zb.main.x*_reciprocal[zb.main.y];
    zb.main.d_f = zb.main.grad << 16;
    grad = sar16(zb.main.grad);
    zb.main.d_i = grad+colour_width;

    zb.top.grad = zb.top.y ? zb.top.x*_reciprocal[zb.top.y] : 0;
    zb.top.d_f = zb.top.grad << 16;
    zb.top.d_i = sar16(zb.top.grad)+colour_width;

    zb.main.f = zb.top.f = 0x80000000;

    offset = high16(a->v[X])+high16(a->v[Y])*colour_width;
    zb.awsl.start = zb.awsl.end = zb.colour_buffer+offset*BPP;
    zb.awsl.zstart = (char *)zb.depth_buffer+offset*2;
    zb.top.count = syb-sya;

    zb.bot.f = 0x80000000;

    zb.bot.grad = zb.bot.y ? zb.bot.x*_reciprocal[zb.bot.y] : 0;
    zb.bot.d_f = zb.bot.grad << 16;
    zb.bot.d_i = sar16(zb.bot.grad)+colour_width;
    zb.bot.count = syc-syb;

    g_divisor = zb.top.x*zb.main.y-zb.main.x*zb.top.y;
    if (!g_divisor) return;

    direction = zb.top.count && zb.top.grad>=zb.main.grad ||
	zb.bot.count && zb.bot.grad<zb.main.grad;

#define PARAM_SETUP(param,p) {\
    dp1 = b->p-a->p;\
    dp2 = c->p-a->p;\
    param.grad_x = SafeFixedMac2Div(dp1,zb.main.y,-dp2,zb.top.y,g_divisor);\
    param.grad_y = SafeFixedMac2Div(dp2,zb.top.x,-dp1,zb.main.x,g_divisor);\
    param.current = a->p+param.grad_x/2;\
    param.d_nocarry = BrFixedToInt(zb.main.grad)*param.grad_x+param.grad_y;\
    param.d_carry = param.d_nocarry+param.grad_x;\
}

#if LIGHT
    PARAM_SETUP(zb.pi,comp[C_I]);
#endif
#if ZB
    PARAM_SETUP(zb.pz,v[Z]);
#endif

    PARAM_SETUP(zb.pu,comp[C_U]);
    PARAM_SETUP(zb.pv,comp[C_V]);

    zb.awsl.du_nocarry = zb.pu.d_nocarry << 16;
    zb.awsl.dv_nocarry = zb.pv.d_nocarry << 16;
    zb.awsl.du_carry = zb.pu.d_carry << 16;
    zb.awsl.dv_carry = zb.pv.d_carry << 16;
    zb.awsl.u_current = zb.pu.current << 16;
    zb.awsl.v_current = zb.pv.current << 16;

	/*
	 * Per scan line increments
	 */
    if (powerof2(zb.awsl.texture_width)) {
		zb.awsl.du_int_nocarry = (sar16(zb.pu.d_nocarry) & zb.awsl.texture_width-1)
		    -zb.awsl.texture_width;
		zb.awsl.du_int_carry = (sar16(zb.pu.d_carry) & zb.awsl.texture_width-1)
		    -zb.awsl.texture_width;
		zb.awsl.u_int_current = sar16(zb.pu.current) & zb.awsl.texture_width-1;
    } else {
		zb.awsl.du_int_nocarry = sar16(zb.pu.d_nocarry) % zb.awsl.texture_width;
		zb.awsl.du_int_carry = sar16(zb.pu.d_carry) % zb.awsl.texture_width;
		zb.awsl.u_int_current = sar16(zb.pu.current) % zb.awsl.texture_width;
		if (zb.awsl.du_int_nocarry>=0) zb.awsl.du_int_nocarry -= zb.awsl.texture_width;
		if (zb.awsl.du_int_carry>=0) zb.awsl.du_int_carry -= zb.awsl.texture_width;
		if (zb.awsl.u_int_current<0) zb.awsl.u_int_current += zb.awsl.texture_width;
    }

    if (powerof2(texture_height)) {
		dv_int_nocarry = (sar16(zb.pv.d_nocarry) & texture_height-1)
		    -texture_height;
		dv_int_carry = (sar16(zb.pv.d_carry) & texture_height-1)
		    -texture_height;
		v_int_current = sar16(zb.pv.current) & texture_height-1;
    } else {
		dv_int_nocarry = sar16(zb.pv.d_nocarry) % texture_height;
		dv_int_carry = sar16(zb.pv.d_carry) % texture_height;
		v_int_current = sar16(zb.pv.current) % texture_height;
		if (dv_int_nocarry>=0) dv_int_nocarry -= texture_height;
		if (dv_int_carry>=0) dv_int_carry -= texture_height;
		if (v_int_current<0) v_int_current += texture_height;
    }

    zb.awsl.dsource_nocarry = zb.awsl.du_int_nocarry*BPP+zb.awsl.texture_stride*dv_int_nocarry;
    zb.awsl.dsource_carry = zb.awsl.du_int_carry*BPP+zb.awsl.texture_stride*dv_int_carry;
    zb.awsl.source_current = zb.texture_buffer+zb.awsl.u_int_current*BPP+zb.awsl.texture_stride*v_int_current;
    zb.awsl.u_int_current += 0x8000;

    if (direction) {
		zb.awsl.du = zb.pu.grad_x << 16;
		zb.awsl.dv = zb.pv.grad_x << 16;
		zb.awsl.du_int = sar16(zb.pu.grad_x) % zb.awsl.texture_width;
		if (powerof2(texture_height))
		    dv_int = (sar16(zb.pv.grad_x) & texture_height-1)-texture_height;
		 else {
		    dv_int = sar16(zb.pv.grad_x) % texture_height;
	    	if (dv_int>=0) dv_int -= texture_height;
		 }
    } else {
		zb.awsl.du = -zb.pu.grad_x << 16;
		zb.awsl.dv = -zb.pv.grad_x << 16;
		zb.awsl.du_int = sar16(-zb.pu.grad_x) % zb.awsl.texture_width;
		if (powerof2(texture_height)) {
		    dv_int = (sar16(-zb.pv.grad_x) & texture_height-1)-texture_height;
		 } else
		    dv_int = sar16(-zb.pv.grad_x) % texture_height;
	    if (dv_int>=0) dv_int -= texture_height;
    }

    if (zb.awsl.du_int>=0) zb.awsl.du_int -= zb.awsl.texture_width;

    zb.awsl.dsource = zb.awsl.du_int*BPP+zb.awsl.texture_stride*dv_int;
    zb.awsl.edge = &zb.top;

    ASSERT(zb.awsl.source_current>=zb.awsl.texture_start);

    TNAME();

    zb.awsl.end = zb.colour_buffer+high16(b->v[X])*BPP+high16(b->v[Y])*zb.row_width;
    zb.awsl.edge = &zb.bot;

    ASSERT(zb.awsl.source_current>=zb.awsl.texture_start);

    TNAME();
}
