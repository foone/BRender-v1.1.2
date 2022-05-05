;; Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
;;
;; $Id: realsupt.asm 1.3 1995/09/07 15:19:42 sam Exp $
;; $Locker: sam $
;;
;; Support routines for digging around with real mode and interrupts
;;
	.386p
	.model flat,c

	include dpmi.inc

; Structure for describing a block of real mode memory
;
dosio_real_memory struc
pm_off		dword	?
pm_seg		word	?

rm_off		word	?
rm_seg		word	?
dosio_real_memory ends

; Registers structure from C
;
br_regs	struc
r_eax		dword	?
r_ebx		dword	?
r_ecx		dword	?
r_edx		dword	?
r_esi		dword	?
r_edi		dword	?
r_cflag		dword	?
r_eflags	dword	?
br_regs	ends

br_sregs	struc
s_es		word	?
s_cs		word	?
s_ss		word	?
s_ds		word	?
br_sregs	ends

pl_registers	struc
interrupt	word	?
rds		word	?
res		word	?
rfs		word	?
rgs		word	?
reax		dword	?
redx		dword	?
pl_registers	ends

	.data
		db '$Id: realsupt.asm 1.3 1995/09/07 15:19:42 sam Exp $',0
		align 4
	.code

; br_uint_32 BR_ASM_CALL _RealAllocate(dosio_real_memory *mem, br_uint_32 size);
;
_RealAllocate proc uses ebx ecx, drm: ptr dosio_real_memory, msize:dword

		mov	ebx,msize
		add	ebx,15
		shr	ebx,4

ifdef __DPMI__
		mov	eax,0100h		; Allocate DOS Memory
		int	031h			; DPMI
		jc	error

		mov	ebx,drm
		mov	[ebx].dosio_real_memory.rm_seg,ax
		mov	[ebx].dosio_real_memory.pm_seg,dx

		xor	eax,eax
		mov	[ebx].dosio_real_memory.pm_off,eax
		mov	[ebx].dosio_real_memory.rm_off,ax

elseifdef __PHARLAP386__

		mov	eax,25C0h		; Allocate Real Memory
		int	021h
		jc	error

		mov	ebx,drm
		mov	[ebx].dosio_real_memory.rm_seg,ax

		shl	eax,4
		mov	[ebx].dosio_real_memory.pm_off,eax

		mov	ax,cs			; Get RPL from CS
		and	ax,3
		or	ax,034h			; OR into hardwired LDT selector
		mov	[ebx].dosio_real_memory.pm_seg,ax

		xor	eax,eax
		mov	[ebx].dosio_real_memory.rm_off,ax
elseifdef __X32__

		mov	eax,4800h		; Allocate Real Memory
		int	021h
		jc	error

		mov	ecx,drm
		mov	[ecx].dosio_real_memory.rm_seg,ax
		xor	ebx,ebx
		mov	[ecx].dosio_real_memory.rm_off,bx

if 0
		mov	[ecx].dosio_real_memory.pm_seg,ds
		mov	[ecx].dosio_real_memory.pm_off,ebx
else
		mov	bx,_x386_zero_base_selector
		mov	[ecx].dosio_real_memory.pm_seg,bx

		shl	eax,4
		mov	[ecx].dosio_real_memory.pm_off,eax
endif

endif
		xor	eax,eax
		ret
error:
		mov	eax,1
		ret

_RealAllocate endp

; void BR_ASM_CALL _RealFree(dosio_real_memory *mem);
;
_RealFree proc uses ebx, drm: ptr dosio_real_memory

ifdef __DPMI__
		xor	edx,edx
		mov	ebx,drm
		mov	dx,[ebx].dosio_real_memory.pm_seg
		mov	eax,0101h		; Free DOS Memory
		int	31h			; DPMI

elseifdef __PHARLAP386__

		xor	ecx,ecx
		mov	ebx,drm
		mov	cx,[ebx].dosio_real_memory.rm_seg
		mov	eax,25C1h
		int	21h

elseifdef __X32__
		; X-32 cannot release real-mode memeory!
endif
		ret
_RealFree endp

; void BR_ASM_CALL _RealLock(dosio_real_memory *mem, br_uint_32 size);
;
_RealLock proc uses ebx, drm: ptr dosio_real_memory, msize: dword

ifdef __DPMI__
		mov	eax,drm
		mov	bx,word ptr [eax].dosio_real_memory.pm_seg
		mov	eax,0006h		; Get Segment Base Address
		int	31h			; DPMI
						; cx:dx
		mov	bx,cx
		mov	cx,dx
		mov	edi,msize
		mov	esi,edi
		shr	edi,16

		mov	eax,0600h		; Lock region
		int	31h			; DPMI

elseifdef __PHARLAP386__
		mov	eax,drm
		mov	bx,word ptr [eax].dosio_real_memory.pm_seg
		mov	eax,2508h		; Get segment base
		int	21h

		mov	eax,drm
		mov	ebx,0500h
		mov	edx,msize
		mov	eax,252Bh		; Lock region
		int	21h
