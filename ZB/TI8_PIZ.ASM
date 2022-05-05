;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: ti8_piz.asm 1.24 1995/08/31 16:47:42 sam Exp $
;; $Locker:  $
;;
;; 8 bit Indexed mode
;;
;; Triangle scan convertion and rendering
;;
;; Perfect scan, integer vertices, Z  buffered
;;
	.386p
	.model small,c

	include	zb.inc
	include 586_macs.inc

USE_RCP		equ	1

if 1
SHOW_CODESIZE	macro _name,_total
		%out _name = _total
		endm
else
SHOW_CODESIZE		macro x,y
		endm
endif

X	equ 0
Y	equ 1
Z	equ 2

_X	equ 0
_Y	equ 4
_Z	equ 8

;; Setup some macros that are used to put colour, texture and depth buffers in different
;; segments
;;
if 0
CSEG		equ	es:
DSEG		equ
TSEG		equ
SETUP_SEGS	macro
;		push	es
;		mov	es,word ptr zb.colour_buffer+4
;		endm
RESTORE_SEGS	macro
;		pop	es
		endm
else
CSEG		equ
DSEG		equ
TSEG		equ
SETUP_SEGS	macro
		endm
RESTORE_SEGS	macro
		endm
endif


;; Various macros for generating the triangle renderers
;;

; Divide edx:eax by g_divisor with no overflow
;
; Assumes Z flag has sign of numerator
;
; Trashes ebx
;
;
GDIVIDE		macro	dirn
		local	negative,done,overflow

		mov	ebx,g_divisor
		js	negative
		cmp	ebx,edx
		jbe	overflow
	 	div	ebx
	ifidni <dirn>,<b>
		neg	eax
	endif
		jmp	done

negative:	neg	edx
		neg	eax
		sbb	edx,0
		cmp	ebx,edx
		jbe	overflow
	 	div	ebx
	ifidni <dirn>,<f>
		neg	eax
	endif
overflow:
done:
		endm

; Comparison that change sense with rendering direction
;
CMP_f		macro	a,b
		cmp	a,b
		endm

CMP_b		macro	a,b
		cmp	b,a
		endm

; Equates for the vertex coordinates
;
if SCREEN_FIXED
V0_X		equ	<vertex_0[_X]>
V0_Y		equ	<vertex_0[_Y]>
V1_X		equ	<vertex_1[_X]>
V1_Y		equ	<vertex_1[_Y]>
V2_X		equ	<vertex_2[_X]>
V2_Y		equ	<vertex_2[_Y]>
else
V0_X		equ	<[esi].temp_vertex_fixed.v[_X]>
V0_Y		equ	<[esi].temp_vertex_fixed.v[_Y]>
V1_X		equ	<[edi].temp_vertex_fixed.v[_X]>
V1_Y		equ	<[edi].temp_vertex_fixed.v[_Y]>
V2_X		equ	<[ebp].temp_vertex_fixed.v[_X]>
V2_Y		equ	<[ebp].temp_vertex_fixed.v[_Y]>
endif

; Setup of an integer, perfect scan triangle  
;
; Leave g_divisor in eax, with sign of g_divisor in flags
;
SETUP_PI	macro

_setup_start = $

	; Get Y components of vertices
	;
		mov	eax,[esi].temp_vertex_fixed.v[_Y]
		mov	ebx,[edi].temp_vertex_fixed.v[_Y]
		mov	ecx,[ebp].temp_vertex_fixed.v[_Y]

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
		SWAP	edi,ebp
		jmp	short sort_done
sort_2:
	; cab
		SWAP	edi,ebp
		SWAP	esi,edi
		jmp	short sort_done
sort_3:
		cmp	ecx,ebx
		jg	short sort_4
	; cba
		SWAP	esi,ebp
		jmp	short sort_done

sort_4:		cmp	eax,ecx
		jg	short sort_5
	; bac
		SWAP	esi,edi
		jmp	short sort_done
sort_5:
	; bca
		SWAP	esi,edi
		SWAP	edi,ebp
sort_done:

if SCREEN_FIXED
	; Work out integer vertex values
	;
	; XXX Should really do this at projection time
	;
		mov	eax,[esi].temp_vertex_fixed.v[_X]
		mov	ebx,[edi].temp_vertex_fixed.v[_X]
		mov	ecx,[ebp].temp_vertex_fixed.v[_X]
		sar	eax,16
		sar	ebx,16
		sar	ecx,16
		mov	vertex_0[_X],eax
		mov	vertex_1[_X],ebx
		mov	vertex_2[_X],ecx

		mov	eax,[esi].temp_vertex_fixed.v[_Y]
		mov	ebx,[edi].temp_vertex_fixed.v[_Y]
		mov	ecx,[ebp].temp_vertex_fixed.v[_Y]
		sar	eax,16
		sar	ebx,16
		sar	ecx,16
		mov	vertex_0[_Y],eax
		mov	vertex_1[_Y],ebx
		mov	vertex_2[_Y],ecx
endif

	; Work out deltas and gradients along edges
	;

		; long edge
		;
		mov	ebx,V2_Y	; zb.main.y
		sub	ebx,V0_Y
		je	quit		; Ignore zero height polys
		mov	zb.main.y,ebx

if USE_RCP
		mov	eax,V2_X	; zb.main.x
		sub	eax,V0_X
		mov	zb.main.x,eax
					; zb.main.grad = zb.main.x/zb.main.y
		imul	_reciprocal[ebx*4]
else
		mov	edx,V2_X	; zb.main.x
		sub	edx,V0_X
		mov	zb.main.x,edx

		FIX_DIV
		idiv	ebx
endif
		mov	zb.main.grad,eax
		mov	word ptr zb.main.d_f+2,ax
		sar	eax,16
		add	eax,zb.row_width
		mov	zb.main.d_i,eax

		; top short edge
		;
if USE_RCP
		mov	eax,V1_X		; zb.top.x
		sub	eax,V0_X
		mov	zb.top.x,eax

		mov	ebx,V1_Y		; zb.top.y
		sub	ebx,V0_Y
		mov	zb.top.y,ebx
		mov	zb.top.count,ebx
		je	short no_top

		imul	_reciprocal[ebx*4]
