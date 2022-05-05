;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: farmem.asm 1.1 1995/06/30 15:44:14 sam Exp $
;; $Locker:  $
;;
;; Support routines for accessing far memory
;;
	.386p
	.model flat

	.code

; void BR_ASM_CALL FarBlockWrite(int offset, short seg, char *block, int count);
;
_FarBlockWrite proc 
	push	es
	push	esi
	push	edi
	les	edi,16[esp]
	mov	esi,24[esp]
	mov	ecx,28[esp]
	shr	ecx,1
	rep	movsw
	adc	ecx,ecx
	rep	movsb
	pop	edi
	pop	esi
	pop	es
	ret
_FarBlockWrite endp

; void BR_ASM_CALL FarBlockFill(int offset, short seg, unsigned byte value, int count);
;
_FarBlockFill proc 
	push	es
	push	esi
	push	edi
	les	edi,16[esp]
	mov	al,24[esp]
	mov	ecx,28[esp]
	mov	ah,al
	shr	ecx,1
	rep	stosw
	adc	ecx,ecx
	rep	stosb
	pop	edi
	pop	esi
	pop	es
	ret
_FarBlockFill endp

; void BR_ASM_CALL FarBlockRead(int offset, short seg, char *block, int count);
;
_FarBlockRead proc
	push	ds
	push	esi
	push	edi
	lds	esi,16[esp]
	mov	edi,24[esp]
	mov	ecx,28[esp]
	shr	ecx,1
	rep	movsw
	adc	ecx,ecx
	rep	movsb
	pop	edi
	pop	esi
	pop	ds
	ret
_FarBlockRead endp

; void BR_ASM_CALL FarByteWrite(int offset, short seg, char value);
;
_FarByteWrite proc
	push	ds
	lds	edx,8[esp]
	mov	al,16[esp]
	mov	[edx],al
	pop	ds
	ret
_FarByteWrite endp

; unsigned char BR_ASM_CALL FarByteRead(int offset, short seg);
;
_FarByteRead proc
	push	ds
	lds	eax,8[esp]
	mov	al,[eax]
	pop	ds
	ret
_FarByteRead endp

; void BR_ASM_CALL FarWordWrite(int offset, short seg, unsigned short value);
;
_FarWordWrite proc
	push	ds
	lds	edx,8[esp]
	mov	ax,16[esp]
	mov	[edx],ax
	pop	ds
	ret
_FarWordWrite endp

; unsigned short BR_ASM_CALL FarWordRead(int offset, short seg);
;
_FarWordRead proc
	push	ds
	lds	eax,8[esp]
	mov	ax,[eax]
	pop	ds
	ret
_FarWordRead endp

; void BR_ASM_CALL FarDWordWrite(int offset, short seg, unsigned long value);
;
_FarDWordWrite proc
	push	ds
	lds	edx,8[esp]
	mov	eax,16[esp]
	mov	[edx],eax
	pop	ds
	ret
_FarDWordWrite endp

; unsigned long BR_ASM_CALL FarDWordRead(int offset, short seg);
;
_FarDWordRead proc
	push	ds
	lds	eax,8[esp]
	mov	eax,[eax]
	pop	ds
	ret
_FarDWordRead endp

	end

