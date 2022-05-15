;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: mesh386.asm 1.31 1995/08/31 16:47:35 sam Exp $
;; $Locker:  $
;;
;; Downcoded support for ZbRenderMesh
;;
	.386p
	.model flat,c

	include	zb.inc
	include 586_macs.inc

TEMP_VERTEX_SHIFT	equ 6
VERTEX_SHIFT		equ 5

TEMP_VERTEX_SIZE	equ 1 shl TEMP_VERTEX_SHIFT
VERTEX_SIZE 		equ 1 shl VERTEX_SHIFT

SHOW		macro x,y
		%out x = y
		endm

	if BASED_FIXED	; Only provide these when library is fixed point

	if (sizeof temp_vertex_fixed) NE TEMP_VERTEX_SIZE
	.err <sizeof(temp_vertex_fixed) is assumed to be 2^TEMP_VERTEX_SHIFT bytes>
	SHOW	<Size of temp_vertex_fixed     >,<%(sizeof temp_vertex_fixed)>
	endif

if 0
	if (sizeof br_vertex) NE VERTEX_SIZE
	.err <sizeof(br_vertex) is assumed to be 2^VERTEX_SHIFT bytes>
	SHOW	<Size of br_vertex     >,<%(sizeof br_vertex)>
	endif
endif

if 0
	SHOW	<sizeof temp_vertex_fixed  >,<%(sizeof temp_vertex_fixed)>
	SHOW	<temp_vertex_fixed.v       >,<%(temp_vertex_fixed.v)>
	SHOW	<temp_vertex_fixed.outcode >,<%(temp_vertex_fixed.outcode)>
	SHOW	<temp_vertex_fixed.comp    >,<%(temp_vertex_fixed.comp)>

	SHOW	<sizeof temp_face    >,<%(sizeof temp_face)>
	SHOW	<temp_face.surface   >,<%(temp_face.surface)>
	SHOW	<temp_face.flag      >,<%(temp_face.flag)>
	SHOW	<temp_face.codes     >,<%(temp_face.codes)>

	SHOW	<sizeof br_face      >,<%(sizeof br_face)>
	SHOW	<br_face.vertices    >,<%(br_face.vertices)>
	SHOW	<br_face.edges       >,<%(br_face.edges)>
	SHOW	<br_face.material    >,<%(br_face.material)>
	SHOW	<br_face.smoothing   >,<%(br_face.smoothing)>
	SHOW	<br_face.flags       >,<%(br_face.flags)>
	SHOW	<br_face.n.v[0]      >,<%(br_face.n.v[0])>
	SHOW	<br_face.n.v[2]      >,<%(br_face.n.v[2])>
	SHOW	<br_face.n.v[4]      >,<%(br_face.n.v[4])>
	SHOW	<br_face.d           >,<%(br_face.d)>


	SHOW	<br_zbuffer_state.vertex_counts    >,<%(br_zbuffer_state.vertex_counts)>
endif

	.data
		db '$Id: mesh386.asm 1.31 1995/08/31 16:47:35 sam Exp $',0
	align 4

loop_count	dd	?
group_faces	dd	?
group_count	dd	?
group_material	dd	?
screen_xfm	dd	1,2
foo		dd	3,4
	.code

; Process one on-screen group of culled faces
;
; br_uint_32 BR_ASM_CALL ZbOSFFVGroupCulled_A(br_face *fp, struct temp_face *tfp,int count)
;
ZbOSFFVGroupCulled_A proc uses ebx esi edi,
	fp : ptr word,  
	tfp : ptr word,  
	count : dword

		push	ebp

		mov	eax,count
		mov	loop_count,eax
		mov	group_count,0

		mov	esi,fp
		mov	ebp,tfp
		
	; Loop for each face in group
	;
	; edx:eax	Multiply result
	; ecx:ebx	Multiply accumulator
	; esi 		Current br_face pointer
	; ebp 		Current temp_face pointer
	; edi		zb.vertex_counts
	;

		mov	edi,zb.vertex_counts

