;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: pcloops.asm 1.4 1995/06/30 15:42:16 sam Exp $
;; $Locker:  $
;;
	.386p
	.model flat,c
		
	.data
		db '$Id: pcloops.asm 1.4 1995/06/30 15:42:16 sam Exp $',0
		align 4
	.data


	public window_start
	public window_end
	public screen
	public pixelwidth
	public pixelheight
	public pc_gfx_set
	public pixel_size
	public page_size
	public full_banks
	public scanlines_per_page
	public lines_left
	public screen_offset_table
	public svga_modeinfo_ptr
	public svga_info_ptr
	public bank_shift
	public bank_increment
	public scanline_size
	public colour_buffer_size
	public source_scanline_size
	public current_page

; Per scanline info.
;
scanline struct 
	address		dword	?
	page_num	dword	?
	split		dword	?
	_pad		dword	?
scanline ends
	
window_start  		dd 0a000h
window_end  		dd 0b000h
screen			dd 0
screen_offset_table	dd 0
pixelwidth		dd 0
pixelheight		dd 0
pc_gfx_set		dd 0
scanline_size		dd 0
source_scanline_size 	dd 0
colour_buffer_size 	dd 0

pixel_size		dd 0
full_banks		dd 0
lines_left		dd 0
bank_shift		dd 0
bank_increment		dd 0

scanlines_per_page 	dd 0
page_size		dd 0

svga_modeinfo_ptr 	dd 0,0
svga_info_ptr		dd 0,0
current_page		dd 0
num_bytes		dd 0
byte_width		dd 0

break_in_scanline	dd 0
scanline_page		dd 0


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
		

	; set VESA page to edx - trashes ebx
	;
SET_PAGE	macro
		local	no_change

		cmp	edx,current_page
		je	no_change
		push	ebx
		mov	current_page,edx
		xor	ebx,ebx
		mov	eax,04f05h
		int	10h
		pop	ebx	
no_change:
		endm

SET_PAGE_FORCE	macro
		push	ebx
		xor	ebx,ebx
		mov	eax,04f05h
		mov	current_page,edx
		int	10h
		pop	ebx	
no_change:
		endm

	; Move to next vesa page
	;
INC_PAGE	macro
		push	edx
		push	ebx
		mov	edx,current_page
		add	edx,bank_increment
		mov	current_page,edx
		xor	ebx,ebx
		mov	eax,04f05h
		int	10h

		pop	ebx	
		pop	edx	

		endm

	.code
; void BR_ASM_CALL _PagedScreenCopyNoBreaks(br_uint_8 *dest, br_uint_32 dest_qual, br_uint_8 *src, br_uint_32 src_stride);
;	
; copy to paged screen with no page breaks in scanlines
;
	
_PagedScreenCopyNoBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword, src:ptr byte, src_stride:dword

		mov	edi,dest
		mov	es,dest_qual

	; Get source and sizes
	;
		mov	esi,src			; source
		mov	ecx,src_stride		; source stride
		sar	ecx,2
		mov	src_stride,ecx		; source stride / 4

	; copy all of the full display pages
	;	
		mov	ecx,full_banks	; number of full pages
		mov	ebx,0		; y
		
p_loop_0:	push	ecx
		push	ebx
		
	; get page from _screen_offset_table.page
	;
		shl	ebx,4
		mov	eax,screen_offset_table
		mov	edx,4[ebx+eax]
		mov	current_page,edx
		xor	ebx,ebx		
		mov	eax,4f05h
		
		int	10h
		
		pop	ebx
		
	; per display page
	;
		mov	ecx,scanlines_per_page
		mov	edx,ebx
		shl	edx,4
		add	edx,screen_offset_table

	; per scanline
	;
p_loop_1:	push	ecx
		push	edi
		
		mov	ecx,src_stride
		add	edi,[edx]
		rep	movsd
		
		add	edx,16
		inc	ebx
		
		pop	edi
		pop	ecx
		
		loop	p_loop_1
		
		pop	ecx
		loop	p_loop_0
		
		; remainder of scanlines in last page
		;
		mov	ecx,lines_left
		cmp	ecx,0
		jz	p_fin
		
		push	ebx
		
		shl	ebx,4
		mov	eax,screen_offset_table	
		mov	edx,4[ebx+eax]
		mov	current_page,edx
		mov	eax,4f05h
		
		xor	ebx,ebx
		int	10h
		
		pop	ebx
		
		mov	ecx,lines_left
		mov	edx,ebx
		shl	edx,4
		add	edx,screen_offset_table
		
