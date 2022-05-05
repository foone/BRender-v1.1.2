;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: pscan.asm 1.3 1995/02/22 21:53:38 sam Exp $
;; $Locker:  $
;;
;; Scanline loop for perpsective correct texture mapping
;;
		.386p
		.model	c,flat
		include	zb.inc
		.data

z		dd	?
i		dd	?
udW		dd	?
vdW		dd	?
Eu		dd	?
Ev		dd	?
zdest		dd	?
dest		dd	?
dest2		dd	?
source		dd	?
iW		dd	?
dz		dd	?

		align 4

		.code

ScanLinePIZ2TIP	macro	name,pre,incu,decu,incv,decv,post1,post2
name		proc

		pushad

	; Copy various parameters into registers

                mov     esi,tsl.Eu
                mov     ebx,tsl.Ev
                mov     eax,tsl.udW
                mov     edi,tsl.vdW

		mov	vdW,edi
                mov     ecx,zb.pi.current
                mov     edx,tsl.iW
		mov	Ev,ebx
		mov	iW,edx
                mov     i,ecx
		
                mov     ecx,zb.pz.current
                mov     ebp,tsl._end
                mov     z,ecx
		
                mov     ecx,tsl.zstart
                mov     udW,eax
                mov     zdest,ecx
		
                mov     edi,tsl.start
                mov     eax,tsl.source
		
                cmp     edi,ebp
		mov	ebp,zdest
                ja      rtol		; Jump if right to left	
		
	    ; Left to right scan
		
                mov     dest,edi
next_pixelb:
		jae     L50		; Jump if end of scan
		
                mov     source,eax	; Free up eax

	    ; z buffer test

		mov	dl,[ebp]
		mov	ebx,z		; Get new z value
		mov	dh,1[ebp]
		ror	ebx,16
                cmp     bx,dx
                ja      L34		; Jump if new pixel behind old
		
                mov     edx,zb.texture_buffer
		xor	ecx,ecx
		add	edx,eax
		mov	eax,zb.shade_table
                mov     [ebp],bx	; Store new z
                add     ebp,2		; zdest++
		mov	cl,[edx]
                mov     edx,zb.pz.grad_x	; Get z step
		ror	ebx,16
		mov	ch,byte ptr i+2
                add     ebx,edx		; z += dz
		test	cl,cl		; Transparent?
		jz	transp1
                add     ecx,eax		; Address of lit texel
                mov     cl,[ecx]	; Get lit texel
                mov     [edi],cl	; Store texel
transp1:
		mov	eax,zb.pi.grad_x
	        mov	edx,i		; Get lighting
		add	edx,eax		; i += di
		mov	i,edx
		
		jmp	L34a
L34:
		ror	ebx,16
                add     ebp,2
                mov     ecx,zb.pz.grad_x
                add     ebx,ecx
	        mov	ecx,i
		add	ecx,zb.pi.grad_x
		mov	i,ecx
L34a:
		inc	edi
		if	1
		cmp	edi,tsl._end
		jae	L50
		endif
	        mov	edx,iW
                mov     dest,edi
		mov	edi,vdW
                mov     z,ebx
                mov     zdest,ebp
                mov     ecx,tsl.dWx
                mov     ebp,udW
		mov	ebx,Ev
                mov     eax,source	; Restore eax

	; Core perspective calculations

		pre
                sub     esi,ebp
                add     edx,ecx
                cmp     esi,edx
                jge     incuf		; Jump if u not too large
		
L36:
		test    esi,esi
		jl	decuf
L37:
		sub     ebx,edi
;		test	ebx,ebx		; Redundant ?
		jl	decvf
                cmp     ebx,edx
                jge     incvf		; Jump if v not too small
L40:
		post1
		post2
		mov	iW,edx
		mov	Ev,ebx
		mov	edi,dest
                cmp     edi,tsl._end
		mov     ebp,zdest
                jmp     next_pixelb

	    ; Will be right to left
rtol:
                mov     dest,edi