face_loop:

	; Dot the face normal with the eye position
	;		
		movsx	eax,[esi].br_face.n.v[0]

		imul	fw.eye_m.v[0]
		mov	ebx,eax
		mov	ecx,edx

		movsx	eax,[esi].br_face.n.v[2]

		imul	fw.eye_m.v[4]
		add	ebx,eax
		adc	ecx,edx

		movsx	eax,[esi].br_face.n.v[4]

		imul	fw.eye_m.v[8]
		add	eax,ebx
		adc	edx,ecx

		FIX_FMUL

	; If n.eye > face->d then face is visible
	;
		cmp	eax,[esi].br_face.d
		jle	not_visible

	; Mark vertices as visible
	;

		xor	eax,eax
		xor	ebx,ebx
		xor	ecx,ecx

	; Get vertex 0,1,2
	;
		mov	ax,[esi].br_face.vertices[0]
		mov	bx,[esi].br_face.vertices[2]
		mov	cx,[esi].br_face.vertices[4]

	; Mark face as visible
	;
		mov	[ebp].temp_face.flag, TFF_VISIBLE ; temp_face flag

	; Mark vertices as referenced
	;
		mov	byte ptr [edi+eax],1
		mov	byte ptr [edi+ebx],1
		mov	byte ptr [edi+ecx],1

	; Next vertex
	;
		mov	group_count,1

		add	esi,sizeof br_face
		add	ebp,sizeof temp_face

		dec	loop_count
		jne	face_loop

		mov	eax,group_count
		pop	ebp
		ret

	; Next face
	;
not_visible:	mov	byte ptr [ebp].temp_face.flag,0		; temp_face flag
		add	esi,sizeof br_face
		add	ebp,sizeof temp_face

		dec	loop_count
		jne	face_loop

		mov	eax,group_count
		pop	ebp
		ret
ZbOSFFVGroupCulled_A endp

; Process one on-screen group of culled, lit, faces
;
; br_uint_32 BR_ASM_CALL ZbOSFFVGroupCulledLit_A(br_face *fp, struct temp_face *tfp,int count)
;
ZbOSFFVGroupCulledLit_A proc uses ebx esi edi,
	fp : ptr word,  
	tfp : ptr word,  
	count : dword

		push	ebp

		mov	eax,count
		mov	loop_count,eax
		mov	group_count,0

		mov	esi,fp
		mov	ebp,tfp
		
	; Loop for each face in group
	;
	; edx:eax	Multiply result
	; ecx:ebx	Multiply accumulator
	; esi 		Current br_face pointer
	; ebp 		Current temp_face pointer
	; edi		zb.vertex_counts
	;

		mov	edi,zb.vertex_counts

