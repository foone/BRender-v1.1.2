;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: memloops.asm 1.5 1995/08/31 16:29:34 sam Exp $
;; $Locker:  $
;;
;; Inner loops for operations on in-memory pixelmaps
;;
	.386p
	.model flat,c
	.data
	db '$Id: memloops.asm 1.5 1995/08/31 16:29:34 sam Exp $',0
	align	4
bit_to_mask_s	db	11111111b
		db	01111111b
		db	00111111b
		db	00011111b
		db	00001111b
		db	00000111b
		db	00000011b
		db	00000001b
		db	00000000b

bit_to_mask_e	db	00000000b
		db	10000000b
		db	11000000b
		db	11100000b
		db	11110000b
		db	11111000b
		db	11111100b
		db	11111110b
		db	11111111b
	.code


;; Only one source byte wide
;;
COPY_BITS_CORE_1 macro	bpp
		local	v_loop

	if bpp eq 3
		mov	eax,ebx
		shr	eax,8
	endif
		and	dl,dh		; combine masks
		mov	ecx,n_rows
v_loop:
	; Get first source byte and mask it
	;
		mov	al,ss:[esi]
		and	al,dl

	; Set destination pixels according to byte mask
	;
x = 0
	rept 8
	local no_pixel
		add	al,al
		jnc	no_pixel
	if bpp eq 1
		mov	[edi+(x*1)],bl
	elseif bpp eq 2
		mov	[edi+(x*2)+0],bl
		mov	[edi+(x*2)+1],bh
	elseif bpp eq 3
		mov	[edi+(x*3)+0],bl
		mov	[edi+(x*3)+1],bh
		mov	[edi+(x*3)+2],ah
	elseif bpp eq 4
		mov	[edi+(x*4)],ebx
	endif
no_pixel:
x = x + 1
	endm

		add	edi,d_stride
		add	esi,s_stride

		dec	ecx
		jne	v_loop
		pop	ds
		ret
	endm

;; 2 or more source bytes wide
;;
COPY_BITS_CORE_N macro	bpp
		local	v_loop,h_loop,h_loop_last,done_row

	if bpp eq 3
		mov	eax,ebx
		shr	eax,8
	endif

v_loop:		push	ecx
		push	esi
		push	edi

	; Get first source byte and mask it
	;
		mov	al,ss:[esi]
		and	al,dl
h_loop:		inc	esi
h_loop_last:
	; Set destination pixels according to byte mask
	;
x = 0
	rept 8
	local no_pixel
		add	al,al
		jnc	no_pixel
	if bpp eq 1
		mov	[edi](x*1),bl
	elseif bpp eq 2
		mov	[edi]((x*2)+0),bl
		mov	[edi]((x*2)+1),bh
	elseif bpp eq 3
		mov	[edi]((x*3)+0),bl
		mov	[edi]((x*3)+1),bh
		mov	[edi]((x*3)+2),ah
	elseif bpp eq 4
		mov	[edi+(x*4)],ebx
	endif
no_pixel:
x = x + 1
	endm

	; Get next source byte
	;
		mov	al,ss:[esi]
		add	edi,8*bpp
		dec	ecx
		jg	h_loop
		js	done_row

	; If it is the last byte, mask and look one final time
	;
		and	al,dh
		jmp	h_loop_last
done_row:

	; End of row
	;
		pop	edi
		pop	esi
		pop	ecx
		add	edi,d_stride
		add	esi,s_stride

		dec	n_rows
		jne	v_loop
		pop	ds
		ret
	endm

