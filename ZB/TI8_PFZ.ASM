;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: ti8_pfz.asm 1.6 1995/05/25 13:25:15 sam Exp $
;; $Locker:  $
;;
;; 8 bit Indexed mode
;;
;; Triangle scan convertion and rendering
;;
;; Perfect scan, fractional vertices, Z buffered
;;
	.386p
	.model flat,c

	include	zb.inc
	include 586_macs.inc

GRAD_IDIV	equ	0
GRAD_IMUL	equ	0
GRAD_FMUL	equ	1

X	equ 0
Y	equ 1
Z	equ 2

;Z_MASK		equ	0ffff0000h

FRAMEBUFFER_FAR	equ	0

;; Various macros for generating the triangle renderers
;;

; Setup of a fractional, perfect pixel triangle  
;
TRIANGLE_SETUP_PF	macro

	; Get Y components of vertices
	;
		mov	eax,[esi].temp_vertex.v[4]
		mov	ebx,[edi].temp_vertex.v[4]
		mov	ecx,[ebp].temp_vertex.v[4]

	; Sort pointers and values in order of y value
	;
		cmp	eax,ebx
		jg	short sort_3
		cmp	ebx,ecx
		jle	short sort_done
	; abc
		cmp	eax,ecx
		jg	short sort_2
	; acb
		SWAP	ebx,ecx
		SWAP	edi,ebp
		jmp	short sort_done
sort_2:
	; cab
		SWAP	ebx,ecx
		SWAP	edi,ebp
		SWAP	eax,ebx
		SWAP	esi,edi
		jmp	short sort_done
sort_3:
		cmp	ecx,ebx
		jg	short sort_4
	; cba
		SWAP	eax,ecx
		SWAP	esi,ebp
		jmp	short sort_done

sort_4:		cmp	eax,ecx
		jg	short sort_5
	; bac
		SWAP	eax,ebx
		SWAP	esi,edi
		jmp	short sort_done
sort_5:
	; bca
		SWAP	eax,ebx
		SWAP	esi,edi
		SWAP	ebx,ecx
		SWAP	edi,ebp
sort_done:

	; Work out top.count and bot.count - the heights of each part
	; of the triangle
	;
		sar	eax,16
		sar	ebx,16
		sar	ecx,16
		sub	ecx,ebx
		sub	ebx,eax
		mov	zb.bot.count,ecx
		mov	zb.top.count,ebx

		or	ebx,ecx	; Check for zero height polys
		je	quit

	; Work out deltas and gradients along edges
	;
		; top short edge
		;
		mov	edx,[edi].temp_vertex.v[0]	; zb.top.x
		sub	edx,[esi].temp_vertex.v[0]
		mov	zb.top.x,edx

		mov	ebx,[edi].temp_vertex.v[4]	; zb.top.y
		sub	ebx,[esi].temp_vertex.v[4]
		mov	zb.top.y,ebx

		cmp	zb.top.count,0
		je	short no_top
		
		FIX_DIV				; zb.top.grad = zb.top.x/zb.top.y
		idiv	ebx
		mov	zb.top.grad,eax
		mov	word ptr zb.top.d_f+2,ax
		sar	eax,16
		add	eax,zb.row_width
		mov	zb.top.d_i,eax
no_top:
		; bottom short edge
		;
		xor	eax,eax
		mov	edx,[ebp].temp_vertex.v[0]	; zb.bot.x
		sub	edx,[edi].temp_vertex.v[0]
		mov	zb.bot.x,edx

		mov	ebx,[ebp].temp_vertex.v[4]	; zb.bot.y
		sub	ebx,[edi].temp_vertex.v[4]
		mov	zb.bot.y,ebx

		cmp	zb.bot.count,0
		je	short no_bottom

		FIX_DIV				; zb.bot.grad = zb.bot.x/zb.bot.y
		idiv	ebx
		mov	zb.bot.grad,eax
		mov	word ptr zb.bot.d_f+2,ax
		sar	eax,16
		add	eax,zb.row_width
		mov	zb.bot.d_i,eax