p_loop_2:	push	ecx
		push	edi
		
		mov	ecx,src_stride
		add	edi,[edx]
		rep	movsd
		
		add	edx,16
		inc	ebx
	
		pop	edi
		pop	ecx
		loop	p_loop_2
		
p_fin:
		ret

_PagedScreenCopyNoBreaks endp

; void BR_ASM_CALL PagedScreenCopyBreaks(br_uint_8 *dest, br_uint_32 dest_qual, br_uint_8 *src, br_uint_32 src_stride);
;
; copy to paged screen with page breaks in scanlines
;
_PagedScreenCopyBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword, src:ptr byte, src_stride:dword

		mov	edi,dest
		mov	es,dest_qual

	; Get source and sizes
	;
		mov	esi,src		; source
		mov	ecx,src_stride	; stride
		sar	ecx,2
		mov	src_stride,ecx
		
		mov	ecx,full_banks
		mov	ebx,0		; y
		mov	edx,0		; page
		
pb_l_0:		push	ecx
		push	ebx
		push	edx
	
	; get page from _screen_offset_table.page
	;
		mov	ecx,bank_shift
		shl	edx,cl
		xor	ebx,ebx		
		mov	eax,4f05h
		mov	current_page,edx
		
		int	10h
		
		pop	edx
		pop	ebx
	
	; per page
	;
		mov	ecx,page_size
		sar	ecx,2
		
		push	edi
		
		rep	movsd
		
		pop	edi
		
		inc	edx
		inc	ebx
		
		pop	ecx
		loop	pb_l_0

	; do dribbly bits left over
	;
		mov	eax,lines_left
		cmp	eax,0
		jz	pb_fin
	
		mov	ecx,bank_shift
		shl	edx,cl
		xor	ebx,ebx		
		mov	eax,4f05h
		mov	current_page,edx
		
		int	10h
		
		mov	eax,lines_left
		mov	ecx,src_stride
		mul	ecx
		
		mov	ecx,eax
		rep	movsd

pb_fin:		ret

_PagedScreenCopyBreaks endp

;void BR_ASM_CALL _PagedRectangleCopyNoBreaks(
;	br_uint_8 *dest, br_uint_32 dest_qual,
;	br_uint_8 *src, br_uint_32 src_stride,
;	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height)
;
; rectangle copy to paged screen with no page breaks in scanlines
;
	
_PagedRectangleCopyNoBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword, src:ptr byte, src_stride:dword,
		px:dword, py:dword, pwidth:dword, pheight:dword

		mov	edi,dest
		mov	es,dest_qual

	; Get source and sizes
	;
		mov	eax,pwidth
		mul	pixel_size
		mov	ebx,eax
		sar	eax,2
		mov	pwidth,eax
		mov	ecx,ebx
		shl	eax,2
		sub	ecx,eax
		mov	num_bytes,ecx
	
		mov	esi,src		; source
		
		mov	ebx,py		; y
		
		mov	edx,px
		mov	eax,pixel_size
		mul	edx
		mov	edx,eax		; x
		
		mov	ecx,pheight	; height
p_r_c_0:	push	ecx
		push	esi
		push	edi
		
	; per scanline
	;
	
		mov	eax,screen_offset_table
		mov	ecx,ebx
		shl	ecx,4
		add	ecx,eax
		
		add	edi,[ecx]	; addr
		add	edi,edx		; +x
		
		push	edx
		
		mov	edx,4[ecx]
		cmp	edx,current_page
		jz	p_r_c_1
		
		push	ebx
		
		mov	current_page,edx
		
		mov	eax,04f05h
		xor	ebx,ebx
		int	10h		; set page
		
		pop	ebx
		
p_r_c_1:	pop	edx

	
	; pixel blt
	;
		mov	ecx,pwidth
		rep	movsd
		
		mov	ecx,num_bytes
		cmp	ecx,0
		jz	p_r_c_2
	
		rep	movsb
	