; void BR_ASM_CALL _MemCopyBits_A(
;		char *dest, br_uint_32 dest_qual, br_int_32 d_stride,
;		br_uint_8 *src,br_uint_32 s_stride,br_uint_32 start_bit,br_uint_32 end_bit,
;		br_uint_32 nrows,br_uint_32 colour)
;
_MemCopyBits_A proc uses ebx esi edi,
	dest:ptr word,	dest_qual: dword, d_stride:dword,
	src: ptr byte, s_stride:dword, start_bit:dword, end_bit:dword,
	n_rows:dword, bpp:dword, colour: dword

		mov	edi,dest
		mov	esi,src

	; Advance to start byte
	;
		mov	eax,start_bit
		mov	ecx,end_bit
		mov	edx,eax

		shr	eax,3		; n_bytes = n_bits/8
		add	esi,eax
		shl	eax,3
		sub	ecx,eax

		push	edx
		mul	bpp
		add	edi,eax
		pop	edx

	; Work out count, and masks at start and end of line
	;
		mov	eax,ecx
		and	edx,7
		and	eax,7
		mov	dl,bit_to_mask_s[edx]
		mov	dh,bit_to_mask_e[eax]
		shr	ecx,3		; n_bytes = n_bits/8

		mov	ebx,colour
		mov	eax,bpp
		push	ds
		mov	ds,word ptr dest_qual

	; Go to appropriate core loops
	;
		test	ecx,ecx
		jne	multi_byte

		dec	eax
		je	bpp1_1
		dec	eax
		je	bpp1_2
		dec	eax
		je	bpp1_3
		dec	eax
		je	bpp1_4
		pop	ds
		ret
multi_byte:
		dec	eax
		je	bppn_1
		dec	eax
		je	bppn_2
		dec	eax
		je	bppn_3
		dec	eax
		je	bppn_4
		pop	ds
		ret

bpp1_1:		COPY_BITS_CORE_1	1
bpp1_2:		COPY_BITS_CORE_1	2
bpp1_3:		COPY_BITS_CORE_1	3
bpp1_4:		COPY_BITS_CORE_1	4
	
bppn_1:		COPY_BITS_CORE_N	1
bppn_2:		COPY_BITS_CORE_N	2
bppn_3:		COPY_BITS_CORE_N	3
bppn_4:		COPY_BITS_CORE_N	4

_MemCopyBits_A endp

; void BR_ASM_CALL _MemFill_A(char *dest, br_uint_32 dest_qual, br_uint_32 pixels, br_uint_32 bpp, br_uint_32 colour);
;
_MemFill_A proc uses ebx esi edi es,
	dest: ptr byte, dest_qual:dword, pixels: dword, bpp: dword, colour:dword

		mov	es,word ptr dest_qual
		mov	edi,dest
		mov	eax,colour
		mov	ecx,pixels

		mov	ebx,bpp
		cmp	ebx,4
		je	bpp_4
		cmp	ebx,3
		je	bpp_3
		cmp	ebx,2
		je	bpp_2
		cmp	ebx,1
		je	bpp_1
		ret

; 1 byte per pixel
;
bpp_1:
	; replicate colour into 32 bits
	;
		mov	ah,al
		mov	ebx,eax
		shl	eax,16
		mov	al,bl
		mov	ah,bl
bpp_1_fill:
	; Get aligned
	;
		mov	ebx,edi
		and	ebx,3
		je	aligned

align_loop:	mov	es:[edi],al
		inc	edi
		dec	ecx
		je	done
		dec	ebx
		jne	align_loop
aligned:
	; fill dwords
	;
		mov	ebx,ecx
		shr	ecx,2
		rep	stosd

	; Fill remaining bytes
	;
		mov	ecx,ebx
		and	ecx,3
		rep	stosb
done:
		ret		

; 2 byte per pixel
;
bpp_2:
	; replicate colour into 32 bits
	;
		mov	ebx,eax
		shl	eax,16
		mov	al,bl
		mov	ah,bh

	; fill dwords
	;
		shr	ecx,1
		rep	stosd

	; Fill remaining words
	;
		adc	ecx,ecx
		rep	stosw
		ret		

; 4 bytes per pixel - just go for it!
;
bpp_4:		rep	stosd
		ret