no_bottom:
		; long edge
		;
		mov	edx,[ebp].temp_vertex.v[0]	; zb.main.x
		sub	edx,[esi].temp_vertex.v[0]
		mov	zb.main.x,edx

		mov	ebx,[ebp].temp_vertex.v[4]	; zb.main.y
		sub	ebx,[esi].temp_vertex.v[4]
		mov	zb.main.y,ebx

		FIX_DIV				; zb.main.grad = zb.main.x/zb.main.y
		idiv	ebx
		mov	zb.main.grad,eax
		mov	word ptr zb.main.d_f+2,ax
		sar	eax,16
		add	eax,zb.row_width
		mov	zb.main.d_i,eax

	; Work out divisor for use in parameter gradient calcs.
	;
if GRAD_IDIV
		mov	eax,zb.main.x
		imul	zb.top.y
		mov	ebx,eax
		mov	ecx,edx

		mov	eax,zb.top.x
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx
		FIX_MUL
		mov	g_divisor,eax		; bottom of gradient calc.
endif
if GRAD_IMUL
		mov	eax,zb.main.x
		imul	zb.top.y
		mov	ebx,eax
		mov	ecx,edx

		mov	eax,zb.top.x
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx
		FIX_MUL
		test	eax,eax

		jz	gd_is_zero
		mov	ebx,eax
		xor	edx,edx
		mov	eax,40000000h
		idiv	ebx
		add	eax,eax
		add	eax,eax
gd_is_zero:
		mov	g_divisor,eax		; bottom of gradient calc.
endif
if  GRAD_FMUL
		mov	eax,zb.main.x
		imul	zb.top.y
		mov	ebx,eax
		mov	ecx,edx

		mov	eax,zb.top.x
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx

		mov	g_d_temp,eax
		mov	g_d_temph,edx

		fild	qword ptr g_d_temp
		fld	fixed_one
		fdivrp	st(1),st
		fstp	g_divisor
endif
	; Initialise bottom x to intersection with middle scanline
	;
		xor	ecx,ecx
		sub	cx,word ptr [edi].temp_vertex.v[4]	; pdy = 1.0-fraction(p1y)

		mov	eax,zb.bot.grad		; x_b = p1x+pdy*zb.bot.grad
		imul	ecx
		FIX_MUL
		add	eax,[edi].temp_vertex.v[0]
		mov	word ptr zb.bot.f+2,ax
		sar	eax,16
		mov	zb.bot.i,eax

	; Initialise top x to intersection with first scanline
	;
	; From here on, only vertex 0 is needed
	;
	; Find x & y offset from vertex 0 to first pixel for use in param. initialisation
	;
		xor	ecx,ecx
		sub	cx,word ptr [esi].temp_vertex.v[4]	; pdy = 1.0-fraction(p0y)
		mov	p0_offset_y,ecx

		mov	eax,zb.main.grad			; x = p0x+pdy*zb.main.grad
		imul	ecx
		FIX_MUL
		add	eax,[esi].temp_vertex.v[0]
		mov	zb.main.f+2,eax

		xor	ax,ax
		mov	word ptr zb.main.i+2,ax ; XXX

		add	eax,010000h
		sub	eax,[esi].temp_vertex.v[0]	; pdx = integer(x)+1.0-p0x
		mov	p0_offset_x,eax

		mov	eax,zb.top.grad		; x_t = p0x+pdy*zb.top.grad
		imul	ecx
		FIX_MUL
		add	eax,[esi].temp_vertex.v[0]
		mov	word ptr zb.top.f+2,ax
		sar	eax,16
		mov	zb.top.i,eax

	; Convert integer parts of X variables to pixel offsets
	;
		mov	ebx,zb.row_width
		mov	ecx,dword ptr zb.colour_buffer
		xor	eax,eax
		mov	ax,word ptr [esi].temp_vertex.v[4]+2
		imul	ebx
		add	zb.main.i,eax
		add	eax,ecx
		add	zb.top.i,eax

		xor	eax,eax
		mov	ax,word ptr [edi].temp_vertex.v[4]+2
		imul	ebx
		add	eax,ecx
		add	zb.bot.i,eax
		endm

