;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: vesaapi.asm 1.2 1995/06/30 15:42:25 sam Exp $
;; $Locker:  $
;;
;; VESA Interrupt API
;;
	.386p
	.model flat,c

	include dpmi.inc
		
	.data
		db '$Id: vesaapi.asm 1.2 1995/06/30 15:42:25 sam Exp $',0
		align 4
	extern _RealSegment0000:word

real_off	dw	0
real_seg	dw	0

prot_ptr	df	0

BLOCK_SIZE	equ	256	; Size of info blocks
MODE_MAX	equ	128	; Maximum number of modes to copy
MODE_PTR	equ	14	; Offset of video mode ptr  in info block

; Parameters to Pharlap INT 21, 2511
;
real_registers	struct
inum		word	0
rds		word	0
res		word	0
rfs		word	0
rgs		word	0
reax		dword	0
redx		dword	0
real_registers	ends


ifdef __DPMI__
real_regs	dpmi_registers	<,>
else
real_regs	real_registers	<,>
endif
	.code
	extern _RealSegmentsSetup:proc

; int BR_ASM_CALL _VESABegin(void);
;
; Allocate communications buffer
;
_VESABegin proc uses ebx esi edi es

		call	_RealSegmentsSetup
ifdef __DPMI__
	; Allocate DOS memory via DPMI
	;
		mov	eax,0100h
		mov	ebx,BLOCK_SIZE/16	 
		int	31h
		jc	failed
		mov	real_seg,ax
		mov	word ptr prot_ptr+4,dx
		xor	eax,eax
		mov	real_off,ax
		mov	dword ptr prot_ptr,eax

elseifdef __PHARLAP386__
	; Pharlap  - Use DOS comms. buffer
	;
	 
		mov	eax,02517h	
		int	21h
		cmp	edx,BLOCK_SIZE
		jl	failed

		mov	dword ptr real_off,ecx
		mov	dword ptr prot_ptr,ebx
		mov	word ptr prot_ptr+4,es
elseifdef __X32__

	; X32-VM - Use DOS comms. buffer
	;
		mov	eax,0250Dh
		int	21h
		cmp	ecx,BLOCK_SIZE
		jl	failed

		mov	dword ptr real_off,ebx
		mov	dword ptr prot_ptr,edx
		mov	word ptr prot_ptr+4,es
endif

		xor	eax,eax
		ret
failed:
		mov	eax,1
		ret
_VESABegin endp

; void BR_ASM_CALL _VESAEnd(void);
;
; Release buffer and tidy up
;
_VESAEnd proc uses ebx esi edi
ifdef __DPMI__
	; Allocate DOS memory via DPMI
	;
		mov	eax,0101h
		movzx	edx,word ptr prot_ptr+4
		int	31h
endif
		ret
_VESAEnd endp

; int BR_ASM_CALL _VESAInfo(struct vesa_info *vip, br_uint_16 *modes);
;
; Read the general mode info and mode list
;
_VESAInfo proc uses ebx esi edi es, vip: ptr byte, modes:ptr word

		push	ebp
ifdef __DPMI__

	; Use DPMI to call real mode interrupt
	;
		mov	real_regs.reax,04f00h
		xor	eax,eax
		mov	ax,real_off
		mov	real_regs.redi,eax
		mov	ax,real_seg
		mov	real_regs.rds,ax
		mov	real_regs.res,ax

		mov	ax,ds
		mov	es,ax
		mov	edi,offset real_regs
		mov	eax,300h
		mov	ebx,10h
		xor	ecx,ecx
		int	31h
		cmp	word ptr real_regs.reax,0004fh
		jne	failed
else
	; Use Pharlap/X32 to call real mode
	;
		mov	real_regs.inum,010h
		mov	real_regs.reax,04f00h
		mov	ax,real_seg
		mov	real_regs.res,ax
		movzx	edi,real_off

		mov	eax,02511h
		mov	edx,offset real_regs
		int	21h

		cmp	ax,0004fh
		jne	failed
endif
     		pop	ebp

	; Copy parameter block into callers structure
	;

		mov	ax,ds
		mov	es,ax
		mov	edi,vip

		push	ds		
		lds	esi,prot_ptr
		mov	ebx,[esi+MODE_PTR]
		mov	ecx,BLOCK_SIZE/4
		rep	movsd

		pop	ds

	; Copy modes into callers array
	;
		mov	edi,modes
ifdef __DPMI__
		mov	esi,ebx
		shr	ebx,16
		mov	eax,0002h
		int	31h
		jc	failed
		mov	es,ax
		and	esi,0ffffh
else 
	; Get selector of real memeory
	;
		mov	es,_RealSegment0000

	; Convert real seg:off to physical address
	;
		mov	esi,ebx
		shr	ebx,16
		shl	ebx,4
		and	esi,0ffffh
		add	esi,ebx
endif

		mov	ecx,MODE_MAX
mode_loop:
		mov	ax,es:[esi]
		mov	[edi],ax
		add	esi,2
		add	edi,2
		cmp	ax,0ffffh
		loopne	mode_loop
		
		xor	eax,eax
		ret

failed:		mov	eax,1
		pop	ebp
		ret
_VESAInfo endp

; int BR_ASM_CALL _VESAModeInfo(struct vesa_modeinfo *vmip, br_uint_32 mode);
;
; Read mode specific information
;
_VESAModeInfo proc uses ebx esi edi es, vmip: ptr byte, mode:dword
		push	ebp

		mov	ecx,mode