face_loop:

	; Dot the face normal with the eye position
	;		
		movsx	eax,[esi].br_face.n.v[0]

		imul	fw.eye_m.v[0]
		mov	ebx,eax
		mov	ecx,edx

		movsx	eax,[esi].br_face.n.v[2]

		imul	fw.eye_m.v[4]
		add	ebx,eax
		adc	ecx,edx

		movsx	eax,[esi].br_face.n.v[4]

		imul	fw.eye_m.v[8]
		add	eax,ebx
		adc	edx,ecx

		FIX_FMUL

	; If n.eye > face->d then face is visible
	;
		cmp	eax,[esi].br_face.d
		jle	not_visible

	; Mark vertices as visible
	;

		xor	eax,eax
		xor	ebx,ebx
		xor	ecx,ecx

	; Get vertex 0,1,2
	;
		mov	ax,[esi].br_face.vertices[0]
		mov	bx,[esi].br_face.vertices[2]
		mov	cx,[esi].br_face.vertices[4]

	; Mark face as visible
	;
		mov	[ebp].temp_face.flag,TFF_VISIBLE ; temp_face flag

	; Mark vertices as referenced
	;
		mov	edi,zb.vertex_counts
		mov	byte ptr [edi+eax],1
		mov	byte ptr [edi+ebx],1
		mov	byte ptr [edi+ecx],1

		mov	group_count,1

	; Call lighting function
	;
	ifdef __WATCOMCX__
	; watcom register calling
	;
		shl	eax,VERTEX_SHIFT
		mov	ebx,zb.model
		add	eax,[ebx].br_model.prepared_vertices
		mov	edx,esi
		
		call	fw.face_surface_fn

	elseifdef __GNUC__

	; GNU-C - wdisasm fails to dissasemble "call dword ptr funcptr" right with -au
	;
		shl	eax,VERTEX_SHIFT
		mov	ebx,zb.model
		add	eax,[ebx].br_model.prepared_vertices
		xor	ebx,ebx
		push	ebx
		push	esi
		push	eax

		mov	eax,fw.face_surface_fn
		call	eax

		add	esp,12
	else

	; __cdecl calling 
	;
		shl	eax,VERTEX_SHIFT
		mov	ebx,zb.model
		add	eax,[ebx].br_model.prepared_vertices

		xor	ebx,ebx
		push	ebx
		push	esi
		push	eax

		call	dword ptr fw.face_surface_fn

		add	esp,12
	endif
		mov	[ebp].temp_face.surface,eax

	; Next face
	;
		add	esi,sizeof br_face
		add	ebp,sizeof temp_face

		dec	loop_count
		jne	face_loop

		mov	eax,group_count
		pop	ebp
		ret

	; Next face
	;
not_visible:	mov	byte ptr [ebp].temp_face.flag,0	; temp_face flag
		add	esi,sizeof br_face
		add	ebp,sizeof temp_face

		dec	loop_count
		jne	face_loop

		mov	eax,group_count
		pop	ebp
		ret
ZbOSFFVGroupCulledLit_A endp