elseifdef __X32__
		mov	eax,drm
		mov	bx,word ptr [eax].dosio_real_memory.pm_seg
		mov	eax,2508h		; Get segment base
		int	21h

		mov	eax,drm
		mov	ebx,0500h
		mov	edx,msize
		mov	eax,252Bh		; Lock region
		int	21h
endif
		ret
_RealLock endp

; void BR_ASM_CALL _RealUnlock(dosio_real_memory *mem, br_uint_32 size);
;
_RealUnlock proc uses ebx, drm: ptr dosio_real_memory, msize: dword

ifdef __DPMI__
		mov	eax,drm
		mov	bx,word ptr [eax].dosio_real_memory.pm_seg
		mov	eax,0006h		; Get Segment Base Address
		int	31h			; DPMI
						; cx:dx

		mov	bx,cx
		mov	cx,dx
		mov	edi,msize
		mov	esi,edi
		shr	edi,16

		xor	edx,edx
		mov	eax,0601h		; Unlock region
		int	31h			; DPMI

elseifdef __PHARLAP386__

		mov	eax,drm
		mov	bx,word ptr [eax].dosio_real_memory.pm_seg
		mov	eax,2508h		; Get segment base
		int	21h

		mov	ebx,0600h
		mov	edx,msize
		mov	eax,252Bh		; Unlock region
		int	21h

elseifdef __X32__

		mov	eax,drm
		mov	bx,word ptr [eax].dosio_real_memory.pm_seg
		mov	eax,3504h		; Get segment base
		int	21h

		mov	ebx,0600h
		mov	edx,msize
		mov	eax,252Bh		; Unlock region
		int	21h
endif
		ret
_RealUnlock endp

if 1
; void BR_ASM_CALL _RealVectorGet(br_uint_8 vector, br_uint_16 *vsegp, br_uint_16 *voffp);
;
_RealVectorGet proc uses ebx, vector: dword, vsegp:ptr word, voffp:ptr word
ifdef __DPMI__
		xor	ebx,ebx
		mov	bl,byte ptr vector
		mov	eax,0200h		; Get Real Mode Interrupt Vector
		int	31h			; DPMI
		mov	ebx,vsegp
		mov	[ebx],cx
		mov	ebx,voffp
		mov	[ebx],dx

else ; PHARLAP or X-32
		xor	ecx,ecx
		mov	cl,byte ptr vector
		mov	eax,2503h		; Get Real Mode Interrupt Vector
		int	21h
		mov	eax,voffp
		mov	[eax],bx
		shr	ebx,16
		mov	eax,vsegp
		mov	[eax],bx
endif
		ret
_RealVectorGet endp

; void BR_ASM_CALL _RealVectorSet(br_uint_8 vector, br_uint_16 vseg, br_uint_16 voff);
;
_RealVectorSet proc uses ebx, vector: dword, vseg:dword, voff:dword

ifdef __DPMI__
		xor	ebx,ebx
		mov	bl,byte ptr vector
		mov	cx,word ptr vseg
		mov	dx,word ptr voff
		mov	eax,0201h		; Set Real Mode Interrupt Vector
		int	31h			; DPMI

else ; PHARLAP or X-32
		mov	cl,byte ptr vector
		mov	bx,word ptr vseg
		shl	ebx,16
		mov	bx,word ptr voff
		mov	eax,02505h		; Set Real Mode Interrupt Vector
		int	21h
endif
		ret
_RealVectorSet endp
endif

; void BR_ASM_CALL _RealVectorCall(br_uint_8 vector, union BR_REGS *ri, union BR_REGS *ro, struct BR_SREGS *sr);
;
; Invokes an interrupt in real mode
;
_RealVectorCall proc uses ebx edi es, vec:dword, regs_in: ptr br_regs, regs_out: ptr br_regs, sregs: ptr br_sregs