p_r_c_2:	pop	edi
		pop	esi
		
		add	esi,source_scanline_size
		inc	ebx		; y++
		
		pop	ecx
		loop p_r_c_0
		
		ret
_PagedRectangleCopyNoBreaks endp

;void BR_ASM_CALL _PagedRectangleCopyBreaks(
;	br_uint_8 *dest, br_uint_32 dest_qual,
;	br_uint_8 *src, br_uint_32 src_stride,
;	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height)
;
; rectangle copy to paged screen with page breaks in scanlines
;
_PagedRectangleCopyBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword, src:ptr byte, src_stride:dword,
		px:dword, py:dword, pwidth:dword, pheight:dword

		mov	edi,dest
		mov	es,dest_qual

	; Get source and sizes
	;
		mov	eax,pwidth
		mul	pixel_size
		mov	byte_width,eax

		mov	ebx,eax
		sar	eax,2
		mov	pwidth,eax
		and	ebx,3
		mov	num_bytes,ebx
	

		mov	esi,src		; source
		
		
		mov	eax,px
		mul	pixel_size
		mov	px,eax		; x
		
		mov	ecx,pheight	; height

		mov	ebx,py		; y
		shl	ebx,4
		add	ebx,screen_offset_table
		

scanline_loop:	push	ecx
		push	ebx
		push	esi
		push	edi

	; per scanline
	;
		add	edi,[ebx].scanline.address	; addr
		add	edi,px				; +x
		
		mov	edx,[ebx].scanline.page_num
		SET_PAGE

	; pixel blt
	;
		cmp	[ebx].scanline.split,0		; page break on scanline
		jne	has_break
		
	;	scanlines with no page breaks
	;
copy_and_loop:	
		mov	ecx,pwidth
		rep	movsd
		
		mov	ecx,num_bytes
		rep	movsb

		jmp	next_scanline
		
has_break:
	;	scanlines with page breaks
	;
		cmp	edi,window_end

	;	< 0  either before break or crossing break
	;	> 0  after break
	
		ja	after_break
		
		mov	eax,edi
		add	eax,byte_width

		cmp	eax,window_end
		
	;	> 0  spanning break
	;	< 0  all before break
		
		ja	spans_break
	    	jmp	copy_and_loop
		
after_break:	
	;	data after page break
	;
		INC_PAGE
		
		sub	edi,page_size

		jmp	copy_and_loop
		
spans_break:
	;	data spanning break
	;
	
		mov	ecx,window_end
		sub	ecx,edi

		mov	ebx,byte_width
		sub	ebx,ecx
		
		rep	movsb
		
		INC_PAGE
		
		mov	ecx,ebx
		sub	edi,page_size
		rep	movsb
		
next_scanline:	pop	edi
		pop	esi
		
		add	esi,source_scanline_size

		pop	ebx
		add	ebx,16		; y++
		
		pop	ecx
		dec	ecx
		jne	scanline_loop

		ret
_PagedRectangleCopyBreaks endp

;void BR_ASM_CALL _PagedRectangleClearNoBreaks(
;	br_uint_8 *dest, br_uint_32 dest_qual,
;	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height,
;	br_uint_32 colour)
;
; PagedRectangleClearNoBreaks(x,y,width,height,colour)
;			      8 12 16     20   24
;	
; rectangle clear to paged screen with no page breaks in scanlines
;
_PagedRectangleClearNoBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword,
		px:dword, py:dword, pwidth:dword, pheight:dword,
		colour:dword

		mov	edi,dest
		mov	es,dest_qual

	; Get source and sizes
	;
		mov	ebx,py		; y
		
		mov	edx,px
		mov	eax,pixel_size
		mul	edx
		mov	edx,eax		; x
		
		mov	ecx,pheight	; height
p_r_cl_0:	push	ecx
		push	edi
		
	; per scanline
	;
	
		mov	eax,screen_offset_table
		mov	ecx,ebx
		shl	ecx,4
		add	ecx,eax
		
		add	edi,[ecx]	; addr
		add	edi,edx		; +x
		
		push	edx
		
		mov	edx,4[ecx]
		cmp	edx,current_page
		jz	p_r_cl_1
		
		push	ebx
		
		mov	current_page,edx
		
		mov	eax,04f05h
		xor	ebx,ebx
		int	10h		; set page
		
		pop	ebx
		