; 3 bytes per pixel
;	
bpp_3:
		cmp	al,ah	; If 3 fill bytes are the same, just use 1bpp fill
		jne	real_3
		mov	ebx,eax
		shr	ebx,8
		cmp	bh,al
		jne	real_3

		lea	ecx,[ecx*2+ecx]	; pixels *= 3
		rol	eax,8		; Set top byte of colour
		mov	al,ah
		jmp	bpp_1_fill
real_3:

	; Fill eax, ebx, edx with 4 repeats of colour
	;
					; EAX  EBX  EDX
					; LH   LH   LH
					; BGRX XXXX XXXX
		rol	eax,8		; XBGR XXXX XXXX
		mov	edx,eax		; XBGR XXXX XBGR
		mov	al,dh		; BBGR XXXX XBGR
		ror	eax,16		; GRBB XXXX XBGR
		mov	ebx,edx		; GRBB XBGR XBGR
		rol	ebx,8		; GRBB RXBG XBGR
		mov	bl,al		; GRBB GXBG XBGR
		mov	dl,ah		; GRBB GXBG RBGR
		mov	bh,ah		; GRBB GRBG RBGR
		rol	eax,8		; BGRB GRBG RBGR

		push	ds
		mov	ds,word ptr dest_qual

       	; Fill in 4 pixel chunks
	;
		mov	esi,ecx
		shr	esi,2
		je	bpp_3_last

bpp_3_loop:	mov	[edi+0],eax
		mov	[edi+4],ebx
		mov	[edi+8],edx
		add	edi,12
		dec	esi
		jne	bpp_3_loop

	; Do left over pixels a byte at a time
	;
bpp_3_last:
		and	ecx,3
		je	bpp_3_done
bpp_3_last_loop:
		mov	[edi+0],al
		mov	[edi+1],ah
		mov	[edi+2],bh
		add	edi,3
		dec	ecx
		jne	bpp_3_last_loop
bpp_3_done:
		pop	ds
		ret
_MemFill_A endp

; void BR_ASM_CALL _MemRectFill_A(char *dest, br_uint_32 dest_qual, br_uint_32 pwidth, br_uint_32 pheight,
; 				  br_int_32 d_stride, br_uint_32 bpp, br_uint_32 colour);
;
_MemRectFill_A proc uses ebx esi edi es,
	dest: ptr byte, dest_qual:dword, pwidth: dword, pheight: dword, d_stride:dword, bpp: dword, colour:dword

		mov	es,word ptr dest_qual
		mov	esi,d_stride
		mov	edi,dest
		mov	eax,colour
		mov	ecx,pwidth

		mov	ebx,bpp
		cmp	ebx,4
		je	bpp_4
		cmp	ebx,3
		je	bpp_3
		cmp	ebx,2
		je	bpp_2
		cmp	ebx,1
		je	bpp_1
		ret

; 1 byte per pixel
;
bpp_1:
	; replicate colour into 32 bits
	;
		mov	ah,al
		mov	ebx,eax
		shl	eax,16
		mov	al,bl
		mov	ah,bl

bpp_1_fill:
		sub	esi,ecx
		mov	ebx,ecx

row_loop_1:
		mov	ecx,ebx

	; Get aligned
	;
		mov	edx,edi
		and	edx,3
		je	aligned

align_loop:	mov	es:[edi],al
		inc	edi
		dec	ecx
		je	done
		dec	edx
		jne	align_loop
aligned:
	; fill dwords
	;
		mov	edx,ecx
		shr	ecx,2
		rep	stosd

	; Fill remaining bytes
	;
		mov	ecx,edx
		and	ecx,3
		rep	stosb

done:		add	edi,esi
		dec	pheight
		jne	row_loop_1
		ret

; 2 byte per pixel
;
bpp_2:
	; replicate colour into 32 bits
	;
		mov	ebx,eax
		shl	eax,16
		mov	al,bl
		mov	ah,bh

		sub	esi,ecx
		sub	esi,ecx

		mov	ebx,ecx
