;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: segregs.asm 1.2 1995/02/22 22:07:15 sam Exp $
;; $Locker:  $
;;
	.386p
	.model flat

	.data
		db '$Id: segregs.asm 1.2 1995/02/22 22:07:15 sam Exp $',0
		align 4
	.code

; unsigned short ASM_CALL GetDS(void);
;
		public _GetDS
_GetDS proc
		mov	ax,ds
		ret
_GetDS endp

; unsigned short ASM_CALL GetES(void);
;
		public _GetES
_GetES proc
		mov	ax,ds
		ret
_GetES endp

; unsigned short ASM_CALL GetCS(void);
;
		public _GetCS
_GetCS proc
		mov	ax,cs
		ret
_GetCS endp

; unsigned short ASM_CALL GetSS(void);
;
		public _GetSS
_GetSS proc
		mov	ax,ss
		ret
_GetSS endp

	end