; Per-parameter initialisation of perfect scan, fractionl vertex triangle
;
TRIANGLE_PARAM_PF	macro vparam,param


if GRAD_IDIV
	; d_p_x = (d_p_1.zb.main.y - d_p_2.zb.top.y)/g_denom
	;
		mov	eax,[ebp].temp_vertex.vparam	; d_p_2
		sub	eax,[esi].temp_vertex.vparam
		push	eax
		imul	zb.top.y
		mov	ebx,eax
		mov	ecx,edx
		mov	eax,[edi].temp_vertex.vparam	; d_p_1
		sub	eax,[esi].temp_vertex.vparam
		push	eax
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx

	 	idiv	g_divisor
		mov	param.grad_x,eax

	; d_p_y = (d_p_2.zb.top.x - d_p_1.zb.main.x)/g_denom
	;
		pop	eax				; d_p_1
		imul	zb.main.x
		mov	ebx,eax
		mov	ecx,edx
		pop	eax				; d_p_2
		imul	zb.top.x
		sub	eax,ebx
		sbb	edx,ecx

	 	idiv	g_divisor
		mov	param.grad_y,eax
endif
if GRAD_IMUL
	; d_p_x = (d_p_1.zb.main.y - d_p_2.zb.top.y)/g_denom
	;
		mov	eax,[ebp].temp_vertex.vparam	; d_p_2
		sub	eax,[esi].temp_vertex.vparam
		push	eax
		imul	zb.top.y
		mov	ebx,eax
		mov	ecx,edx
		mov	eax,[edi].temp_vertex.vparam	; d_p_1
		sub	eax,[esi].temp_vertex.vparam
		push	eax
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx

		FIX_MUL
		imul	g_divisor
		FIX_MUL
		mov	param.grad_x,eax

	; d_p_y = (d_p_2.zb.top.x - d_p_1.zb.main.x)/g_denom
	;
		pop	eax				; d_p_1
		imul	zb.main.x
		mov	ebx,eax
		mov	ecx,edx
		pop	eax				; d_p_2
		imul	zb.top.x
		sub	eax,ebx
		sbb	edx,ecx

		FIX_MUL
		imul	g_divisor
		FIX_MUL
		mov	param.grad_y,eax
endif
if GRAD_FMUL
	; d_p_x = (d_p_1.zb.main.y - d_p_2.zb.top.y)/g_denom
	;
		mov	eax,[ebp].temp_vertex.vparam	; d_p_2
		sub	eax,[esi].temp_vertex.vparam
		push	eax
		imul	zb.top.y
		mov	ebx,eax
		mov	ecx,edx
		mov	eax,[edi].temp_vertex.vparam	; d_p_1
		sub	eax,[esi].temp_vertex.vparam
		push	eax
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx

		mov	g_d_temp,eax
		mov	g_d_temph,edx

		fild	qword ptr g_d_temp

		fmul	g_divisor
		fistp	param.grad_x

	; d_p_y = (d_p_2.zb.top.x - d_p_1.zb.main.x)/g_denom
	;
		pop	eax				; d_p_1
		imul	zb.main.x
		mov	ebx,eax
		mov	ecx,edx
		pop	eax				; d_p_2
		imul	zb.top.x
		sub	eax,ebx
		sbb	edx,ecx

		mov	g_d_temp,eax
		mov	g_d_temph,edx

		fild	qword ptr g_d_temp

		fmul	g_divisor
		fistp	param.grad_y
