;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: fxadc386.asm 1.4 1995/05/25 13:23:57 sam Exp $
;; $Locker:  $
;;
fixed typedef dword

	.386p
	.model flat,c

	.data
		db '$Id: fxadc386.asm 1.4 1995/05/25 13:23:57 sam Exp $',0
	.code

; br_fixed_ls BR_ASM_CALL BrFixedAddCarry(br_fixed_ls a, br_fixed_ls b, char * flag);
;
; return a+b and return flag if carry from bit 15
;

BrFixedAddCarry proc uses ebx edx, param_a: fixed, param_b: fixed, param_flag: ptr byte
		mov	ebx,param_flag
		xor	eax,eax
		mov	byte ptr [ebx],0

		mov	ax,word ptr param_a
		mov	dx,word ptr param_a+2

		add	ax,word ptr param_b
		jnc	no_carry
		mov	byte ptr [ebx],-1
no_carry:
		adc	dx,word ptr param_b+2
		shl	edx,16
		or	eax,edx
		ret
BrFixedAddCarry endp


	end