ifdef __DPMI__
	; Transcribe input registers
	;
		mov	eax,regs_in
		mov	ebx,[eax].br_regs.r_edi
		mov	real_regs.redi,ebx
		mov	ebx,[eax].br_regs.r_esi
		mov	real_regs.resi,ebx
		mov	ebx,[eax].br_regs.r_edx
		mov	real_regs.redx,ebx
		mov	ebx,[eax].br_regs.r_ecx
		mov	real_regs.recx,ebx
		mov	ebx,[eax].br_regs.r_ebx
		mov	real_regs.rebx,ebx
		mov	ebx,[eax].br_regs.r_eax
		mov	real_regs.reax,ebx

		mov	eax,sregs
		mov	bx,[eax].br_sregs.s_cs
		mov	real_regs.rcs,bx
		mov	bx,[eax].br_sregs.s_ds
		mov	real_regs.rds,bx
		mov	bx,[eax].br_sregs.s_es
		mov	real_regs.res,bx

	; NULL stack pointer
	;
		xor	bx,bx
		mov	real_regs.rss,bx
		mov	real_regs.rsp,bx
	
	; Use DPMI to call vector
	;
		xor	ebx,ebx
		mov	bl, byte ptr vec
		mov	ax,ds
		mov	es,ax
		mov	edi,offset real_regs
		xor	ecx,ecx
		mov	eax,300h	    	; Simulate Real Mode Interrupt
		int	31h			; DPMI

	; Transcribe output registers
	;
		mov	eax,regs_out
		mov	ebx,real_regs.redi
		mov	[eax].br_regs.r_edi,ebx
		mov	ebx,real_regs.resi
		mov	[eax].br_regs.r_esi,ebx
		mov	ebx,real_regs.redx
		mov	[eax].br_regs.r_edx,ebx
		mov	ebx,real_regs.recx
		mov	[eax].br_regs.r_ecx,ebx
		mov	ebx,real_regs.rebx
		mov	[eax].br_regs.r_ebx,ebx
		mov	ebx,real_regs.reax
		mov	[eax].br_regs.r_eax,ebx

		xor	ebx,ebx
		mov	bx,real_regs.rflags
		mov	[eax].br_regs.r_eflags,ebx
		and	ebx,1
		mov	[eax].br_regs.r_cflag,ebx

		mov	eax,sregs
		mov	bx,real_regs.rcs
		mov	[eax].br_sregs.s_cs,bx
		mov	bx,real_regs.rds
		mov	[eax].br_sregs.s_ds,bx
		mov	bx,real_regs.res
		mov	[eax].br_sregs.s_es,bx

		xor	bx,bx
		mov	[eax].br_sregs.s_ss,bx

	; XXX Flags

else ; PHARLAP or X-32
	; Transcribe input registers
	;
		mov	edx,regs_in

		mov	edi,[edx].br_regs.r_edi
		mov	esi,[edx].br_regs.r_esi
		mov	ecx,[edx].br_regs.r_ecx
		mov	ebx,[edx].br_regs.r_ebx

		mov	eax,[edx].br_regs.r_eax
		mov	real_regs.reax,eax
		mov	eax,[edx].br_regs.r_edx
		mov	real_regs.redx,eax

		mov	edx,sregs
		mov	ax,[edx].br_sregs.s_ds
		mov	real_regs.rds,ax
		mov	ax,[edx].br_sregs.s_es
		mov	real_regs.res,ax

		mov	bx, word ptr vec
		mov	real_regs.interrupt,bx

		push	ebp
		mov	eax,2511h
		mov	edx,offset real_regs
		int	21h
		pop	ebp
		mov	edx,regs_out

		xor	eax,eax
		pushf
		pop	ax
		mov	[edx].br_regs.r_eflags,eax
		and	eax,1
		mov	[edx].br_regs.r_cflag,eax

		mov	[edx].br_regs.r_edi,edi
		mov	[edx].br_regs.r_esi,esi
		mov	[edx].br_regs.r_ecx,ecx
		mov	[edx].br_regs.r_ebx,ebx

		mov	eax,real_regs.reax
		mov	[edx].br_regs.r_eax,eax
		mov	eax,real_regs.redx
		mov	[edx].br_regs.r_edx,eax

		mov	edx,sregs
		mov	ax,real_regs.rds
		mov	[edx].br_sregs.s_ds,ax
		mov	ax,real_regs.res
		mov	[edx].br_sregs.s_es,ax
		
endif
		ret
_RealVectorCall endp

; void _RealSegmentsSetup(void);
;
; Create a public selector to the real mode 1M in _real_segment
;
		public _RealSegmentsSetup
_RealSegmentsSetup proc uses ebx

		cmp	segs_setup,0
		jne	done

ifdef __DPMI__

	; Use DPMI services to create descriptors
	;

	; 0000
	;
		mov	eax,2
		mov	ebx,00000h
		int	31h
		mov	_RealSegment0000,ax

	; A000
	;
		mov	eax,2
		mov	ebx,0A000h
		int	31h
		mov	_RealSegmentA000,ax

elseifdef __PHARLAP386__

		mov	ax,cs			; Get RPL from CS
		and	ax,3
		or	ax,034h			; OR into hardwired LDT selector
		mov	_RealSegment0000,ax

elseifdef __X32__

		mov	ax,_x386_zero_base_selector
		mov	_RealSegment0000,ax
endif

		mov	segs_setup,1
done:
		ret
_RealSegmentsSetup endp

	.data

	public _RealSegment0000
	public _RealSegmentA000

_RealSegment0000	dw	0
_RealSegmentA000	dw	0

segs_setup	dw	0

ifdef __DPMI__
real_regs	dpmi_registers	<,>
else
real_regs	pl_registers	<,>
endif

ifdef __X32__
		extrn _x386_zero_base_selector:word
endif

	end