endif

	; Initialise parameter to starting pixel centre
	;
		mov	eax,param.grad_y		; p = p0x+pdy*d_p_y+pdx*d_p_x
		imul	p0_offset_y
		mov	ecx,eax
		mov	ebx,edx
		mov	eax,param.grad_x
		imul	p0_offset_x
		add	eax,ecx
		adc	edx,ebx
		FIX_MUL
		add	eax,[esi].temp_vertex.vparam
		mov	param.current,eax

	; Work out parameter increments per scanline
	;
		movsx	eax,word ptr zb.main.grad+2
		imul	param.grad_x
		add	eax,param.grad_y
		mov	param.d_nocarry,eax
		add	eax,param.grad_x
		mov	param.d_carry,eax
		endm

; Invoke the trapezoid macro for the appropriate renderer
;
TRIANGLE_RENDER	macro	trapezoid_mac

	; See which side has long edge
	;
if GRAD_IDIV
	 	cmp	g_divisor,0
		jl	reversed
endif
if GRAD_IMUL
	 	cmp	g_divisor,0
		jl	reversed
endif
if GRAD_FMUL
		cmp dword ptr g_divisor+4,0
		jl	reversed
endif

; Scan convert top half of triangle - if any - forwards
;
%		trapezoid_mac	top,1

; Scan convert bottom half of triangle - if any - forwards
;
%		trapezoid_mac	bot,1
quit:		ret

reversed:
; Scan convert top half of triangle - if any - backwards
;
%		trapezoid_mac	top,0

; Scan convert bottom half of triangle - if any - backwards
;
%		trapezoid_mac	bot,0
		ret

 		endm

	.data
		db '$Id: ti8_pfz.asm 1.6 1995/05/25 13:25:15 sam Exp $',0
		align 4

p0_offset_x	dd	0	; Offset of start pixel centre from vertex 0
p0_offset_y	dd	0

temp_i		dd	0
temp_u		dd	0
temp_v		dd	0

if GRAD_IMUL
g_divisor	dd	0	; Bottom of param. gradient calc.
endif

if GRAD_IDIV
g_divisor	dd	0	; Bottom of param. gradient calc.
endif

if GRAD_FMUL
g_divisor	real8	0.0	; Bottom of param. gradient calc.
fixed_one	real4	65536.0
g_d_temp	dd	0
g_d_temph	dd	0
endif
	.code

; Trapezoid loop for flat shaded z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PFZ2	macro	half,forward
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:
		push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; bl:	colour
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
	if forward
		cmp	esi,ecx
		jae	short no_pixels