p_r_cl_1:
	; pixel blt
	;
		mov	edx,pwidth
			
p_r_cl_2:
		mov	ecx,pixel_size
		lea	esi,colour
		rep	movsb
		
		dec	edx
		jne	p_r_cl_2
	
		pop	edx
		pop	edi
		
		add	esi,colour_buffer_size
		inc	ebx		; y++
		
		pop	ecx
		dec	ecx
		jne	p_r_cl_0

		
		ret
_PagedRectangleClearNoBreaks endp

;void BR_ASM_CALL _PagedRectangleClearBreaks(
;	br_uint_8 *dest, br_uint_32 dest_qual,
;	br_uint_32 x, br_uint_32 y, br_uint_32 width,br_uint_32 height,
;	br_uint_32 colour)
;
; PagedRectangleClearBreaks(x,y,width,height,colour)
;			      8 12 16     20   24
;	
; rectangle clear to paged screen with page breaks in scanlines
;
_PagedRectangleClearBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword,
		px:dword, py:dword, pwidth:dword, pheight:dword,
		colour:dword

		mov	edi,dest
		mov	es,dest_qual

	; Get source and sizes
	;
		mov	ebx,py	; y
		
		mov	edx,px
		mov	eax,pixel_size
		mul	edx
		mov	edx,eax		; x
		
		mov	ecx,pheight	; height
p_r_clb_0:	push	ecx
		push	edi
		
	; per scanline
	;
	
		mov	eax,screen_offset_table
		mov	ecx,ebx
		shl	ecx,4
		add	ecx,eax
		
		add	edi,[ecx].scanline.address	; addr
		add	edi,edx		; +x
		
		push	edx
		
		mov	edx,[ecx].scanline.page_num
		SET_PAGE
		pop	edx

	; pixel blt
	;
		cmp	[ecx].scanline.split,0	; page break on scanline
		jne	p_r_clb_2
		
	;	scanlines with no page breaks
	;
	
		mov	ecx,pwidth
p_r_clb_4:	push	ecx
		lea	esi,colour
		mov	ecx,pixel_size
		
		rep	movsb

		pop	ecx
		loop	p_r_clb_4
	
		jmp	p_r_clb_3
		
p_r_clb_2:
	;	scanlines with page breaks
	;
		mov	ecx,window_end
		sub	ecx,edi
		
	;	> 0  either before break or crossing break
	;	< 0  after break
	
		jle	p_r_clb_5
		
		;	ecx = bytes left in page
		
		push	edx
		
		mov	eax,pwidth
		mov	edx,pixel_size
		mul	edx		; width*pixelsize
		pop	edx 		
		
		sub	ecx,eax		; bytes_left - width*pixelsize
		
	;	> 0  all before break
	;	< 0  spanning break
		
		jle	p_r_clb_6
		
		mov	ecx,pwidth
p_r_clb_8:	push	ecx
		mov	eax,ecx
		lea	esi,colour
		mov	ecx,pixel_size
		
		rep	movsb
		
		pop	ecx
		loop	p_r_clb_8
		
		jmp	p_r_clb_3
		
p_r_clb_5:	
	;	data after page break
	;
		INC_PAGE
		sub	edi,page_size

		mov	ecx,pwidth
p_r_clb_7:	push	ecx
		lea	esi,colour
		mov	ecx,pixel_size
		
		rep	movsb
		
		pop	ecx
		loop	p_r_clb_7
		jmp	p_r_clb_3
		
p_r_clb_6:
	;	data spanning break
	;
		mov	ecx,window_end
		sub	ecx,edi
		mov	eax,pwidth

p_r_clb_9:	lea	esi,colour
		mov	ecx,pixel_size
		
		rep	movsb
		dec	eax		
		cmp	edi,window_end
		jne	p_r_clb_9
			
	;	flip pages
	;
		push	eax
		INC_PAGE
		pop	eax
	
	;	copy rest
	;
		and	eax,eax
		jz p_r_clb_3

		mov	ecx,eax
		sub	edi,page_size