else
		mov	edx,V1_X		; zb.top.x
		sub	edx,V0_X
		mov	zb.top.x,edx

		mov	ebx,V1_Y		; zb.top.y
		sub	ebx,V0_Y
		mov	zb.top.y,ebx
		mov	zb.top.count,ebx
		je	short no_top

		FIX_DIV				; zb.top.grad = zb.top.x/zb.top.y
		idiv	ebx
endif

		mov	zb.top.grad,eax
		mov	word ptr zb.top.d_f+2,ax
		sar	eax,16
		add	eax,zb.row_width
		mov	zb.top.d_i,eax
no_top:
		; bottom short edge (don't need to save deltas)
		;
		mov	ebx,V2_Y		; zb.bot.y
		sub	ebx,V1_Y
		mov	zb.bot.count,ebx
		je	short no_bottom

if USE_RCP
		mov	eax,V2_X		; zb.bot.x
		sub	eax,V1_X

		imul	_reciprocal[ebx*4]
else
		mov	edx,V2_X		; zb.bot.x
		sub	edx,V1_X

		FIX_DIV				; zb.bot.grad = zb.bot.x/zb.bot.y
		idiv	ebx
endif

		mov	zb.bot.grad,eax
		mov	word ptr zb.bot.d_f+2,ax
		sar	eax,16
		add	eax,zb.row_width
		mov	zb.bot.d_i,eax

no_bottom:

	; Work out start values for edges as pixel offsets
	;
if 1
		mov	ebx,zb.row_width
		mov	ecx,dword ptr zb.colour_buffer

		mov	eax,V0_Y
		imul	ebx
		add	eax,V0_X

		mov	zb.main.f,080000000h
		mov	zb.main.i,eax
		add	eax,ecx
		mov	zb.top.f,080000000h
		mov	zb.top.i,eax

		mov	eax,V1_Y
		imul	ebx
		add	eax,V1_X
else
		lea	ebx,zb.row_table
		mov	ecx,dword ptr zb.colour_buffer

		mov	eax,V0_Y
		mov	eax,[ebx+eax*4]
		add	eax,V0_X

		mov	zb.main.f,080000000h
		mov	zb.main.i,eax
		add	eax,ecx
		mov	zb.top.f,080000000h
		mov	zb.top.i,eax

		mov	eax,V1_Y
		mov	eax,[ebx+eax*4]
		add	eax,V1_X
endif

		add	eax,ecx
		mov	zb.bot.f,080000000h
		mov	zb.bot.i,eax

	; Work out divisor for use in parameter gradient calcs.
	;
		mov	eax,zb.main.x
		imul	zb.top.y
		mov	ebx,eax
		mov	eax,zb.top.x
		imul	zb.main.y
		sub	eax,ebx

		je	quit			; quit if g_divisor is 0

						; Leave g_divisor in eax
		endm

; Per parameter initialisation of perfect scan, integer vertex triangle
;
PARAM_PI_DIRN macro vparam,param,dirn

	; d_p_x = (d_p_1.zb.main.y - d_p_2.zb.top.y)/g_denom
	;
		mov	eax,[ebp].temp_vertex_fixed.vparam	; d_p_2
		sub	eax,[esi].temp_vertex_fixed.vparam
		push	eax
		imul	zb.top.y
		mov	ebx,eax
		mov	eax,[edi].temp_vertex_fixed.vparam	; d_p_1
		mov	ecx,edx

		sub	eax,[esi].temp_vertex_fixed.vparam
		push	eax
		imul	zb.main.y
		sub	eax,ebx
		sbb	edx,ecx

		GDIVIDE dirn

		mov	param.grad_x,eax

	; d_p_y = (d_p_2.zb.top.x - d_p_1.zb.main.x)/g_denom
	;
		pop	eax				; d_p_1
		imul	zb.main.x
		mov	ebx,eax
		pop	eax				; d_p_2
		mov	ecx,edx
		imul	zb.top.x
		sub	eax,ebx
		sbb	edx,ecx

		GDIVIDE dirn

		mov	param.grad_y,eax

	; Initialise parameter  from top vertex
	;
		mov	ebx,param.grad_x
		mov	eax,[esi].temp_vertex_fixed.vparam

		sar	ebx,1
		adc	eax,ebx		

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

	.data
		db '$Id: ti8_piz.asm 1.24 1995/08/31 16:47:42 sam Exp $',0
		align 4

	; reciprocal table from fixed386.asm
	;
		extern _reciprocal:dword

p0_offset_x	dd	0	; Offset of start pixel centre from vertex 0
p0_offset_y	dd	0

temp_i		dd	0
temp_u		dd	0
temp_v		dd	0
temp_r		dd	0
temp_g		dd	0
temp_b		dd	0
temp_y		dd	0

	; Integer vertex poitisions
	;
vertex_0	dd	2 dup(?)
vertex_1	dd	2 dup(?)
vertex_2	dd	2 dup(?)

g_divisor	dd	0	; Bottom of param. gradient calc.

	.code

if 0

; void TriangleRenderPIZ2(struct br_vertex *pvertex_0,
;			struct br_vertex *pvertex_1,
;			struct br_vertex *pvertex_2);
;
; C stub directly to triangle renderer
;
TriangleRenderPIZ2 proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2 endp


; Trapezoid loop for flat shaded z (16 bit) buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2	macro	half,dirn
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
	; edx:	z 	(16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	short no_pixels

pixel_loop:
	ifidni <dirn>,<b>
		sub	edx,ebp		; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif
		mov	eax,edx
		shr	eax,16
		cmp	ax, DSEG [edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	DSEG [edi],ax	;	*zptr = z
		mov	CSEG [esi],bl	;	*ptr = colour
pixel_behind:

	ifidni <dirn>,<f>
		add	edx,ebp		; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif
		CMP_&dirn esi,ecx
		jb	short pixel_loop
no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

		dec	zb.half.count
		jne	short scan_loop
		jmp	short done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
	endm

; Triangle_PIZ2
;
; Render a triangle into frame buffer
;
;	Flat colour
;	Z buffer
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2 proc
		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

	; Scan forwards
	;
		PARAM_PI_DIRN v[Z*4],zb.pz,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x
		mov	bl,byte ptr zb.pi.current+2

		TRAPEZOID_PIZ2 top,f
		TRAPEZOID_PIZ2 bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x
		mov	bl,byte ptr zb.pi.current+2

		TRAPEZOID_PIZ2 top,b
		TRAPEZOID_PIZ2 bot,b
		ret


	SHOW_CODESIZE	<RawTriangle_PIZ2>,<%($-_setup_start)>

RawTriangle_PIZ2 endp

endif	; Hawkeye

; void TriangleRenderPIZ2I(struct br_vertex *pvertex_0,
;			struct br_vertex *pvertex_1,
;			struct br_vertex *pvertex_2);
;
; C stub directly to triangle renderer
;
TriangleRenderPIZ2I proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2I

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2I endp

; Trapezoid loop for gouraud shaded z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2I	macro	half,dirn
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
	; eax:	colour	(16.16)
	; ecx:	address of end of scanline
	; edx:	z 	(16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	short no_pixels
pixel_loop:

	ifidni <dirn>,<b>
		sub	edx,ebp		; z += d_z_z
		sub	eax,zb.pi.grad_x
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

		mov	ebx,edx
		shr	ebx,16
		cmp	bx, DSEG [edi]	; if (z > *zptr)
		jae	short pixel_behind

		mov	DSEG [edi],bx	;	*zptr = z

		mov	ebx,eax
		shr	ebx,16
		mov	CSEG [esi],bl	;	*ptr = colour
pixel_behind:

	ifidni <dirn>,<f>
		add	edx,ebp		; z += d_z_z
		add	eax,zb.pi.grad_x
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif
		CMP_&dirn esi,ecx
		jb	short pixel_loop

no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry
		mov	eax,zb.pi.d_nocarry
		add	zb.pi.current,eax

		dec	zb.half.count
		jne	short scan_loop
		jmp	short done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry
		mov	eax,zb.pi.d_carry
		add	zb.pi.current,eax

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ2I
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour index
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2I proc
		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_I*4],zb.pi,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2I top,f
		TRAPEZOID_PIZ2I bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_I*4],zb.pi,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2I top,b
		TRAPEZOID_PIZ2I bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2I>,<%($-_setup_start)>
RawTriangle_PIZ2I endp

if 0		; Hawkeye

TriangleRenderPIZ2T proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2T

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2T endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2T	macro	half,dirn
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
	; ecx:	address of end of scanline
	; edx:	z 	(16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels
pixel_loop:
	ifidni <dirn>,<b>
	;; Per pixel update - backwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edx,ebp		; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

	;; Depth test
	;;
		mov	eax,edx
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		add	ebx,zb.texture_buffer
		mov	bl,TSEG [ebx]

		and	bl,bl		; Check for transparency
		je	pixel_behind

		mov	DSEG [edi],ax	;	*zptr = z
		mov	CSEG [esi],bl	;	*ptr = colour

pixel_behind:

	ifidni <dirn>,<f>
	;; Per pixel update - forwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		add	edx,ebp		; z += d_z_z
		inc	esi		; ptr++
		add	edi,2		; z_ptr++
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop

no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax  		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ2T
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2T proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2T top,f
		TRAPEZOID_PIZ2T bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2T top,b
		TRAPEZOID_PIZ2T bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2T>,<%($-_setup_start)>
RawTriangle_PIZ2T endp


TriangleRenderPIZ2TI proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2TI

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2TI endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
;	Hawkeye Changing quite a bit...

TRAPEZOID_PIZ2TI	macro	half,dirn
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
		add	esi,dword ptr zb.colour_buffer

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	ebp,zb.pi.current
		mov	temp_u,eax
		mov	temp_v,ebx
		mov	temp_i,ebp
		lea	edi,[edi+esi*2]

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	short no_pixels

pixel_loop:
	ifidni <dirn>,<b>
		mov	ebp,temp_i
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	edx,zb.pz.grad_x ; z += d_z_z
		sub	ebp,zb.pi.grad_x
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		sub	edi,2		; z_ptr++
		mov	temp_i,ebp
		mov	temp_u,eax
		mov	temp_v,ebx
		dec	esi		; ptr++
	endif

		mov	eax,edx
		xor	ebx,ebx

	;; Depth test
	;;
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		mov	bl,byte ptr temp_u+2
		mov	ebp,zb.texture_buffer
		mov	bh,byte ptr temp_v+2
		mov	bl,TSEG [ebp+ebx]
;		and	bl,bl		; Check for transparency
;		test	bl,bl
;		je	pixel_behind

		mov	DSEG [edi],ax	;	*zptr = z
		mov	bh,byte ptr temp_i+2
		mov	eax,temp_u
		mov	ebp,temp_i
		add	ebx,zb.shade_table
		mov	bl,[ebx]
		inc	esi		; ptr++
		add	edi,2		; z_ptr++
		mov	CSEG [esi],bl	;	*ptr = colour
		jmp	short cheat

pixel_behind:

	ifidni <dirn>,<f>
	;; Per pixel update - forwards
	;;
		mov	ebp,temp_i
		mov	eax,temp_u
		inc	esi		; ptr++
		add	edi,2		; z_ptr++
cheat:
		mov	ebx,temp_v
		add	ebp,zb.pi.grad_x
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		add	edx,zb.pz.grad_x ; z += d_z_z
		mov	temp_i,ebp
		mov	temp_u,eax
		mov	temp_v,ebx
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop
no_pixels:
	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
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

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	
		adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY

	;; Per scanline parameter update - carry
	;;
		mov	ebp,zb.pi.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	edx,zb.pz.d_carry
		add	ebp,zb.pi.d_carry
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pi.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ2TI
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated shade index
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2TI proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_I*4],zb.pi,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TI top,f
		TRAPEZOID_PIZ2TI bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_I*4],zb.pi,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TI top,b
		TRAPEZOID_PIZ2TI bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2TI>,<%($-_setup_start)>
RawTriangle_PIZ2TI endp

if 0
; void TriangleRenderPIZ4(struct br_vertex *pvertex_0,
;			struct br_vertex *pvertex_1,
;			struct br_vertex *pvertex_2);
;
; C stub directly to triangle renderer
;
TriangleRenderPIZ4 proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ4

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ4 endp

; Trapezoid loop for flat shaded z (32 bit) buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ4	macro	half,dirn
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:	push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*4]
		add	esi,dword ptr zb.colour_buffer

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; bl:	colour
	; ecx:	address of end of scanline
	; edx:	z 	(16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	short no_pixels
pixel_loop:
	ifidni <dirn>,<b>
		sub	edx,ebp		; z += d_z_z
		sub	edi,4		; z_ptr++
		dec	esi		; ptr++
	endif
		cmp	edx, DSEG [edi]	; if (z > *zptr)
		jae	short pixel_behind
		mov	DSEG [edi],edx	;	*zptr = z
		mov	CSEG [esi],bl	;	*ptr = colour
pixel_behind:

	ifidni <dirn>,<f>
		add	edx,ebp		; z += d_z_z
		add	edi,4		; z_ptr++
		inc	esi		; ptr++
	endif
		CMP_&dirn esi,ecx
		jb	short pixel_loop
no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

		dec	zb.half.count
		jne	short scan_loop
		jmp	short done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
	endm

; Triangle_PIZ4
;
; Render a triangle into frame buffer
;
;	Flat colour
;	Z buffer
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ4 proc
		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

	; Scan forwards
	;
		PARAM_PI_DIRN v[Z*4],zb.pz,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x
		mov	bl,byte ptr zb.pi.current+2

		TRAPEZOID_PIZ4 top,f
		TRAPEZOID_PIZ4 bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x
		mov	bl,byte ptr zb.pi.current+2

		TRAPEZOID_PIZ4 top,b
		TRAPEZOID_PIZ4 bot,b
		ret


	SHOW_CODESIZE	<RawTriangle_PIZ4>,<%($-_setup_start)>

RawTriangle_PIZ4 endp

; void TriangleRenderPIZ4I(struct br_vertex *pvertex_0,
;			struct br_vertex *pvertex_1,
;			struct br_vertex *pvertex_2);
;
; C stub directly to triangle renderer
;
TriangleRenderPIZ4I proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ4I

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ4I endp

; Trapezoid loop for gouraud shaded z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ4I	macro	half,dirn
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
		lea	edi,[edi+esi*4]
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
		CMP_&dirn esi,ecx
		jae	short no_pixels
pixel_loop:

	ifidni <dirn>,<b>
		sub	edx,ebp		; z += d_z_z
		sub	eax,zb.pi.grad_x
		sub	edi,4		; z_ptr++
		dec	esi		; ptr++
	endif

		cmp	edx, DSEG [edi]	; if (z > *zptr)
		jae	short pixel_behind

		mov	DSEG [edi],edx	;	*zptr = z

		mov	ebx,eax
		shr	ebx,16
		mov	CSEG [esi],bl	;	*ptr = colour
pixel_behind:

	ifidni <dirn>,<f>
		add	edx,ebp		; z += d_z_z
		add	eax,zb.pi.grad_x
		add	edi,4		; z_ptr++
		inc	esi		; ptr++
	endif
		CMP_&dirn esi,ecx
		jb	short pixel_loop

no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry
		mov	eax,zb.pi.d_nocarry
		add	zb.pi.current,eax

		dec	zb.half.count
		jne	short scan_loop
		jmp	short done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry
		mov	eax,zb.pi.d_carry
		add	zb.pi.current,eax

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ4I
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour index
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ4I proc
		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_I*4],zb.pi,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ4I top,f
		TRAPEZOID_PIZ4I bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_I*4],zb.pi,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ4I top,b
		TRAPEZOID_PIZ4I bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ4I>,<%($-_setup_start)>