next_pixelbr:
		jbe     L50		; Jump if end of scan
		
                mov     source,eax	; Free up eax

	    ; z buffer test

		mov	dl,[ebp]
		mov	ebx,z		; Get new z value
		mov	dh,1[ebp]
		ror	ebx,16
                cmp     bx,dx
                ja      R34		; Jump if new pixel behind old
		
                mov     edx,zb.texture_buffer
		xor	ecx,ecx
		add	edx,eax
		mov	eax,zb.shade_table
                mov     [ebp],bx	; Store new z
                sub     ebp,2		; zdest++
		mov	cl,[edx]
                mov     edx,zb.pz.grad_x	; Get z step
		ror	ebx,16
		mov	ch,byte ptr i+2
                sub     ebx,edx		; z += dz
		test	cl,cl		; Transparent?
		jz	transp1r
                add     ecx,eax		; Address of lit texel
                mov     cl,[ecx]	; Get lit texel
                mov     [edi],cl	; Store texel
transp1r:
		mov	eax,zb.pi.grad_x
	        mov	edx,i		; Get lighting
		sub	edx,eax		; i += di
		mov	i,edx
		
		jmp	R34a
R34:
		ror	ebx,16
                sub     ebp,2
                mov     ecx,zb.pz.grad_x
                sub     ebx,ecx
	        mov	ecx,i
		sub	ecx,zb.pi.grad_x
		mov	i,ecx
R34a:
		dec	edi
		if	1
		cmp	edi,tsl._end
		jbe	L50
		endif
	        mov	edx,iW
                mov     dest,edi
		mov	edi,vdW
                mov     z,ebx
                mov     zdest,ebp
                mov     ecx,tsl.dWx
                mov     ebp,udW
		mov	ebx,Ev
                mov     eax,source	; Restore eax

	; Core perspective calculations

		pre
                add     esi,ebp
                sub     edx,ecx
                cmp     esi,edx
                jge     incub		; Jump if u not too large
		
R36:
		test    esi,esi
		jl	decub
R37:
		add     ebx,edi
;		test	ebx,ebx		; Redundant ?
		jl	decvb
                cmp     ebx,edx
                jge     incvb		; Jump if v not too small
R40:
		post1
		post2
		mov	iW,edx
		mov	Ev,ebx
		mov	edi,dest
                cmp     edi,tsl._end
		mov     ebp,zdest
                jmp     next_pixelbr

L50:
		popad
		ret
		
decuf:
                mov     ebp,udW
decufloop:
                decu
                sub     ebp,tsl.dWx
                add     esi,edx
                jnc     decufloop
		
                mov     udW,ebp
		jmp	L37
		
incuf:
                mov     ecx,tsl.dWx
		mov     ebp,udW
                sub     esi,edx
		
incufloop:
                incu     		; Increment u
                add     ebp,ecx
                sub     esi,edx
                jge     incufloop
		add	esi,edx
		
                mov     udW,ebp
		jmp	L37

decvf:
                mov     ebp,tsl.dWx
decvfloop:       
                decv     		; Decrement v
                sub     edi,ebp
                add     ebx,edx
                jnc     decvfloop
		mov	vdW,edi
		jmp	L40
		
incvf:
                mov     ecx,tsl.dWx
                sub     ebx,edx
		
incvfloop:
		incv     		; Increment v
                add     edi,ecx
                sub     ebx,edx
                jge     incvfloop
		
		add	ebx,edx
		mov	vdW,edi
		jmp	L40

		; Back
decub:
                mov     ebp,udW
decubloop:
                decu
                sub     ebp,tsl.dWx
                add     esi,edx
                jnc     decubloop
		
                mov     udW,ebp
		jmp	R37
		
incub:
                mov     ecx,tsl.dWx
		mov     ebp,udW
                sub     esi,edx
		
incubloop:
                incu     		; Increment u
                add     ebp,ecx
                sub     esi,edx
                jge     incubloop
		add	esi,edx
		
                mov     udW,ebp
		jmp	R37

decvb:
                mov     ebp,tsl.dWx