p_r_clb_a:	push	ecx
		lea	esi,colour
		mov	ecx,pixel_size
		
		rep	movsb
		
		pop	ecx
		loop	p_r_clb_a
		
p_r_clb_3:	pop	edi
		
		inc	ebx		; y++
		
		pop	ecx
		dec	ecx
		jz	p_r_clb_out
		jmp	p_r_clb_0
		
p_r_clb_out:
		ret
_PagedRectangleClearBreaks endp

; PagedSetPixelNoBreaks(x,y,br_colour)
;	   		8 12 16
;
; void BR_ASM_CALL _PagedSetPixelNoBreaks(br_uint_8 *dest, br_uint_32 qual, br_uint_32 px, br_uint_32 py, br_uint_32 colour)
;	
; set pixel on paged screen no page breaks in scanlines
;
_PagedSetPixelNoBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword,
		px:dword, py:dword,
		colour:dword

		mov	edi,dest
		mov	es,dest_qual
		
	; Get source and sizes
	;
		
		mov	edx,py		; y
		mov	ebx,screen_offset_table
		shl	edx,4
		add	ebx,edx
		add	edi,[ebx].scanline.address
		
		mov	eax,px		; x
		mul	pixel_size
		
		add	edi,eax
		
		mov	edx,[ebx].scanline.page_num
		SET_PAGE

		mov	ecx,pixel_size
		
		lea	esi,colour

		rep	movsb
		
		ret
_PagedSetPixelNoBreaks endp

; PagedSetPixelBreaks(x,y,br_colour)
;	   	     8  12  16
;	
; void BR_ASM_CALL _PagedSetPixelBreaks(br_uint_8 *dest, br_uint_32 qual, br_uint_32 px, br_uint_32 py, br_uint_32 colour)
;	
; set pixel on paged screen page breaks in scanlines
;
_PagedSetPixelBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword,
		px:dword, py:dword,
		colour:dword

		mov	edi,dest
		mov	es,dest_qual
		
	; Get source and sizes
	;
		mov	edx,py		; y
		mov	ebx,screen_offset_table
		shl	edx,4
		add	ebx,edx

		add	edi,[ebx].scanline.address
		
		mov	eax,px		; x
		mul	pixel_size
		
		add	edi,eax
		
		mov	edx,[ebx].scanline.page_num
		cmp	[ebx].scanline.split,0
		je	p_spb_2		; no page break in scanline
		
		cmp	edi,window_end
		jb	p_spb_2		; same page
	
		add	edx,bank_increment ; next page
		sub	edi,page_size
		
p_spb_2:
		SET_PAGE
		
		mov	ecx,pixel_size
		lea	esi,colour
		rep	movsb
		
		ret
_PagedSetPixelBreaks endp


; void BR_ASM_CALL _PagedCopyBitsNoBreaks(
;		br_uint_8 *dest,
;		br_uint_32 dest_qual,
;		br_int_32 x,
;		br_int_32 y,
;		br_uint_8 *src,
;		br_uint_32 s_stride,
;		br_uint_32 start_bit,
;		br_uint_32 end_bit,
;               br_uint_32 n_rows,
;		br_uint_32 colour);
;

;	copy bits for paged VESA display with no page breaks in scanlines
;
	
_PagedCopyBitsNoBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword,
		px:dword, py:dword,
		src:ptr byte, src_stride:dword,
		start_bit:dword, end_bit:dword,
		n_rows:dword, colour:dword
	
		mov	es,word ptr dest_qual
		mov	edi,dest

		mov	esi,src

	; Advance to start byte
	;
		mov	eax,px
		mul	pixel_size
		mov	px,eax		; x
	
		mov	ebx,py		; y
		shl	ebx,4
		add	ebx,screen_offset_table
	
		mov	eax,start_bit	; start_bit
		mov	ecx,end_bit	; end_bit
		mov	edx,eax

		shr	eax,3		; n_bytes = n_bits/8
		add	esi,eax
		shl	eax,3
		add	edi,eax
		sub	ecx,eax

	; Work out count, and masks at start and end of line
	;
		mov	eax,ecx
		and	edx,7
		and	eax,7
		mov	dl,bit_to_mask_s[edx]
		mov	dh,bit_to_mask_e[eax]
		shr	ecx,3		; n_bytes = n_bits/8

		test	ecx,ecx
		jne	multi_byte

	; Only one byte wide
	;
		and	dl,dh		; combine masks
		mov	ecx,n_rows	; n_rows

