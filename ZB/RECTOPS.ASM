;; Copyright (c) 1992,1994,1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: rectops.asm 1.3 1995/02/22 21:57:00 sam Exp $
;; $Locker:  $
;;
;; Some special case routines for supporting rectangles in pixelmaps
;;
;; Clear rectangle to value
;;
;; Copy a rectangle bwteewen two pixelmaps of the same geometry
;;
	.386p
	.model flat,c
	.data
	db '$Id: rectops.asm 1.3 1995/02/22 21:57:00 sam Exp $',0
	align	4

; Pixelmap structure - coresponds to inc/pixelmap.h
;

BR_PMT_INDEX_1		equ	0
BR_PMT_INDEX_2		equ	1
BR_PMT_INDEX_4		equ	2
BR_PMT_INDEX_8		equ	3

BR_PMT_RGB_555		equ	4
BR_PMT_RGB_565		equ	5
BR_PMT_RGB_888		equ	6
BR_PMT_RGBX_888		equ	7
BR_PMT_RGBA_8888	equ	8

BR_PMT_YUYV_8888	equ	9
BR_PMT_YUV_888		equ	10
BR_PMT_DEPTH_16		equ	11
BR_PMT_DEPTH_32		equ	12
BR_PMT_ALPHA_8		equ	13


BR_PMF_PIXELS_ALLOCED	equ	1
BR_PMF_MAP_ALLOCED	equ	2
BR_PMF_NO_ACCESS	equ	4
BR_PMF_QUALIFIED	equ	8

br_pixelmap	struct 4
	identifier	dword	 	?
	pixels		dword	 	?
	qualifier	dword		?
	map		dword 		?
	row_bytes	word		?
	_type		byte		?
	flags		byte		?
	base_x		word 		?
	base_y		word		?
	_width		word 		?
	height		word 		?
	origin_x	word 		?
	origin_y	word 		?
br_pixelmap	ends

	; Table of shift values to convert pixels to bits
	;
pixel_type_shift label byte
	;	Shift		Bits pp
	db	0	;	1	BR_PMT_INDEX_1		
	db	1	;	2	BR_PMT_INDEX_2		
	db	2	;	4	BR_PMT_INDEX_4		
	db	3	;	8	BR_PMT_INDEX_8		

	db	4	;	16	BR_PMT_RGB_555		
	db	4	;	16	BR_PMT_RGB_565		
	db	5	;	24	BR_PMT_RGB_888		
	db	5	;	32	BR_PMT_RGBX_888		
	db	5	;	32	BR_PMT_RGBA_8888	

	db	4	;	16	BR_PMT_YUYV_8888	
	db	5	;	32	BR_PMT_YUV_888		

	db	4	;	16	BR_PMT_DEPTH_16		
	db	5	;	32	BR_PMT_DEPTH_32		
	db	3	;	8	BR_PMT_ALPHA_8		

	.code

; void BR_ASM_CALL GfxRectangleClear(br_pixelmap *dest,
;			br_uint_16 x0, br_uint_16 y0,
;			br_uint_16 x1, br_uint_16 y1,
;			br_uint_32 pvalue);
;
; Clears the rectangle between (x0,y0) and (x1,y1) inclusive to the given pixel value
;
; 'pvalue' should have the required pixel value replicated up to 32 bits
;
; The touched rectangle may be extended to aligned boundaries
;
GfxRectangleClear proc uses ebx esi edi es ,
		dest : ptr br_pixelmap,
		x0 : dword, y0 : dword,
		x1 : dword, y1 : dword,
		pvalue : dword

	; Get dest pixelmap ptr
	;
		mov	esi,dest

	; Work out start aligned to next .le. dword
	;
		xor	eax,eax
		mov	al,[esi].br_pixelmap._type
		mov	cl,pixel_type_shift[eax]
		mov	edi,x0
		mov	ebx,x1
		inc	ebx
		shl	edi,cl		; X end as bit offset
		shl	ebx,cl		; X end as bit offset

		add	ebx,31		; Align to dword
		shr	edi,5		; convert to dword offset
		shr	ebx,5

	; Work out dword count
	;
		sub	ebx,edi

	; Work out destination pointer
	;
		movsx	ecx,[esi].br_pixelmap.row_bytes
		mov	eax,y0
		imul	ecx
		lea	edi,[eax+edi*4]
		add	edi,[esi].br_pixelmap.pixels

	; Work out modulo
	;
		mov	eax,ebx
		shl	eax,2
		sub	ecx,eax

		mov	esi,ecx

	; Work out row count
	;
		mov	edx,y1
		sub	edx,y0

	; Clear scanlines
	;		
;		mov	ax,ds
;		mov	es,ax
		mov	eax,pvalue

row_loop:	mov	ecx,ebx		; Row width in dwords

if 1
		rep	stosd
else
word_loop:
		jecxz	wloop_done
		xor	[edi],eax
		add	edi,4
		dec	ecx
		jne	word_loop
wloop_done:
endif		
		add	edi,esi		; Move to next row
		dec	edx
		jns	row_loop

		ret
GfxRectangleClear endp

; void BR_ASM_CALL GfxRectangleTransfer(br_pixelmap *dest,
;			br_uint_16 x0, br_uint_16 y0,
;			br_uint_16 x1, br_uint_16 y1,
;			br_pixlmap *src);
;
; Transfers the rectangle between (x0,y0) and (x1,y1) inclusive from src to dest
;
; Src and dest pixelmaps must be of of the same type and geometry.
;
; The touched rectangle may be extended to aligned boundaries
;
GfxRectangleTransfer proc uses ebx esi edi es ,
		dest : ptr br_pixelmap,
		x0 : dword, y0 : dword,
		x1 : dword, y1 : dword,
		src : ptr br_pixelmap

	; Get source pixelmap ptr
	;
		mov	esi,src

	; Work out start aligned to next .le. dword
	;
		xor	eax,eax
		mov	al,[esi].br_pixelmap._type
		mov	cl,pixel_type_shift[eax]
		mov	edi,x0
		mov	ebx,x1
		inc	ebx
		shl	edi,cl		; X end as bit offset
		shl	ebx,cl		; X end as bit offset

		add	ebx,31		; Align to dword
		shr	edi,5		; convert to dword offset
		shr	ebx,5

	; Work out dword count
	;
		sub	ebx,edi

	; Work out source and destination pointers
	;
		movsx	ecx,[esi].br_pixelmap.row_bytes
		mov	eax,y0
		imul	ecx

		lea	eax,[eax+edi*4]

		mov	edi,dest
		mov	esi,[esi].br_pixelmap.pixels
		mov	edi,[edi].br_pixelmap.pixels
		add	esi,eax
		add	edi,eax

	; Work out modulo
	;
		mov	eax,ebx
		shl	eax,2
		sub	ecx,eax

		mov	eax,ecx
			  
	; Work out row count
	;
		mov	edx,y1
		sub	edx,y0


	; Copy scanlines
	;		
;		mov	cx,ds
;		mov	es,cx

row_loop:	mov	ecx,ebx		; Row width in dwords

if 1
		rep	movsd
else
		jecxz	wloop_done
		push	eax
word_loop:
		mov	eax,[esi]
		xor	eax,055555555h
		mov	[edi],eax
		add	esi,4
		add	edi,4
		dec	ecx
		jne	word_loop
		pop	eax
wloop_done:
endif
		add	esi,eax
		add	edi,eax		; Move to next row
		dec	edx
		jns	row_loop

		ret
GfxRectangleTransfer endp



  	end