decvbloop:       
                decv     		; Decrement v
                sub     edi,ebp
                add     ebx,edx
                jnc     decvbloop
		mov	vdW,edi
		jmp	R40
		
incvb:
                mov     ecx,tsl.dWx
                sub     ebx,edx
		
incvbloop:
		incv     		; Increment v
                add     edi,ecx
                sub     ebx,edx
                jge     incvbloop
		
		add	ebx,edx
		mov	vdW,edi
		jmp	R40
		
name		endp

		endm

ScanLinePIZ2TP	macro	name,pre,incu,decu,incv,decv,post1,post2
name		proc

		pushad

                mov     esi,tsl.Eu
                mov     ebx,tsl.Ev
                mov     eax,tsl.udW
                mov     edi,tsl.vdW
                mov     edx,tsl.iW
                mov     ecx,zb.pz.current
                mov     ebp,tsl._end
		
		mov	vdW,edi
		mov	iW,edx
		mov	Ev,ebx
                mov     z,ecx
                mov     udW,eax
		
                mov     ecx,tsl.zstart
                mov     zdest,ecx
		
                mov     edi,tsl.start
                mov     eax,tsl.source
		
                cmp     edi,ebp
		mov	ebp,zdest
                ja      rtol		; Jump if right to left	
		
	        ; Left to right scan
		
                mov     dest,edi
L33:
		jae     L50		; Jump if end of scan
		
                mov     source,eax	; Free up eax
		mov	ebx,z		; Get new z value
		mov	dx,[ebp]
		ror	ebx,16
                cmp     bx,dx
                ja      L34		; Jump if new pixel behind old
		
                mov     edx,zb.texture_buffer
                mov     [ebp],bx	; Store new z
		add	eax,edx
                mov     edx,zb.pz.grad_x	; Get z step
		mov	cl,[eax]
                add     ebp,2		; zdest++
		ror	ebx,16
		test	cl,cl		; Transparent?
		jz	transp2
                mov     [edi],cl	; Store texel
transp2:
                add     ebx,edx		; z += dz
		
		jmp	L34a
L34:
		ror	ebx,16
                mov     ecx,zb.pz.grad_x
                add     ebp,2
                add     ebx,ecx
L34a:
		inc	edi
		cmp	edi,tsl._end
		jae	L50
	        mov	edx,iW
                mov     dest,edi
		mov	edi,vdW
                mov     z,ebx
                mov     zdest,ebp
                mov     ecx,tsl.dWx
                mov     ebp,udW
		mov	ebx,Ev
                mov     eax,source	; Restore eax
		pre
                sub     esi,ebp
                add     edx,ecx
                cmp     esi,edx
                jge     incuf		; Jump if u not too large
		
L36:            test    esi,esi
		jl	decuf
L37:
		sub     ebx,edi
		test	ebx,ebx
		jl	decvf
                cmp     ebx,edx
                jge     incvf		; Jump if v not too small
L40:
		post1
		post2
		mov	iW,edx
		mov	Ev,ebx
		mov	edi,dest
                cmp     edi,tsl._end
		mov     ebp,zdest
                jmp     L33

	        ; Will be right to left scan
		; I was here!!!!!!!!!!!!!!!!!
		
rtol:
                mov     dest,edi
R33:
		jbe     L50		; Jump if end of scan
		
                mov     source,eax	; Free up eax
		mov	ebx,z		; Get new z value
		mov	dx,[ebp]
		ror	ebx,16
                cmp     bx,dx
                ja      R34		; Jump if new pixel behind old
		
                mov     edx,zb.texture_buffer
                mov     [ebp],bx	; Store new z
		add	eax,edx
                mov     edx,zb.pz.grad_x	; Get z step
		mov	cl,[eax]
                sub     ebp,2		; zdest++
		ror	ebx,16
		test	cl,cl		; Transparent?
		jz	transp2r
                mov     [edi],cl	; Store texel
transp2r:
                sub     ebx,edx		; z += dz
		
		jmp	R34a
