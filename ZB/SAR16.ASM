;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: sar16.asm 1.3 1995/05/25 13:25:14 sam Exp $
;; $Locker:  $
;;
;; Out of line call for gauranteed SAR
;;
	.386p
	.model flat
	.code
__sar16	proc
	mov	eax,4[esp]
	sar	eax,16
	ret
__sar16	endp

	end