; Premultiply viewport into model->screen matrix 
;
; Return !0 if there was an overflow
;
; br_uint_32 BR_ASM_CALL ZbOSCopyModelToScreen_A(void);
;
ZbOSCopyModelToScreen_A proc uses ebx esi edi

	; zb.os_model_to_scre0..3][0] = fw.model_to_screen[0.[0..3][0] * fw.vp_width
	; zb.os_model_to_screen[0..3][1] = fw.model_to_screen[0..3][1] * fw.vp_height
	; zb.os_model_to_screen[0..3][2] = fw.model_to_screen[0..3][2] * -1
	;
	; Keep a 64 bit result for row 3 - the translations and perspective
	; that will not be multiplied (incoming points are x,y,z,1)
	;

	; Loop for rows 0-2
	;
		mov	ecx,3
		xor	esi,esi

row_loop:
		; X
		mov	eax, fw.model_to_screen.m[esi+0]
		imul	fw.vp_width

		mov	ebx,edx
		and	ebx,0ffff8000h
		je	no_x_overflow
		cmp	ebx,0ffff8000h
		jne	overflowed

no_x_overflow:	shr	eax,16
		shl	edx,16
		or	eax,edx

		mov	zb.os_model_to_screen.m[esi+0],eax

		; Y
		mov	eax,fw.model_to_screen.m[esi+4]
		imul	fw.vp_height

		mov	ebx,edx
		and	ebx,0ffff8000h
		je	no_y_overflow
		cmp	ebx,0ffff8000h
		jne	overflowed

no_y_overflow:	shr	eax,16
		shl	edx,16
		or	eax,edx

		mov	zb.os_model_to_screen.m[esi+4],eax

		; Z
		mov	eax,fw.model_to_screen.m[esi+8]
		neg	eax
		mov	zb.os_model_to_screen.m[esi+8],eax

		; W
		mov	eax,fw.model_to_screen.m[esi+12]
		mov	zb.os_model_to_screen.m[esi+12],eax

		add	esi,16
		dec	ecx
		jne	row_loop

	; row 3
	;
		; X
		mov	eax, fw.model_to_screen.m[48+0]
		imul	fw.vp_width
		mov	zb.os_model_to_screen.m[48+0],eax
		mov	zb.os_model_to_screen_hi[0],edx

		; Y
		mov	eax,fw.model_to_screen.m[48+4]
		imul	fw.vp_height
		mov	zb.os_model_to_screen.m[48+4],eax
		mov	zb.os_model_to_screen_hi[4],edx

		; Z
		mov	eax,fw.model_to_screen.m[48+8]
		neg	eax
		
		mov	edx,eax
		sar	edx,16
		shl	eax,16

		mov	zb.os_model_to_screen.m[48+8],eax
		mov	zb.os_model_to_screen_hi[8],edx

		; W
		mov	eax,fw.model_to_screen.m[48+12]
		mov	zb.os_model_to_screen.m[48+12],eax

		xor	eax,eax
		ret
overflowed:
		mov	eax,-1
		ret
ZbOSCopyModelToScreen_A endp

VERTEX_TRANSFORM_PROJECT macro

		; W
		;
		mov	eax,zb.os_model_to_screen.m[0+12]
		imul	[esi].br_vertex.p.v[0]
		mov	ebp,eax
		mov	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+12]
		imul	[esi].br_vertex.p.v[4]
		add	ebp,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+12]
		imul	[esi].br_vertex.p.v[8]
		add	ebp,eax
		adc	ecx,edx

		shr	ebp,16
		shl	ecx,16
		or	ebp,ecx

		add	ebp,zb.os_model_to_screen.m[48+12]

		; X
		;
		mov	ebx,zb.os_model_to_screen.m[48+0]
		mov	ecx,zb.os_model_to_screen_hi[0]

		mov	eax,zb.os_model_to_screen.m[0+0]
		imul	[esi].br_vertex.p.v[0]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+0]
		imul	[esi].br_vertex.p.v[4]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+0]
		imul	[esi].br_vertex.p.v[8]
		add	eax,ebx
		adc	edx,ecx

		idiv	ebp
		add	eax,fw.vp_ox
		mov	[edi].temp_vertex_fixed.v[0],eax

		; Y
		;
		mov	ebx,zb.os_model_to_screen.m[48+4]
		mov	ecx,zb.os_model_to_screen_hi[4]

		mov	eax,zb.os_model_to_screen.m[0+4]
		imul	[esi].br_vertex.p.v[0]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+4]
		imul	[esi].br_vertex.p.v[4]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+4]
		imul	[esi].br_vertex.p.v[8]
		add	eax,ebx
		adc	edx,ecx

		idiv	ebp
		add	eax,fw.vp_oy
		mov	[edi].temp_vertex_fixed.v[4],eax

		; Z
		;
		mov	ebx,zb.os_model_to_screen.m[48+8]
		mov	ecx,zb.os_model_to_screen_hi[8]

		mov	eax,zb.os_model_to_screen.m[0+8]
		imul	[esi].br_vertex.p.v[0]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+8]
		imul	[esi].br_vertex.p.v[4]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+8]
		imul	[esi].br_vertex.p.v[8]
		add	eax,ebx
		adc	edx,ecx

		shld	edx,eax,15
		shl	eax,15

		mov	[edi].temp_vertex_fixed.comp[C_W*4],ebp		; Set comp[C_W]

		div	ebp

		mov	[edi].temp_vertex_fixed.v[8],eax
endm

BOUND_CHECK	macro	min,max
		local not_min,not_max

		cmp	eax,min
		jge	not_min
		mov	min,eax
not_min:

		cmp	eax,max
		jle	not_max
		mov	max,eax
not_max:
		endm