R34:
		ror	ebx,16
                mov     ecx,zb.pz.grad_x
                sub     ebp,2
                sub     ebx,ecx
R34a:
		dec	edi
		cmp	edi,tsl._end
		jbe	L50
	        mov	edx,iW
                mov     dest,edi
		mov	edi,vdW
                mov     z,ebx
                mov     zdest,ebp
                mov     ecx,tsl.dWx
                mov     ebp,udW
		mov	ebx,Ev
                mov     eax,source	; Restore eax
		pre
                add     esi,ebp
                sub     edx,ecx
                cmp     esi,edx
                jge     incub		; Jump if u not too large
		
R36:            test    esi,esi
		jl	decub
R37:
		add     ebx,edi
		test	ebx,ebx
		jl	decvb
                cmp     ebx,edx
                jge     incvb		; Jump if v not too small
R40:
		post1
		post2
		mov	iW,edx
		mov	Ev,ebx
		mov	edi,dest
                cmp     edi,tsl._end
		mov     ebp,zdest
                jmp     R33
		
L50:
		popad
		ret
		
decuf:
                mov     ebp,udW
decufloop:
                decu
                sub     ebp,tsl.dWx
                add     esi,edx
                jnc     decufloop
		
                mov     udW,ebp
		jmp	L37
		
incuf:
                mov     ecx,tsl.dWx
		mov     ebp,udW
                sub     esi,edx
incufloop:
                incu
                add     ebp,ecx
                sub     esi,edx
                jge     incufloop
		add	esi,edx
		
                mov     udW,ebp
		jmp	L37

decvf:
                mov     ebp,tsl.dWx
decvfloop:       
                decv
                sub     edi,ebp
                add     ebx,edx
                jnc     decvfloop
		mov	vdW,edi
		jmp	L40
		
incvf:
                mov     ecx,tsl.dWx
                sub     ebx,edx
		
incvfloop:
		incv
                add     edi,ecx
                sub     ebx,edx
                jge     incvfloop
		
		add	ebx,edx
		mov	vdW,edi
		jmp	L40

		; Back
decub:
                mov     ebp,udW
decubloop:
                decu
                sub     ebp,tsl.dWx
                add     esi,edx
                jnc     decubloop
		
                mov     udW,ebp
		jmp	R37
		
incub:
                mov     ecx,tsl.dWx
		mov     ebp,udW
                sub     esi,edx
incubloop:
                incu
                add     ebp,ecx
                sub     esi,edx
                jge     incubloop
		add	esi,edx
		
                mov     udW,ebp
		jmp	R37

decvb:
                mov     ebp,tsl.dWx
decvbloop:       
                decv
                sub     edi,ebp
                add     ebx,edx
                jnc     decvbloop
		mov	vdW,edi
		jmp	R40
		
incvb:
                mov     ecx,tsl.dWx
                sub     ebx,edx
		
incvbloop:
		incv
                add     edi,ecx
                sub     ebx,edx
                jge     incvbloop
		
		add	ebx,edx
		mov	vdW,edi
		jmp	R40
		
		
name		endp
		endm

ScanLinePIZ2TIP	ScanLinePIZ2TIP256,<>,<inc al>,<dec al>,<inc ah>,<dec ah>,<>,<>
ScanLinePIZ2TIP	ScanLinePIZ2TIP64,<shl eax,2>,<add al,100b>,<sub al,100b>,<inc ah>,<dec ah>,<shr eax,2>,<and eax,0fffh>
ScanLinePIZ2TP	ScanLinePIZ2TP256,<>,<inc al>,<dec al>,<inc ah>,<dec ah>,<>,<>
ScanLinePIZ2TP	ScanLinePIZ2TP64,<shl eax,2>,<add al,100b>,<sub al,100b>,<inc ah>,<dec ah>,<shr eax,2>,<and eax,0fffh>
ScanLinePIZ2TP	ScanLinePIZ2TP1024,<shl eax,6>,<add ax,40h>,<sub ax,40h>,<add eax,10000h>,<sub eax,10000h>,<shr eax,6>,<and eax,0fffffh>

		end