row_loop_2:
		mov	ecx,ebx

	; fill dwords
	;
		shr	ecx,1
		rep	stosd

	; Fill remaining words
	;
		adc	ecx,ecx
		rep	stosw

		add	edi,esi
		dec	pheight
		jne	row_loop_2
		ret		

; 4 bytes per pixel - just go for it!
;
bpp_4:
		sub	esi,ecx
		sub	esi,ecx
		sub	esi,ecx
		sub	esi,ecx

		mov	ebx,ecx
row_loop_4:
		mov	ecx,ebx
		rep	stosd

		add	edi,esi
		dec	pheight
		jne	row_loop_4
		ret

; 3 bytes per pixel
;	
bpp_3:
		cmp	al,ah	; If 3 fill bytes are the same, just use 1bpp fill
		jne	real_3
		mov	ebx,eax
		shr	ebx,8
		cmp	bh,al
		jne	real_3

		lea	ecx,[ecx*2+ecx]	; pixels *= 3
		rol	eax,8		; Set top byte of colour
		mov	al,ah
		jmp	bpp_1_fill

real_3:

	; Fill eax, ebx, edx with 4 repeats of colour
	;
					; EAX  EBX  EDX
					; LH   LH   LH
					; BGRX XXXX XXXX
		rol	eax,8		; XBGR XXXX XXXX
		mov	edx,eax		; XBGR XXXX XBGR
		mov	al,dh		; BBGR XXXX XBGR
		ror	eax,16		; GRBB XXXX XBGR
		mov	ebx,edx		; GRBB XBGR XBGR
		rol	ebx,8		; GRBB RXBG XBGR
		mov	bl,al		; GRBB GXBG XBGR
		mov	dl,ah		; GRBB GXBG RBGR
		mov	bh,ah		; GRBB GRBG RBGR
		rol	eax,8		; BGRB GRBG RBGR


		push	ds
		mov	ds,word ptr dest_qual		

row_loop_3:	push	ecx
		push	edi

       	; Fill in 4 pixel chunks
	;
		mov	esi,ecx
		shr	esi,2
		je	bpp_3_last

bpp_3_loop:	mov	[edi+0],eax
		mov	[edi+4],ebx
		mov	[edi+8],edx
		add	edi,12
		dec	esi
		jne	bpp_3_loop

	; Do left over pixels a byte at a time
	;
bpp_3_last:
		and	ecx,3
		je	bpp_3_done
bpp_3_last_loop:
		mov	[edi+0],al
		mov	[edi+1],ah
		mov	[edi+2],bh
		add	edi,3
		dec	ecx
		jne	bpp_3_last_loop
bpp_3_done:
		pop	edi
		pop	ecx
		add	edi,d_stride
		dec	pheight
		jne	row_loop_3

		pop	ds
		ret
_MemRectFill_A endp

; void BR_ASM_CALL _MemRectCopy_A(char *dest, br_uint_32 dest_qual, char *src, br_uint_32 src_qual,
;				  br_uint_32 pwidth, br_uint_32 pheight,
; 				  br_int_32 d_stride,br_int_32 s_stride,
;				  br_uint_32 bpp);
;
_MemRectCopy_A proc uses ebx esi edi ds es,
	dest: ptr byte, dest_qual:dword, src:ptr byte, src_qual: dword,
	pwidth: dword, pheight: dword, d_stride:dword, s_stride:dword, bpp: dword

		mov	ds,word ptr src_qual
		mov	es,word ptr dest_qual
		mov	esi,src
		mov	edi,dest

		mov	eax,bpp
	   	mul	pwidth
		mov	ecx,eax

row_loop:	push	ecx
		push	esi
		push	edi

	; Get destination aligned
	;
		mov	edx,edi
		and	edx,3
		je	aligned

align_loop:	mov	al,[esi]
		mov	es:[edi],al
		inc	esi
		inc	edi
		dec	ecx
		je	done
		dec	edx
		jne	align_loop
aligned:
	; Copy dwords
	;
		mov	edx,ecx
		shr	ecx,2
		rep	movsd

	; Copy remaining bytes
	;
		mov	ecx,edx
		and	ecx,3
		rep	movsb