VERTEX_TRANSFORM_PROJECT_CHECK	macro

		; W
		;
		mov	eax,zb.os_model_to_screen.m[0+12]
		imul	[esi].br_vertex.p.v[0]
		mov	ebp,eax
		mov	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+12]
		imul	[esi].br_vertex.p.v[4]
		add	ebp,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+12]
		imul	[esi].br_vertex.p.v[8]
		add	ebp,eax
		adc	ecx,edx

		shr	ebp,16
		shl	ecx,16
		or	ebp,ecx

		add	ebp,zb.os_model_to_screen.m[48+12]
		

		; X
		;
		mov	ebx,zb.os_model_to_screen.m[48+0]
		mov	ecx,zb.os_model_to_screen_hi[0]

		mov	eax,zb.os_model_to_screen.m[0+0]
		imul	[esi].br_vertex.p.v[0]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+0]
		imul	[esi].br_vertex.p.v[4]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+0]
		imul	[esi].br_vertex.p.v[8]
		add	eax,ebx
		adc	edx,ecx

		idiv	ebp
		add	eax,fw.vp_ox
		mov	[edi].temp_vertex_fixed.v[0],eax

		BOUND_CHECK zb.bounds[0],zb.bounds[8]

		; Y
		;
		mov	ebx,zb.os_model_to_screen.m[48+4]
		mov	ecx,zb.os_model_to_screen_hi[4]

		mov	eax,zb.os_model_to_screen.m[0+4]
		imul	[esi].br_vertex.p.v[0]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+4]
		imul	[esi].br_vertex.p.v[4]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+4]
		imul	[esi].br_vertex.p.v[8]
		add	eax,ebx
		adc	edx,ecx

		idiv	ebp
		add	eax,fw.vp_oy
		mov	[edi].temp_vertex_fixed.v[4],eax

		BOUND_CHECK zb.bounds[4],zb.bounds[12]

		; Z
		;
		mov	ebx,zb.os_model_to_screen.m[48+8]
		mov	ecx,zb.os_model_to_screen_hi[8]

		mov	eax,zb.os_model_to_screen.m[0+8]
		imul	[esi].br_vertex.p.v[0]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[16+8]
		imul	[esi].br_vertex.p.v[4]
		add	ebx,eax
		adc	ecx,edx

		mov	eax,zb.os_model_to_screen.m[32+8]
		imul	[esi].br_vertex.p.v[8]
		add	eax,ebx
		adc	edx,ecx

		shld	edx,eax,15
		shl	eax,15

		mov	[edi].temp_vertex_fixed.comp[C_W*4],ebp		; Set comp[C_W]
		div	ebp

		mov	[edi].temp_vertex_fixed.v[8],eax

		endm

; Call lighting function
;
VERTEX_LIGHT	macro

	ifdef __WATCOMCX__
	; watcom register calling
	;
		mov	eax,esi
		lea	edx,[(32-6)+esi]
		lea	ebx,[edi].temp_vertex_fixed.comp
		call	fw.surface_fn

	elseifdef __GNUC__

	; GNU-C - wdisasm fails to dissasemble "call dword ptr funcptr" right with -au
	;
		lea	eax,[edi].temp_vertex_fixed.comp
		push	eax
		lea	eax,[(32-6)+esi]

		push	eax
		push	esi
		mov	eax,fw.surface_fn
		call	eax
		add	sp,12
	else

	; __cdecl calling 
	;
		lea	eax,[edi].temp_vertex_fixed.comp
		push	eax
		lea	eax,[(32-6)+esi]
		push	eax
		push	esi
		call	fw.surface_fn
		add	sp,12
	endif

endm


; Transform and projects a groups of vertices
;
; br_uint_32 BR_ASM_CALL ZbOSTVGroup_A(br_vertex *vp, struct temp_vertex_fixed *tvp,int count, br_uint_8 *countp);
;
ZbOSTVGroup_A proc uses ebx esi edi,
	vp : ptr word,  
	tvp : ptr word,  
	count : dword,
	countp : ptr byte

		push	ebp

		mov	eax,count
		mov	loop_count,eax

		mov	esi,vp
		mov	edi,tvp

		mov	ebp,countp