RawTriangle_PIZ4I endp

TriangleRenderPIZ4T proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ4T

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ4T endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ4T	macro	half,dirn
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
		lea	edi,[edi+esi*4]
		add	esi,dword ptr zb.colour_buffer

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	temp_u,eax
		mov	temp_v,ebx

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels
pixel_loop:
	ifidni <dirn>,<b>
	;; Per pixel update - backwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edx,ebp		; z += d_z_z
		sub	edi,4		; z_ptr++
		dec	esi		; ptr++
	endif
	
	;; Depth test
	;;
		cmp	edx,DSEG [edi]	; if (z > *zptr)
		jae	short pixel_behind

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		add	ebx,zb.texture_buffer
		mov	bl,TSEG [ebx]

		and	bl,bl		; Check for transparency
		je	pixel_behind

		mov	DSEG [edi],edx	;	*zptr = z
		mov	CSEG [esi],bl	;	*ptr = colour
pixel_behind:

	ifidni <dirn>,<f>
	;; Per pixel update - forwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		add	edx,ebp		; z += d_z_z
		add	edi,4		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop

no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax  		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ4T
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ4T proc
		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ4T top,f
		TRAPEZOID_PIZ4T bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ4T top,b
		TRAPEZOID_PIZ4T bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ4T>,<%($-_setup_start)>