v1_loop:	push	edi
		add	edi,[ebx].scanline.address
		add	edi,px
		
	; Get first source byte and mask it
	;
		mov	al,[esi]
		and	al,dl

	; Set destination pixels according to byte mask
	;
		push	ecx
		push	esi
		push	ebx
		
		push	eax
		push	edx
		mov	edx,[ebx].scanline.page_num
		SET_PAGE
		pop	edx
		pop	eax

		mov	ebx,0
		
	rept 8
	local no_pixel
		add	al,al
		jnc	no_pixel
		
		mov	ecx,pixel_size
		lea	esi,colour
	
		push	edi
			
		add	edi,ebx
		
		rep	movsb
		
		pop	edi
no_pixel:
		add	ebx,pixel_size
	endm
		pop	ebx
		pop	esi
		pop	ecx
		
		pop	edi

		add	esi,src_stride
		
		add	ebx,16

		dec	ecx
		jne	v1_loop

		ret

	; 2 or more source bytes wide
	;
		
multi_byte:

v_loop:		push	ebx
		push	ecx
		push	esi
		push	edi
		push	ecx

		add	edi,[ebx].scanline.address
		add	edi,px

		push	edx		
		mov	edx,[ebx].scanline.page_num
		SET_PAGE
		pop	edx
		
		pop	ecx

	; Get first source byte and mask it
	;
		mov	al,[esi]
		and	al,dl
h_loop:		inc	esi
h_loop_last:
	; Set destination pixels according to byte mask
	;
		push	ecx
		push	esi
		
		mov	ebx,0
		
	rept 8
	local no_pixel
		add	al,al
		jnc	no_pixel
		
		push	edi
		lea	esi,colour
		add	edi,ebx
		mov	ecx,pixel_size
		
		rep	movsb
		
		pop	edi
no_pixel:
		add	ebx,pixel_size
	endm
		pop	esi
		pop	ecx

	; Get next source byte
	;
		mov	eax,pixel_size
		shl	eax,3
		add	edi,eax
		
		mov	al,[esi]
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
		pop	ebx
		
		add	ebx,16
		
		add	esi,src_stride

		dec	n_rows
		jne	v_loop

		ret

_PagedCopyBitsNoBreaks endp

; void BR_ASM_CALL _PagedCopyBitsBreaks(
;		br_uint_8 *dest,
;		br_uint_32 dest_qual,
;		br_int_32 x,
;		br_int_32 y,
;		br_uint_8 *src,
;		br_uint_32 s_stride,
;		br_uint_32 start_bit,
;		br_uint_32 end_bit,
;               br_uint_32 n_rows,
;		br_uint_32 colour);
;
; void PagedCopyBitsBreaks(
;		br_uint_16 x,		8
;		br_uint_16 y,   	12
;		br_uint_8 *src,		16
;		br_uint_16 s_stride,	20
;		br_uint_16 start_bit,	24
;		br_uint_16 end_bit,	28
;               br_uint_16 n_rows,	32
;		br_uint_32 colour);	36
;
;	copy bits for paged VESA display with page breaks in scanlines
;

_PagedCopyBitsBreaks proc uses ebx esi edi es,
		dest:ptr byte, dest_qual:dword,
		px:dword, py:dword,
		src:ptr byte, src_stride:dword,
		start_bit:dword, end_bit:dword,
		n_rows:dword, colour:dword
	
		mov	es,word ptr dest_qual
		mov	edi,dest

		mov	esi,src

	; Advance to start byte
	;
		mov	eax,px
		mul	pixel_size
		mov	px,eax		; x
	
		mov	ebx,py		; y
		shl	ebx,4
		add	ebx,screen_offset_table
	
		mov	eax,start_bit	; start_bit
		mov	ecx,end_bit	; end_bit
		mov	edx,eax

		shr	eax,3		; n_bytes = n_bits/8
		add	esi,eax
		shl	eax,3
		add	edi,eax
		sub	ecx,eax

	; Work out count, and masks at start and end of line
	;
		mov	eax,ecx
		and	edx,7
		and	eax,7
		mov	dl,bit_to_mask_s[edx]
		mov	dh,bit_to_mask_e[eax]
		shr	ecx,3		; n_bytes = n_bits/8

		test	ecx,ecx
		jne	multi_byte

	; Only one byte wide
	;
		and	dl,dh		; combine masks
		mov	ecx,n_rows	; n_rows