vertex_loop:
		cmp	byte ptr [ebp],0
		je	next_vertex

		push	ebp
		VERTEX_TRANSFORM_PROJECT
		pop	ebp
	; Next vertex
	;
next_vertex:
		add	esi,VERTEX_SIZE
		add	edi,TEMP_VERTEX_SIZE
		inc	ebp
		dec	loop_count
		jne	vertex_loop

		pop	ebp
		ret
ZbOSTVGroup_A endp

; Transform, projects and bounds checks a groups of vertices
;
; br_uint_32 BR_ASM_CALL ZbOSTVGroupBC_A(br_vertex *vp, struct temp_vertex_fixed *tvp,int count, br_uint_8 *countp);
;
ZbOSTVGroupBC_A proc uses ebx esi edi,
	vp : ptr word,  
	tvp : ptr word,  
	count : dword,
	countp : ptr byte

		push	ebp

		mov	eax,count
		mov	loop_count,eax

		mov	esi,vp
		mov	edi,tvp

		mov	ebp,countp
vertex_loop:
		cmp	byte ptr [ebp],0
		je	next_vertex

		push	ebp
		VERTEX_TRANSFORM_PROJECT_CHECK
		pop	ebp
	; Next vertex
	;
next_vertex:
		add	esi,VERTEX_SIZE
		add	edi,TEMP_VERTEX_SIZE
		inc	ebp
		dec	loop_count
		jne	vertex_loop

		pop	ebp
		ret
ZbOSTVGroupBC_A endp


; Transform, project and light a group of vertices
;
; br_uint_32 BR_ASM_CALL ZbOSTVGroupLit_A(br_vertex *vp, struct temp_vertex_fixed *tvp,int count, br_uint_8 *countp);
;

ZbOSTVGroupLit_A proc uses ebx esi edi,
	vp : ptr word,  
	tvp : ptr word,  
	count : dword,
	countp : ptr byte

		push	ebp

		mov	eax,count
		mov	loop_count,eax

		mov	esi,vp
		mov	edi,tvp

		mov	ebp,countp
vertex_loop:
		
		cmp	byte ptr [ebp],0
		je	next_vertex

		push	ebp
		VERTEX_TRANSFORM_PROJECT
		VERTEX_LIGHT
		pop	ebp

	; Next vertex
	;
next_vertex:
		add	esi,VERTEX_SIZE
		add	edi,TEMP_VERTEX_SIZE
		inc	ebp
		dec	loop_count
		jne	vertex_loop

		pop	ebp
		ret
ZbOSTVGroupLit_A endp

; Transform, project and light a group of vertices
;
; br_uint_32 BR_ASM_CALL ZbOSTVGroupLitBC_A(br_vertex *vp, struct temp_vertex_fixed *tvp,int count, br_uint_8 *countp);
;

ZbOSTVGroupLitBC_A proc uses ebx esi edi,
	vp : ptr word,  
	tvp : ptr word,  
	count : dword,
	countp : ptr byte

		push	ebp

		mov	eax,count
		mov	loop_count,eax

		mov	esi,vp
		mov	edi,tvp

		mov	ebp,countp
vertex_loop:
		cmp	byte ptr [ebp],0
		je	next_vertex

		push	ebp
		VERTEX_TRANSFORM_PROJECT_CHECK
		VERTEX_LIGHT
		pop	ebp

	; Next vertex
	;
next_vertex:
		add	esi,VERTEX_SIZE
		add	edi,TEMP_VERTEX_SIZE
		inc	ebp
		dec	loop_count
		jne	vertex_loop

		pop	ebp
		ret
ZbOSTVGroupLitBC_A endp

	endif	; BASED_FIXED

	end