done:
		pop	edi
		pop	esi
		pop	ecx

		add	edi,d_stride
		add	esi,s_stride
		dec	pheight
		jne	row_loop
		ret
_MemRectCopy_A endp

; void BR_ASM_CALL _MemCopy_A(char *dest, br_uint_32 dest_qual, char *src, br_uint_32 src_qual,
;				  br_uint_32 pixels,
;				  br_uint_32 bpp);
;
_MemCopy_A proc uses ebx esi edi ds es,
	dest: ptr byte, dest_qual:dword, src:ptr byte, src_qual:dword,
	pixels: dword, bpp: dword

		mov	ds,word ptr src_qual
		mov	es,word ptr dest_qual
		mov	esi,src
		mov	edi,dest

		mov	eax,bpp
	   	mul	pixels
		mov	ecx,eax

	; Get destination aligned
	;
		mov	edx,edi
		and	edx,3
		je	aligned

align_loop:	mov	al,[esi]
		mov	es:[edi],al
		inc	esi
		inc	edi
		dec	ecx
		dec	edx
		jne	align_loop

aligned:
	; Copy dwords
	;
		mov	edx,ecx
		shr	ecx,2
		rep	movsd

	; Copy remaining bytes
	;
		mov	ecx,edx
		and	ecx,3
		rep	stosb

		ret
_MemCopy_A endp


; void BR_ASM_CALL _MemLine(char *dest, br_uint_32 dest_qual, br_int_32 x1, br_int_32 y1,br_int_32 x2, br_int_32 y2, br_uint_32 bpp, br_uint_32 colour)
;
; Draw a line given address of first pixel, and y1 <= y2
;
; XXX


; void BR_ASM_CALL _MemPixelSet(char *dest, br_uint_32 dest_qual, br_uint_32 bytes, br_uint_32 colour)
;
; Plot a pixel
;
_MemPixelSet proc uses ebx esi edi es, dest: ptr byte, dest_qual:dword, bytes:dword, colour:dword
		mov	eax,colour
		mov	ebx,bytes
		mov	edi,dest
		mov	es,dest_qual
		cmp	ebx,1
		je	bpp_1
		cmp	ebx,2
		je	bpp_2
		cmp	ebx,3
		je	bpp_3
		cmp	ebx,4
		je	bpp_4
		ret

bpp_1:		mov	es:[edi],al
		ret

bpp_2:		mov	es:[edi],ax
		ret

bpp_4:		mov	es:[edi],eax
		ret

bpp_3:		mov	es:[edi+0],al
		mov	es:[edi+1],ah
		shr	eax,8
		mov	es:[edi+2],ah
     		ret
_MemPixelSet endp

; br_uint_32 BR_ASM_CALL _MemPixelGet(char *dest, br_uint_32 dest_qual, br_uint_32 bytes)
;
; Read a pixel
;
_MemPixelGet proc uses ebx esi edi es, dest: ptr byte, dest_qual:dword, bytes:dword
		mov	ebx,bytes
		mov	edi,dest
		mov	es,dest_qual
		xor	eax,eax

		cmp	ebx,1
		je	bpp_1
		cmp	ebx,2
		je	bpp_2
		cmp	ebx,3
		je	bpp_3
		cmp	ebx,4
		je	bpp_4
		ret

bpp_1:		mov	al,es:[edi]
		ret

bpp_2:		mov	ax,es:[edi]
		ret

bpp_4:		mov	eax,es:[edi]
		ret

bpp_3:		mov	ah,es:[edi+2]
		shl	eax,8
		mov	ah,es:[edi+1]
		mov	al,es:[edi+0]
     		ret
_MemPixelGet endp

; br_uint_16 BR_ASM_CALL _GetSysQual(void);
;
; Return a value for the qualifier argument that renders into system memory
;
_GetSysQual	proc
		mov	ax,ds
		ret
_GetSysQual	endp
	end