v1_loop:	push	edi
		add	edi,[ebx].scanline.address
		add	edi,px
		

		push	edx
		mov	edx,[ebx].scanline.page_num
		mov	scanline_page,edx
		SET_PAGE
		pop	edx

	; Get first source byte and mask it
	;
		mov	al,[esi]
		and	al,dl

	; Set destination pixels according to byte mask
	;
		push	ecx
		push	esi
		push	ebx
		
		mov	ecx,[ebx].scanline.split
		mov	break_in_scanline,ecx
		
		mov	ebx,0
		
	rept 8
	local no_pixel,no_page_1a
		add	al,al
		jnc	no_pixel
		
		mov	ecx,pixel_size
		lea	esi,colour
	
		push	edi
			
		add	edi,ebx
		
		push	ecx
		
		mov	ecx,break_in_scanline
		jz	no_page_1a		; no break in scanline
		
		cmp	edi,window_end
		jb	no_page_1a		; before break
		
		push	eax
		push	edx
		push	ebx

		mov	eax,04f05h
		xor	ebx,ebx
		mov	edx,1
		mov	ecx,bank_shift
		shl	edx,cl
		
		add	edx,scanline_page	; next page to current scanline
		mov	current_page,edx
		
		int	10h
		
		sub	edi,page_size
		
		pop	ebx
		pop	edx
		pop	eax

no_page_1a:	pop	ecx
				
		rep	movsb
		
		pop	edi
no_pixel:
		add	ebx,pixel_size
	endm
		pop	ebx
		pop	esi
		pop	ecx
		
		pop	edi

		add	esi,src_stride
		
		add	ebx,16

		dec	ecx
		jne	v1_loop
		
		ret

	; 2 or more source bytes wide
	;
		
multi_byte:

v_loop:		push	ebx
		push	ecx
		push	esi
		push	edi
		push	ecx
		
		add	edi,[ebx].scanline.address
		add	edi,px
		
		mov	ecx,[ebx].scanline.split
		mov	break_in_scanline,ecx
		
		push	edx
		mov	edx,[ebx].scanline.page_num
		mov	scanline_page,edx
		SET_PAGE
		pop	edx

no_page_2:	pop	ecx

	; Get first source byte and mask it
	;
		mov	al,[esi]
		and	al,dl
h_loop:		inc	esi
h_loop_last:
	; Set destination pixels according to byte mask
	;
		push	ecx
		push	esi
		
		mov	ebx,0
		
	rept 8
	local no_pixel,no_page_2a
		add	al,al
		jnc	no_pixel
		
		push	edi
		lea	esi,colour
		add	edi,ebx
		mov	ecx,pixel_size
		
		push	ecx
		
		cmp	break_in_scanline,0
		je	no_page_2a		; no page break in scanline
		
		cmp	edi,window_end
		jb	no_page_2a		; before break in scanline
		
		push	eax
		push	ebx
		push	edx
		
		mov	eax,04f05h
		xor	ebx,ebx
		mov	edx,1
		mov	ecx,bank_shift
		shl	edx,cl
		add	edx,scanline_page	; page after current scanline
		mov	current_page,edx
		
		int	10h
		
		
		pop	edx
		pop	ebx
		pop	eax
		
		sub	edi,page_size

no_page_2a:	pop	ecx
		
		rep	movsb
		
		pop	edi
no_pixel:
		add	ebx,pixel_size
	endm
		pop	esi
		pop	ecx

	; Get next source byte
	;
		mov	eax,pixel_size
		shl	eax,3
		add	edi,eax
		
		mov	al,[esi]
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
		pop	ebx
		
		add	ebx,16
		
		add	esi,src_stride

		dec	n_rows
		jne	v_loop

		ret

_PagedCopyBitsBreaks endp


		end
