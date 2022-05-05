;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: realseg.asm 1.3 1995/02/22 22:07:13 sam Exp $
;; $Locker:  $
;;
	.386p
	.model flat,c

	.data
		db '$Id: realseg.asm 1.3 1995/02/22 22:07:13 sam Exp $',0
		align 4
	.data

	public _DOSSegment0000
	public _DOSSegmentA000

_DOSSegment0000	dw	0
_DOSSegmentA000	dw	0

segs_setup	dw	0

ifdef __X32__
		extrn _x386_zero_base_selector:word
endif

	.code

; void SetupRealSegments(void);
;
; Create a public selector to the real mode 1M in _real_segment
;
		public _DOSRealSegmentsSetup
_DOSRealSegmentsSetup proc uses ebx

		cmp	segs_setup,0
		jne	done

ifdef __DPMI__

	; Use DPMI services to create descriptors
	;

if 1
	; 0000
	;
		mov	eax,2
		mov	ebx,00000h
		int	31h
		mov	_DOSSegment0000,ax

	; A000
	;
		mov	eax,2
		mov	ebx,0A000h
		int	31h
		mov	_DOSSegmentA000,ax
endif

elseifdef __PHARLAP386__

		mov	ax,cs			; Get RPL from CS
		and	ax,3
		or	ax,034h			; OR into hardwired LDT selector
		mov	_DOSSegment0000,ax

elseifdef __X32__

		mov	ax,_x386_zero_base_selector
		mov	_DOSSegment0000,ax
endif

		mov	segs_setup,1
done:
		ret
_DOSRealSegmentsSetup endp

	end