pixel_loop:
		mov	eax,edx
		shr	eax,16
		cmp	ax,[edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	[edi],ax	;	*zptr = z

if FRAMEBUFFER_FAR
		mov	es:[esi],bl	;	*ptr = colour
else
		mov	[esi],bl	;	*ptr = colour
endif
pixel_behind:
		add	edx,ebp		; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	     	cmp	esi,ecx
		jb	short pixel_loop
	else
		cmp	esi,ecx
		jbe	short no_pixels
pixel_loop:
		sub	edx,ebp		; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++

		mov	eax,edx
		shr	eax,16
		cmp	ax,[edi]	; if (z > *zptr)
		jae	short pixel_behind

		mov	[edi],ax	;	*zptr = z

if FRAMEBUFFER_FAR
		mov	es:[esi],bl	;	*ptr = colour
else
		mov	[esi],bl	;	*ptr = colour
endif

pixel_behind:
	     	cmp	esi,ecx
		ja	short pixel_loop
	endif
no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f		; x+= zb.main.grad (fraction)
		add	zb.main.f,eax
		jc	start_carried

		add	esi,zb.main.d_i	; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

		add	zb.half.f,edi	    ; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i ; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	short scan_loop
		jmp	short done_trapezoid

start_carried:	adc	esi,zb.main.d_i	; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

		add	zb.half.f,edi	    ; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i ; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
	endm

; void TriangleRenderPFZ2(struct br_vertex *pvertex_0,
;			struct br_vertex *pvertex_1,
;			struct br_vertex *pvertex_2);
;
; C stub directly to triangle renderer
;
TriangleRenderPFZ2 proc uses eax ebx ecx edx esi edi,
		pvertex_0 : ptr word,
		pvertex_1 : ptr word,
		pvertex_2 : ptr word

	; Get pointers to vertex structures and colour
	;
	; eax = colour
	; esi = vertex 0
	; edi = vertex 1
	; ebp = vertex 2
	;
		push	ebp
if FRAMEBUFFER_FAR
		push	es
endif
		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PFZ2
if FRAMEBUFFER_FAR
		pop	es
endif
		pop	ebp
		ret
TriangleRenderPFZ2 endp

; Triangle_PFZ2
;
; Render a triangle into frame buffer
;
;	Flat colour
;	Z buffer
;	Fractional vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PFZ2 proc

		TRIANGLE_SETUP_PF
		TRIANGLE_PARAM_PF v[Z*4],zb.pz

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x
		mov	bl,byte ptr zb.pi.current+2
if FRAMEBUFFER_FAR
		mov	es,word ptr zb.colour_buffer+4
endif

		TRIANGLE_RENDER TRAPEZOID_PFZ2

RawTriangle_PFZ2 endp

; void TriangleRenderPFZ2I(struct br_vertex *pvertex_0,
;			struct br_vertex *pvertex_1,
;			struct br_vertex *pvertex_2);
;
; C stub directly to triangle renderer
;
TriangleRenderPFZ2I proc uses eax ebx ecx edx esi edi,
		pvertex_0 : ptr word,
		pvertex_1 : ptr word,
		pvertex_2 : ptr word

	; Get pointers to vertex structures and colour
	;
	; eax = colour
	; esi = vertex 0
	; edi = vertex 1
	; ebp = vertex 2
	;
		push	ebp
if FRAMEBUFFER_FAR
		push	es
endif
		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PFZ2I
if FRAMEBUFFER_FAR
		pop	es
endif
		pop	ebp
		ret
TriangleRenderPFZ2I endp

; Trapezoid loop for gouraud shaded z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PFZ2I	macro	half,forward
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:
		push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer
		mov	eax,zb.pi.current

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; bl:	colour
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
	if forward
		cmp	esi,ecx
		jae	short no_pixels
pixel_loop:
		mov	ebx,edx
		shr	ebx,16
		cmp	bx,[edi]	; if (z > *zptr)
		jae	short pixel_behind

ifndef Z_MASK
		mov	[edi],bx	;	*zptr = z
else
		mov	ebx,edx
		and	ebx,Z_MASK
		mov	[edi],ebx
endif

		mov	ebx,eax
		shr	ebx,16
if FRAMEBUFFER_FAR
		mov	es:[esi],bl	;	*ptr = colour
else
		mov	[esi],bl	;	*ptr = colour
endif
pixel_behind:
		add	edx,ebp		; z += d_z_z
		add	eax,zb.pi.grad_x
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	     	cmp	esi,ecx
		jb	short pixel_loop
	else
		cmp	esi,ecx
		jbe	short no_pixels
pixel_loop:
		sub	edx,ebp		; z += d_z_z
		sub	eax,zb.pi.grad_x
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++

		mov	ebx,edx
		shr	ebx,16
		cmp	bx,[edi]	; if (z > *zptr)
		jae	short pixel_behind

ifndef Z_MASK
		mov	[edi],bx	;	*zptr = z
else
		mov	ebx,edx
		and	ebx,Z_MASK
		mov	[edi],ebx
endif
		mov	ebx,eax
		shr	ebx,16
if FRAMEBUFFER_FAR
		mov	es:[esi],bl	;	*ptr = colour
else
		mov	[esi],bl	;	*ptr = colour
endif
pixel_behind:
	     	cmp	esi,ecx
		ja	short pixel_loop
	endif
no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f		; x+= zb.main.grad (fraction)
		add	zb.main.f,eax
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry
		mov	eax,zb.pi.d_nocarry
		add	zb.pi.current,eax

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	short scan_loop
		jmp	short done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry
		mov	eax,zb.pi.d_carry
		add	zb.pi.current,eax

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PFZ2I
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour index
;	Linear interpolated Z value
;	Fractional vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PFZ2I proc

		TRIANGLE_SETUP_PF
		TRIANGLE_PARAM_PF v[Z*4],zb.pz
		TRIANGLE_PARAM_PF comp[C_I*4],zb.pi

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x
if FRAMEBUFFER_FAR
		mov	es,word ptr zb.colour_buffer+4
endif
		TRIANGLE_RENDER TRAPEZOID_PFZ2I

RawTriangle_PFZ2I endp


TriangleRenderPFZ2T proc uses eax ebx ecx edx esi edi,
		pvertex_0 : ptr word,
		pvertex_1 : ptr word,
		pvertex_2 : ptr word

	; Get pointers to vertex structures and colour
	;
	; eax = colour
	; esi = vertex 0
	; edi = vertex 1
	; ebp = vertex 2
	;
		push	ebp
if FRAMEBUFFER_FAR
		push	es
endif
		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PFZ2T
if FRAMEBUFFER_FAR
		pop	es
endif
		pop	ebp
		ret
TriangleRenderPFZ2T endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PFZ2T	macro	half,forward
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:
		push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	temp_u,eax
		mov	temp_v,ebx

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; bl:	colour
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
	if forward
		cmp	esi,ecx
		jae	no_pixels
pixel_loop:
		mov	ebx,edx
		shr	ebx,16
		cmp	bx,[edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	[edi],bx	;	*zptr = z

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+3
		mov	bh,byte ptr temp_v+3
		add	ebx,zb.texture_buffer
		mov	bl,[ebx]

if FRAMEBUFFER_FAR
		mov	es:[esi],bl	;	*ptr = colour
else
		mov	[esi],bl	;	*ptr = colour
endif
pixel_behind:
		add	edx,ebp		; z += d_z_z

	;; Per pixel update - forwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	     	cmp	esi,ecx
		jb	pixel_loop
	else
		cmp	esi,ecx
		jbe	no_pixels
pixel_loop:
		sub	edx,ebp		; z += d_z_z

	;; Per pixel update - backwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edi,2		; z_ptr++
		dec	esi		; ptr++

		mov	ebx,edx
		shr	ebx,16
		cmp	bx,[edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	[edi],bx	;	*zptr = z

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+3
		mov	bh,byte ptr temp_v+3
		add	ebx,zb.texture_buffer
		mov	bl,[ebx]

if FRAMEBUFFER_FAR
		mov	es:[esi],bl	;	*ptr = colour
else
		mov	[esi],bl	;	*ptr = colour
endif
pixel_behind:
	     	cmp	esi,ecx
		ja	pixel_loop
	endif
no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f		; x+= zb.main.grad (fraction)
		add	zb.main.f,eax
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

if 1
	;; Per scanline parameter update - no carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx
endif

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

if 1
	;; Per scanline parameter update - carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx
endif

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PFZ2T
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated Z value
;	Fractional vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PFZ2T proc

		TRIANGLE_SETUP_PF
		TRIANGLE_PARAM_PF v[Z*4],zb.pz
		TRIANGLE_PARAM_PF comp[C_U*4],zb.pu
		TRIANGLE_PARAM_PF comp[C_V*4],zb.pv

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

if FRAMEBUFFER_FAR
		mov	es,word ptr zb.colour_buffer+4
endif
		TRIANGLE_RENDER TRAPEZOID_PFZ2T

RawTriangle_PFZ2T endp

TriangleRenderPFZ2TI proc uses eax ebx ecx edx esi edi,
		pvertex_0 : ptr word,
		pvertex_1 : ptr word,
		pvertex_2 : ptr word

	; Get pointers to vertex structures and colour
	;
	; eax = colour
	; esi = vertex 0
	; edi = vertex 1
	; ebp = vertex 2
	;
		push	ebp
if FRAMEBUFFER_FAR
		push	es
endif
		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PFZ2TI
if FRAMEBUFFER_FAR
		pop	es
endif
		pop	ebp
		ret
TriangleRenderPFZ2TI endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PFZ2TI	macro	half,forward
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:
		push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	ebp,zb.pi.current
		mov	temp_u,eax
		mov	temp_v,ebx
		mov	temp_i,ebp

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; bl:	colour
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
	if forward
		cmp	esi,ecx
		jae	no_pixels
pixel_loop:
		mov	ebx,edx
		shr	ebx,16
		cmp	bx,[edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	[edi],bx	;	*zptr = z

	;; Per pixel fetch colour
	;;
		xor	eax,eax
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+3
		mov	bh,byte ptr temp_v+3
		add	ebx,zb.texture_buffer
		mov	al,[ebx]
		mov	ah,byte ptr temp_i+2
		add	eax,zb.shade_table
		mov	al,[eax]

if FRAMEBUFFER_FAR
		mov	es:[esi],al	;	*ptr = colour
else
		mov	[esi],al	;	*ptr = colour
endif
pixel_behind:
		add	edx,zb.pz.grad_x ; z += d_z_z

	;; Per pixel update - forwards
	;;
		mov	ebp,temp_i
		mov	eax,temp_u
		mov	ebx,temp_v
		add	ebp,zb.pi.grad_x
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		mov	temp_i,ebp
		mov	temp_u,eax
		mov	temp_v,ebx

		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	     	cmp	esi,ecx
		jb	pixel_loop
	else
		cmp	esi,ecx
		jbe	no_pixels
pixel_loop:
		sub	edx,zb.pz.grad_x ; z += d_z_z

	;; Per pixel update - backwards
	;;
		mov	ebp,temp_i
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	ebp,zb.pi.grad_x
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_i,ebp
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edi,2		; z_ptr++
		dec	esi		; ptr++

		mov	ebx,edx
		shr	ebx,16
		cmp	bx,[edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	[edi],bx	;	*zptr = z

	;; Per pixel fetch colour
	;;
		xor	eax,eax
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+3
		mov	bh,byte ptr temp_v+3
		add	ebx,zb.texture_buffer
		mov	al,[ebx]
		mov	ah,byte ptr temp_i+2
		add	eax,zb.shade_table
		mov	al,[eax]

if FRAMEBUFFER_FAR
		mov	es:[esi],al	;	*ptr = colour
else
		mov	[esi],al	;	*ptr = colour
endif
pixel_behind:
	     	cmp	esi,ecx
		ja	pixel_loop
	endif
no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f		; x+= zb.main.grad (fraction)
		add	zb.main.f,eax
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	ebp,zb.pi.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	ebp,zb.pi.d_nocarry
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pi.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	ebp,zb.pi.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	ebp,zb.pi.d_carry
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pi.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PFZ2TI
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated shade index
;	Linear interpolated Z value
;	Fractional vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PFZ2TI proc

		TRIANGLE_SETUP_PF
		TRIANGLE_PARAM_PF v[Z*4],zb.pz
		TRIANGLE_PARAM_PF comp[C_I*4],zb.pi
		TRIANGLE_PARAM_PF comp[C_U*4],zb.pu
		TRIANGLE_PARAM_PF comp[C_V*4],zb.pv

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
if FRAMEBUFFER_FAR
		mov	es,word ptr zb.colour_buffer+4
endif
		TRIANGLE_RENDER TRAPEZOID_PFZ2TI

RawTriangle_PFZ2TI endp


	end