ifdef __DPMI__

	; Use DPMI to call real mode interrupt
	;
		mov	real_regs.reax,04f01h
		mov	real_regs.recx,ecx
		xor	eax,eax
		mov	ax,real_off
		mov	real_regs.redi,eax
		mov	ax,real_seg
		mov	real_regs.rds,ax
		mov	real_regs.res,ax

		mov	ax,ds
		mov	es,ax
		mov	edi,offset real_regs
		mov	eax,300h
		mov	ebx,10h
		xor	ecx,ecx
		int	31h
		cmp	word ptr real_regs.reax,0004fh
		jne	failed
else
	; Use Pharlap/X32 to call real mode
	;
		mov	real_regs.inum,010h
		mov	real_regs.reax,04f01h
		mov	ax,real_seg
		mov	real_regs.res,ax
		movzx	edi,real_off

		mov	eax,02511h
		mov	edx,offset real_regs
		int	21h

		cmp	ax,0004fh
		jne	failed
endif
     		pop	ebp

	; Copy parameter block into callers structure
	;

		mov	ax,ds
		mov	es,ax
		mov	edi,vmip

		push	ds		
		lds	esi,prot_ptr
		mov	ebx,[esi+MODE_PTR]
		mov	ecx,BLOCK_SIZE/4
		rep	movsd

		pop	ds

		xor	eax,eax
		ret

failed:		mov	eax,1
		ret
_VESAModeInfo endp

; int BR_ASM_CALL _VESAModeSet(br_uint_32 mode);
;
; Set the current mode
;
_VESAModeSet proc uses ebx esi edi, mode:dword
		mov	eax,04f02h
		mov	ebx,mode
		int	10h

return_status label near
		cmp	ax,4fh
		je	ok
		mov	eax,1
		ret
ok:		xor	eax,eax
		ret
_VESAModeSet endp

; int BR_ASM_CALL _VESAModeGet(br_uint_16 *mode);
;
; Return the current mode
;
_VESAModeGet proc uses ebx esi edi, mode:ptr word
		mov	eax,04f03h
		int	10h
		mov	esi,mode
		mov	[esi],bx
		jmp	return_status
_VESAModeGet endp

; int BR_ASM_CALL _VESAScanlineLengthSet(br_uint_32 width, br_uint_16 * bytes, br_uint_16 * pixels, br_uint_16 * scanlines);
;
; Set the current scanline length
;
_VESAScanlineLengthSet proc uses ebx esi edi, pwidth:dword, bytes:ptr word, pixels:ptr word , scanlines: ptr word
		mov	eax,4f06h
		xor	ebx,ebx
		mov	ecx,pwidth
		int	10h
		mov	esi,bytes
		mov	[esi],bx
		mov	esi,pixels
		mov	[esi],cx
		mov	esi,scanlines
		mov	[esi],dx
		jmp	return_status
_VESAScanlineLengthSet endp

; int BR_ASM_CALL _VESAScanlineLengthGet(br_uint_16 * bytes, br_uint_16 * pixels, br_uint_16 * scanlines);
;
; Return the current scanline length
;
_VESAScanlineLengthGet proc uses ebx esi edi, bytes:ptr word, pixels:ptr word , scanlines: ptr word
		mov	eax,4f06h
		mov	ebx,1
		int	10h
		mov	esi,bytes
		mov	[esi],bx
		mov	esi,pixels
		mov	[esi],cx
		mov	esi,scanlines
		mov	[esi],dx
		jmp	return_status
_VESAScanlineLengthGet endp


; int BR_ASM_CALL _VESADisplayStartSet(br_uint_32 x, br_uint_32 y);
;
; Set the current display start
;
_VESADisplayStartSet proc uses ebx esi edi, px:dword, py:dword
		mov	eax,4f07h
		xor	ebx,ebx
		mov	ecx,px
		mov	edx,py
		int	10h
		jmp	return_status
_VESADisplayStartSet endp

; int BR_ASM_CALL _VESADisplayStartGet(br_uint_32 *px, br_uint_32 *py);
;
; Return the display start x,y 
;
_VESADisplayStartGet proc uses ebx esi edi, px:ptr word, py:ptr byte
		mov	eax,4f07h
		mov	ebx,1
		int	10h
		mov	esi,px
		mov	[esi],cx
		mov	esi,py
		mov	[esi],dx
		jmp	return_status
_VESADisplayStartGet endp


; int BR_ASM_CALL _VESAWindowSet(br_uint_32 window, br_uint_32 position);
;
; Set the current window position
;
_VESAWindowSet proc uses ebx esi edi, window:dword, position:dword
		mov	eax,4f05h
		xor	ebx,ebx
		mov	bl,byte ptr window
		mov	edx,position
		int	10h
		jmp	return_status
_VESAWindowSet endp

; int BR_ASM_CALL _VESAWindowGet(br_uint_32 window, br_uint_32 *position);
;
; Get the current window position
;
_VESAWindowGet proc uses ebx esi edi, window:dword, position:ptr byte
		mov	eax,4f05h
		mov	ebx,0100h
		mov	bl,byte ptr window
		int	10h
		mov	esi,position
		mov	[esi],dx
		jmp	return_status
_VESAWindowGet endp

	end