RawTriangle_PIZ4T endp

TriangleRenderPIZ4TI proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ4TI

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ4TI endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ4TI	macro	half,dirn
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:	push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*4]
		add	esi,dword ptr zb.colour_buffer

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	ebp,zb.pi.current
		mov	temp_u,eax
		mov	temp_v,ebx
		mov	temp_i,ebp

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	short no_pixels
pixel_loop:
	ifidni <dirn>,<b>
		mov	ebp,temp_i
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	ebp,zb.pi.grad_x
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_i,ebp
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edx,zb.pz.grad_x ; z += d_z_z
		sub	edi,4		; z_ptr++
		dec	esi		; ptr++
	endif
		cmp	edx,DSEG [edi]	; if (z > *zptr)
		jae	short pixel_behind

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		mov	ebp,zb.texture_buffer
		mov	bl,TSEG [ebp+ebx]
		and	bl,bl		; Check for transparency
		je	pixel_behind

		mov	DSEG [edi],edx	;	*zptr = z

		mov	bh,byte ptr temp_i+2
		add	ebx,zb.shade_table
		mov	bl,[ebx]
		mov	CSEG [esi],bl	;	*ptr = colour
		mov	DSEG [edi],edx	;	*zptr = z

pixel_behind:

	ifidni <dirn>,<f>
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

		add	edx,zb.pz.grad_x ; z += d_z_z
		add	edi,4		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop
no_pixels:
	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
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

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ4TI
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated shade index
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ4TI proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_I*4],zb.pi,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ4TI top,f
		TRAPEZOID_PIZ4TI bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_I*4],zb.pi,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ4TI top,b
		TRAPEZOID_PIZ4TI bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ4TI>,<%($-_setup_start)>
RawTriangle_PIZ4TI endp

endif

TriangleRenderPIZ2TN proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2TN

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2TN endp

; Trapezoid loop for linear texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2TN	macro	half,dirn
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:	push	esi
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
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels
pixel_loop:
	ifidni <dirn>,<b>
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	ebp,zb.pr.grad_x
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edx,zb.pz.grad_x ; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

	;; Depth test
	;;
		mov	eax,edx
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		mov	DSEG [edi],ax	;	*zptr = z
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		mov	ebp,zb.texture_buffer
		mov	al,TSEG [ebp+ebx]	; Get texel
		mov	ebp,zb.bump_buffer
		mov	bl,TSEG [ebp+ebx]	; Get normal
		xor	bh,bh

	;; Get lighting level from normal

		mov	ebp,zb.lighting_table
		mov	ah,[ebp+ebx]

	;; Light texel

		mov	ebp,zb.shade_table
		mov	bl,[ebp+eax]

		mov	CSEG [esi],bl	;	*ptr = colour

pixel_behind:

	ifidni <dirn>,<f>
	;; Per pixel update - forwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		mov	temp_u,eax
		mov	temp_v,ebx

		add	edx,zb.pz.grad_x ; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop
no_pixels:
	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	ebp,zb.pr.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	ebp,zb.pr.d_nocarry
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pr.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		mov	ebp,zb.pg.current
		mov	eax,zb.pb.current
		add	ebp,zb.pg.d_nocarry
		add	eax,zb.pb.d_nocarry
		mov	zb.pg.current,ebp
		mov	zb.pb.current,eax

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	ebp,zb.pr.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	ebp,zb.pr.d_carry
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pr.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		mov	ebp,zb.pg.current
		mov	eax,zb.pb.current
		add	ebp,zb.pg.d_carry
		add	eax,zb.pb.d_carry
		mov	zb.pg.current,ebp
		mov	zb.pb.current,eax

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ2TN
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated shade r g and b for coloured lights
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2TN proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_R*4],zb.pr,f
		PARAM_PI_DIRN comp[C_G*4],zb.pg,f
		PARAM_PI_DIRN comp[C_B*4],zb.pb,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TN top,f
		TRAPEZOID_PIZ2TN bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_R*4],zb.pr,b
		PARAM_PI_DIRN comp[C_G*4],zb.pg,b
		PARAM_PI_DIRN comp[C_B*4],zb.pb,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TN top,b
		TRAPEZOID_PIZ2TN bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2TN>,<%($-_setup_start)>
RawTriangle_PIZ2TN endp


TriangleRenderPIZ2TIB proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2TIB

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2TIB endp

; Trapezoid loop for linear texture mapped z buffered faces with blending
; Note: only *reads* z buffer
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2TIB	macro	half,dirn
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:	push	esi
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
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels
pixel_loop:
	ifidni <dirn>,<b>
		mov	ebp,temp_i
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	ebp,zb.pi.grad_x
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_i,ebp
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edx,zb.pz.grad_x ; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

	;; Depth test
	;;
		mov	eax,edx
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		mov	ebp,zb.texture_buffer
		mov	bl,TSEG [ebp+ebx]

		if	0
		mov	DSEG [edi],ax	;	*zptr = z
		endif

		mov	bh,byte ptr temp_i+2
		mov	eax,zb.shade_table
		mov	bl,[eax+ebx]
		mov	bh, CSEG [esi]
		mov	eax,zb.blend_table
		mov	bl,[eax+ebx]
		mov	CSEG [esi],bl	;	*ptr = colour

pixel_behind:

	ifidni <dirn>,<f>
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

		add	edx,zb.pz.grad_x ; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop
no_pixels:
	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
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

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ2TIB
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated shade index
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2TIB proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_I*4],zb.pi,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TIB top,f
		TRAPEZOID_PIZ2TIB bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_I*4],zb.pi,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TIB top,b
		TRAPEZOID_PIZ2TIB bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2TIB>,<%($-_setup_start)>
RawTriangle_PIZ2TIB endp

ditherspec	macro	i,a,b,c,d
dither&i&0	equ	a*4096
dither&i&1	equ	b*4096
dither&i&2	equ	c*4096
dither&i&3	equ	d*4096
		endm

@dith	macro	x,y
	exitm	%dither&x&&y&
	endm

	; Specify dither pattern

		ditherspec	0,13,1,4,0
		ditherspec	1,8,12,9,5
		ditherspec	2,10,6,7,11
		ditherspec	3,3,15,14,2

TriangleRenderPIZ2TD proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2TD

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2TD endp

; Trapezoid loop for linear dithered texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2TD	macro	half,dirn
		local scan_loop,pixel_loop,no_pixels
		local done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

		ifidni	<half>,<top>
		mov	eax,vertex_0[_Y]
		else
		mov	eax,vertex_1[_Y]
		endif
		and	eax,3

		; Indexed jump into correct row modulo 4

		jmp	rowtable_PIZ2TD&half&&dirn&[eax*4]

		.data
rowtable_PIZ2TD&half&&dirn&	label	ptr dword
		dd	scan_loop&half&&dirn&0
		dd	scan_loop&half&&dirn&1
		dd	scan_loop&half&&dirn&2
		dd	scan_loop&half&&dirn&3

		.code

scan_loop:
		for	row,<0,1,2,3>
		local	pixel_loop,start_carried,no_pixels
		local	next_row

scan_loop&half&&dirn&&row&:

	; Prepare for next iteration of loop
	;
		push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; ecx:	address of end of scanline
	; edx:	z 	(16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels

		; Indexed jump into correct column modulo 4

		mov	eax,esi
		and	eax,3
		jmp	columntable_PIZ2TD&row&&half&&dirn&[eax*4]
		.data
columntable_PIZ2TD&row&&half&&dirn&	label	ptr dword
		ifidni	<dirn>,<f>
		dd	pixel_start&half&&dirn&&row&0
		dd	pixel_start&half&&dirn&&row&1
		dd	pixel_start&half&&dirn&&row&2
		dd	pixel_start&half&&dirn&&row&3
		else
		dd	pixel_start&half&&dirn&&row&0
		dd	pixel_start&half&&dirn&&row&3
		dd	pixel_start&half&&dirn&&row&2
		dd	pixel_start&half&&dirn&&row&1
		endif
		.code

		for	col,<0,1,2,3>
pixel_start&half&&dirn&&row&&col&:
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		ifidni	<dirn>,<f>
		add	eax,@dith(row,col)
		add	ebx,@dith(row,col)
		else
		add	eax,@dith(row,%(4-col and 3))
		add	ebx,@dith(row,%(4-col and 3))
		endif
		mov	temp_u,eax
		mov	temp_v,ebx

		jmp	pixel_loop&half&&dirn&&row&&col&
		endm

pixel_loop:
		for	col,<0,1,2,3>
		local	pixel_behind

pixel_loop&half&&dirn&&row&&col&:

	ifidni <dirn>,<b>
	;; Per pixel update - backwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		add	eax,@dith(row,%(3-col and 3))-@dith(row,%(4-col and 3))
		add	ebx,@dith(row,%(3-col and 3))-@dith(row,%(4-col and 3))
		mov	temp_u,eax
		mov	temp_v,ebx

		sub	edx,ebp		; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

	;; Depth test
	;;
		mov	eax,edx
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		add	ebx,zb.texture_buffer
		mov	bl,TSEG [ebx]

		and	bl,bl		; Check for transparency
		je	pixel_behind

		mov	DSEG [edi],ax	;	*zptr = z
		mov	CSEG [esi],bl	;	*ptr = colour

pixel_behind:

	ifidni <dirn>,<f>
	;; Dithered increments for u and v
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		add	eax,@dith(row,%(col+1 and 3))-@dith(row,col)
		add	ebx,@dith(row,%(col+1 and 3))-@dith(row,col)
		mov	temp_u,eax
		mov	temp_v,ebx

		add	edx,ebp		; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jae	no_pixels

		endm	; col,<0,1,2,3>
		jmp	pixel_loop

no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax  		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		je	done_trapezoid
		jmp	next_row

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		dec	zb.half.count
		je	done_trapezoid
next_row:
		endm	; row,<1,2,3,4>

		jmp	scan_loop

done_trapezoid:
		endm


; RawTriangle_PIZ2TD
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2TD proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2TD top,f
		TRAPEZOID_PIZ2TD bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

		mov	eax,vertex_0[_Y]
		mov	temp_y,eax

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2TD top,b
		TRAPEZOID_PIZ2TD bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2TD>,<%($-_setup_start)>
RawTriangle_PIZ2TD endp

	; Lighting dither may be out of phase with (u,v) dithering
	; ui_phase_x and ui_phase_y specify the phase difference
	; in the x and y directions

ui_phase_x	equ	2
ui_phase_y	equ	2

TriangleRenderPIZ2TID proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2TID

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2TID endp

; Trapezoid loop for lit linear dithered-u,v texture mapped z buffered faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2TID	macro	half,dirn
		local scan_loop,pixel_loop,no_pixels
		local done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

		ifidni	<half>,<top>
		mov	eax,vertex_0[_Y]
		else
		mov	eax,vertex_1[_Y]
		endif
		and	eax,3

		; Indexed jump into correct row modulo 4

		jmp	rowtable_PIZ2TID&half&&dirn&[eax*4]

		.data
rowtable_PIZ2TID&half&&dirn&	label	ptr dword
		dd	scan_loop&half&&dirn&0
		dd	scan_loop&half&&dirn&1
		dd	scan_loop&half&&dirn&2
		dd	scan_loop&half&&dirn&3

		.code

scan_loop:
		for	row,<0,1,2,3>
		local	pixel_loop,start_carried,no_pixels
		local	next_row

scan_loop&half&&dirn&&row&:

	; Prepare for next iteration of loop
	;
		push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; ecx:	address of end of scanline
	; edx:	z 	(16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels

		; Indexed jump into correct column modulo 4

		mov	eax,esi
		and	eax,3
		jmp	columntable_PIZ2TID&row&&half&&dirn&[eax*4]

		.data
columntable_PIZ2TID&row&&half&&dirn&	label	ptr dword
		ifidni	<dirn>,<f>
		dd	pixel_start&half&&dirn&&row&0
		dd	pixel_start&half&&dirn&&row&1
		dd	pixel_start&half&&dirn&&row&2
		dd	pixel_start&half&&dirn&&row&3
		else
		dd	pixel_start&half&&dirn&&row&0
		dd	pixel_start&half&&dirn&&row&3
		dd	pixel_start&half&&dirn&&row&2
		dd	pixel_start&half&&dirn&&row&1
		endif
		.code

		for	col,<0,1,2,3>
pixel_start&half&&dirn&&row&&col&:

		;; Dithered increments for u, v and index

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	ebp,zb.pi.current

		ifidni	<dirn>,<f>
		add	eax,@dith(row,col)
		add	ebx,@dith(row,col)
		add	ebp,@dith(%(ui_phase_y+row and 3),%(ui_phase_x+col and 3))
		else
		add	eax,@dith(row,%(4-col and 3))
		add	ebx,@dith(row,%(4-col and 3))
		add	ebp,@dith(%(ui_phase_y+row and 3),%(ui_phase_x+4-col and 3))
		endif
		mov	temp_u,eax
		mov	temp_v,ebx
		mov	temp_i,ebp

		jmp	pixel_loop&half&&dirn&&row&&col&
		endm

pixel_loop:
		for	col,<0,1,2,3>
		local	pixel_behind

pixel_loop&half&&dirn&&row&&col&:

	ifidni <dirn>,<b>
	;; Dithered increments for u, v and index
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		add	eax,@dith(row,%(3-col and 3))-@dith(row,%(4-col and 3))
		add	ebx,@dith(row,%(3-col and 3))-@dith(row,%(4-col and 3))
		mov	temp_u,eax
		mov	temp_v,ebx

		mov	ebp,temp_i
		sub	ebp,zb.pi.grad_x
		add	ebp,@dith(%(ui_phase_y+row and 3),%(ui_phase_x+3-col and 3))-@dith(%(ui_phase_y+row and 3),%(ui_phase_x+4-col and 3))
		mov	temp_i,ebp

		sub	edx,zb.pz.grad_x		; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

	;; Depth test
	;;
		mov	eax,edx
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		mov	ebp,zb.texture_buffer
		mov	bl,TSEG [ebp+ebx]

		and	bl,bl		; Check for transparency
		je	pixel_behind

		mov	bh,byte ptr temp_i+2
		add	ebx,zb.shade_table
		mov	bl,[ebx]
		mov	DSEG [edi],ax	;	*zptr = z
		mov	CSEG [esi],bl	;	*ptr = colour

pixel_behind:

	ifidni <dirn>,<f>
	;; Per pixel update - forwards
	;;
		mov	eax,temp_u
		mov	ebx,temp_v
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		add	eax,@dith(row,%(col+1 and 3))-@dith(row,col)
		add	ebx,@dith(row,%(col+1 and 3))-@dith(row,col)
		mov	temp_u,eax
		mov	temp_v,ebx

		mov	ebp,temp_i
		add	ebp,zb.pi.grad_x
		add	ebp,@dith(%(ui_phase_y+row and 3),%(ui_phase_x+col+1 and 3))-@dith(%(ui_phase_y+row and 3),%(ui_phase_x+col and 3))
		mov	temp_i,ebp

		add	edx,zb.pz.grad_x		; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jae	no_pixels

		endm	; col,<0,1,2,3>
		jmp	pixel_loop

no_pixels:

	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f

		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax  		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx
		mov	ebp,zb.pi.current
		add	ebp,zb.pi.d_nocarry
		mov	zb.pi.current,ebp

		dec	zb.half.count
		je	done_trapezoid
		jmp	next_row

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx
		mov	ebp,zb.pi.current
		add	ebp,zb.pi.d_carry
		mov	zb.pi.current,ebp

		dec	zb.half.count
		je	done_trapezoid
next_row:
		endm	; row,<1,2,3,4>

		jmp	scan_loop

done_trapezoid:
		endm


; RawTriangle_PIZ2TID
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 
RawTriangle_PIZ2TID proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f
		PARAM_PI_DIRN comp[C_I*4],zb.pi,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2TID top,f
		TRAPEZOID_PIZ2TID bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b
		PARAM_PI_DIRN comp[C_I*4],zb.pi,b

		mov	eax,vertex_0[_Y]
		mov	temp_y,eax

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current
		mov	ebp,zb.pz.grad_x

		TRAPEZOID_PIZ2TID top,b
		TRAPEZOID_PIZ2TID bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2TID>,<%($-_setup_start)>
RawTriangle_PIZ2TID endp

TriangleRenderPIZ2TIC proc uses eax ebx ecx edx esi edi,
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
		SETUP_SEGS

		mov	esi,pvertex_0
		mov	edi,pvertex_1
		mov	ebp,pvertex_2
		call	RawTriangle_PIZ2TIC

		RESTORE_SEGS
		pop	ebp
		ret
TriangleRenderPIZ2TIC endp

; Trapezoid loop for linear texture mapped z buffered RGB lit faces
;
; Arguments control:
;	whether loop uses 'top' or 'bottom' variables
;	which direction scanline are rendered from the long edge
;
TRAPEZOID_PIZ2TIC	macro	half,dirn
   	local scan_loop,pixel_loop,pixel_behind,no_pixels
	local start_carried,done_trapezoid

		cmp	zb.half.count,0
		je	done_trapezoid

		mov	ecx,zb.half.i

	; Prepare for next iteration of loop
	;
scan_loop:	push	esi
		push	edx

		mov	edi,zb.depth_buffer
		lea	edi,[edi+esi*2]
		add	esi,dword ptr zb.colour_buffer

		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		mov	ebp,zb.pr.current
		mov	temp_u,eax
		mov	temp_v,ebx
		mov	temp_r,ebp

		mov	eax,zb.pg.current
		mov	ebx,zb.pb.current
		mov	temp_g,eax
		mov	temp_b,ebx

	; Render same colour pixels along scanline (forwards or backwards)
	;
	; ecx:	address of end of scanline
	; edx:	z (16.16)
	; ebp:	z delta (16.16)
	; esi:	frame buffer ptr
	; edi:	z buffer ptr
	;
		CMP_&dirn esi,ecx
		jae	no_pixels
pixel_loop:
	ifidni <dirn>,<b>
		mov	ebp,temp_r
		mov	eax,temp_u
		mov	ebx,temp_v
		sub	ebp,zb.pr.grad_x
		sub	eax,zb.pu.grad_x
		sub	ebx,zb.pv.grad_x
		mov	temp_r,ebp
		mov	temp_u,eax
		mov	temp_v,ebx

		mov	ebp,temp_g
		mov	eax,temp_b
		sub	ebp,zb.pg.grad_x
		sub	eax,zb.pb.grad_x
		mov	temp_g,ebp
		mov	temp_b,eax

		sub	edx,zb.pz.grad_x ; z += d_z_z
		sub	edi,2		; z_ptr++
		dec	esi		; ptr++
	endif

	;; Depth test
	;;
		mov	eax,edx
		shr	eax,16
		cmp	ax,DSEG [edi]	; if (z > *zptr)
		ja	short pixel_behind

	;; Per pixel fetch colour
	;;
		mov	DSEG [edi],ax	;	*zptr = z
		xor	ebx,ebx
		mov	bl,byte ptr temp_u+2
		mov	bh,byte ptr temp_v+2
		mov	ebp,zb.texture_buffer
		mov	bl,TSEG [ebp+ebx]	; Get texel

	;; Light texel

		mov	ebp,zb.shade_table

	;; Light red component

		mov	bh,byte ptr temp_r+2
		mov	bl,2[ebp+4*ebx]

	;; Light green component

		mov	bh,byte ptr temp_g+2
		mov	bl,1[ebp+4*ebx]

	;; Light blue component

		mov	bh,byte ptr temp_b+2
		mov	bl,[ebp+4*ebx]

		mov	CSEG [esi],bl	;	*ptr = colour

pixel_behind:

	ifidni <dirn>,<f>
	;; Per pixel update - forwards
	;;
		mov	ebp,temp_r
		mov	eax,temp_u
		mov	ebx,temp_v
		add	ebp,zb.pr.grad_x
		add	eax,zb.pu.grad_x
		add	ebx,zb.pv.grad_x
		mov	temp_r,ebp
		mov	temp_u,eax
		mov	temp_v,ebx

		mov	ebp,temp_g
		mov	eax,temp_b
		add	ebp,zb.pg.grad_x
		add	eax,zb.pb.grad_x
		mov	temp_g,ebp
		mov	temp_b,eax

		add	edx,zb.pz.grad_x ; z += d_z_z
		add	edi,2		; z_ptr++
		inc	esi		; ptr++
	endif

		CMP_&dirn esi,ecx
		jb	pixel_loop
no_pixels:
	; Updates for next scanline:
	;
		pop	edx
		pop	esi
		mov	edi,zb.half.d_f
		mov	eax,zb.main.d_f
		add	zb.half.f,edi	    	; x_end += zb.main.grad_end (fraction)
		adc	ecx,zb.half.d_i 	; endptr += integer(dx_end)+CARRY

		add	zb.main.f,eax		; x+= zb.main.grad (fraction)
		jc	start_carried

		add	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_nocarry

	;; Per scanline parameter update - no carry
	;;
		mov	ebp,zb.pr.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	ebp,zb.pr.d_nocarry
		add	eax,zb.pu.d_nocarry
		add	ebx,zb.pv.d_nocarry
		mov	zb.pr.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		mov	ebp,zb.pg.current
		mov	eax,zb.pb.current
		add	ebp,zb.pg.d_nocarry
		add	eax,zb.pb.d_nocarry
		mov	zb.pg.current,ebp
		mov	zb.pb.current,eax

		dec	zb.half.count
		jne	scan_loop
		jmp	done_trapezoid

start_carried:	adc	esi,zb.main.d_i		; fb_offset+=delta+CARRY
		add	edx,zb.pz.d_carry

	;; Per scanline parameter update - carry
	;;
		mov	ebp,zb.pr.current
		mov	eax,zb.pu.current
		mov	ebx,zb.pv.current
		add	ebp,zb.pr.d_carry
		add	eax,zb.pu.d_carry
		add	ebx,zb.pv.d_carry
		mov	zb.pr.current,ebp
		mov	zb.pu.current,eax
		mov	zb.pv.current,ebx

		mov	ebp,zb.pg.current
		mov	eax,zb.pb.current
		add	ebp,zb.pg.d_carry
		add	eax,zb.pb.d_carry
		mov	zb.pg.current,ebp
		mov	zb.pb.current,eax

		dec	zb.half.count
		jne	scan_loop
done_trapezoid:
		endm


; RawTriangle_PIZ2TIC
;
; Render a triangle into frame buffer
;
;	Linear interpolated colour texture
;	Linear interpolated shade r g and b for coloured lights
;	Linear interpolated Z value
;	Integer vertices
;	Perfect point sampling
;
; esi = vertex 0 pointer
; edi = vertex 1 pointer
; ebp = vertex 2 pointer
; 

RawTriangle_PIZ2TIC proc

		SETUP_PI
		jl	reversed
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,f
		PARAM_PI_DIRN comp[C_R*4],zb.pr,f
		PARAM_PI_DIRN comp[C_G*4],zb.pg,f
		PARAM_PI_DIRN comp[C_B*4],zb.pb,f
		PARAM_PI_DIRN comp[C_U*4],zb.pu,f
		PARAM_PI_DIRN comp[C_V*4],zb.pv,f

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TIC top,f
		TRAPEZOID_PIZ2TIC bot,f
quit:		ret

	; Scan backwards
	;
reversed:
		neg	eax
		mov	g_divisor,eax

		PARAM_PI_DIRN v[Z*4],zb.pz,b
		PARAM_PI_DIRN comp[C_R*4],zb.pr,b
		PARAM_PI_DIRN comp[C_G*4],zb.pg,b
		PARAM_PI_DIRN comp[C_B*4],zb.pb,b
		PARAM_PI_DIRN comp[C_U*4],zb.pu,b
		PARAM_PI_DIRN comp[C_V*4],zb.pv,b

	; Load up registers for first time around inner loop
	;
		mov	esi,zb.main.i
		mov	edx,zb.pz.current

		TRAPEZOID_PIZ2TIC top,b
		TRAPEZOID_PIZ2TIC bot,b
		ret

	SHOW_CODESIZE	<RawTriangle_PIZ2TIC>,<%($-_setup_start)>
RawTriangle_PIZ2TIC endp

endif	; Hawkeye
	end
