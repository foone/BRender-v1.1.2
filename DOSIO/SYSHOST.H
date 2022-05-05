/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: syshost.h 1.4 1995/07/28 18:58:54 sam Exp $
 * $Locker:  $
 *
 * Various platform specific defines
 */
#ifndef _SYSHOST_H_
#define _SYSHOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
/*
 * Define the regsiter macros - makes life a lot easier
 */
union BR_REGS {
  struct {
    unsigned long eax;
    unsigned long ebx;
    unsigned long ecx;
    unsigned long edx;
    unsigned long esi;
    unsigned long edi;
    unsigned long cflag;
    unsigned long eflags;
  } e;
  struct {
    unsigned short ax; unsigned short upper_ax;
    unsigned short bx; unsigned short upper_bx;
    unsigned short cx; unsigned short upper_cx;
    unsigned short dx; unsigned short upper_dx;
    unsigned short si; unsigned short upper_si;
    unsigned short di; unsigned short upper_di;
    unsigned long cflag;
    unsigned short flags; unsigned short upper_flags;
  } x;
  struct {
    unsigned char al;
    unsigned char ah;
    unsigned short upper_ax;
    unsigned char bl;
    unsigned char bh;
    unsigned short upper_bx;
    unsigned char cl;
    unsigned char ch;
    unsigned short upper_cx;
    unsigned char dl;
    unsigned char dh;
    unsigned short upper_dx;
  } h;
};

struct BR_SREGS {
	unsigned short es;
	unsigned short cs;
	unsigned short ss;
	unsigned short ds;
};
#pragma pack()

#define REGS_TYPE union BR_REGS
#define SREGS_TYPE struct BR_SREGS

#define WREGS x
#define EREGS e
#define HREGS h

/**
 ** GNU C - Tested with 2.3.3 & DJGPP environment (GO32)
 **/
#ifdef __GNUC__

#include <unistd.h>
#include <graphics.h>
#include <pc.h>
#include <dos.h>

#define KBHIT	kbhit
#define GETKEY	getkey
#define INT86(i,ri,ro)		int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))
#define outp	outportb

#define HAS_ENV_H	0
#define HAS_SETENV	1

/**
 ** WATCOM C 9.5 and 10.0 targetted to Visual C++ 2.0
 **/

#elif __WATCOMC__ && defined(__TARGET_MSC__)

#define SUFFIX_HOST "-VTC"

/*
 * Stop unreferenced variables producing a warning
 * Things like "rcsid" and unused fucntion arguments
 */
#pragma off (unreferenced);

#define KBHIT() 	_bios_keybrd(1)
#define GETKEY()	_bios_keybrd(0)

int __cdecl _int86(int intnum, union _REGS *inregs, union _REGS *outregs);
int __cdecl _int86x(int intnum, union _REGS *inregs, union _REGS *outregs,struct _SREGS *segregs);

#if 0
int __cdecl _inp(unsigned);
unsigned __cdecl _inpw(unsigned);
unsigned long  __cdecl _inpd(unsigned);
int __cdecl _outp(unsigned, int);
unsigned  __cdecl _outpw(unsigned, unsigned);
unsigned long __cdecl _outpd(unsigned, unsigned long);
#endif

#define inp(p) _inp(p)
#define inpw(p) _inpw(p)
#define inpd(p) _inpd(p)
#define outp(p,v) _outp(p,v)
#define outpw(p,v) _outpw(p,v)
#define outpd(p,v) _outpd(p,v)

#define INT86(i,ri,ro)		_int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	_int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define HAS_ENV_H	0
#define HAS_SETENV	0

/**
 ** WATCOM C 9.5 and 10.0
 **/

#elif __WATCOMC__

#include <bios.h>

#define KBHIT() _bios_keybrd(_KEYBRD_READY)
#define GETKEY()_bios_keybrd(_KEYBRD_READ)

#define INT86(i,ri,ro)		int386(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int386x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define DISPLAY_QUAL

#define HAS_ENV_H	1
#define HAS_SETENV	1

/**
 ** Intel Proton
 **/
#elif __PROTONC__

#include <bios.h>

#define KBHIT() _bios_keybrd(_KEYBRD_READY)
#define GETKEY()_bios_keybrd(_KEYBRD_READ)
#define INT86(i,ri,ro)		int386(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int386x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define HAS_ENV_H	1
#define HAS_SETENV	1

/**
 ** Microsoft Visual C++
 **/
#elif _MSC_VER

#if defined(__PHARLAP386__)
#include <pldos32.h>
#endif

#define KBHIT() _bios_keybrd(1)
#define GETKEY()_bios_keybrd(0)
#define INT86(i,ri,ro)		_int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	_int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define HAS_ENV_H	0
#define HAS_SETENV	0

/**
 ** Zortech 3.1 
 **/
#elif __ZTC__

#include <bios.h>

#define KBHIT() bioskey(1)
#define GETKEY() bioskey(0)

#define INT86(i,ri,ro)		int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define HAS_ENV_H	0
#define HAS_SETENV	0

/*
 * Metaware High-C Version 1
 */
#elif __HIGHC_V1__

#define KBHIT() _bios_keybrd(1)
#define GETKEY()_bios_keybrd(0)

#define INT86(i,ri,ro)		int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define HAS_ENV_H	0
#define HAS_SETENV	0

/*
 * Metaware High-C Version 3
 */
#elif __HIGHC__

#include <bios.h>

#define KBHIT() _bios_keybrd(1)
#define GETKEY()_bios_keybrd(0)

#define INT86(i,ri,ro)		int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#define HAS_ENV_H	0
#define HAS_SETENV	0

/**
 ** Borland C
 **/
#elif __BORLANDC__

#if defined(__PHARLAP386__)
#include <pldos32.h>

#define INT86(i,ri,ro)		_int86(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	_int86x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#endif

#if defined(__POWERPACK__)

#define INT86(i,ri,ro)		int386(i,(void *)(ri),(void *)(ro))
#define INT86X(i,ri,ro,sr)	int386x(i,(void *)(ri),(void *)(ro),(void *)(sr))

#endif


#define KBHIT() _bios_keybrd(1)
#define GETKEY()_bios_keybrd(0)

/*
 * Borland C has bugged builtin port fns.
 *
 * (the port number is truncated to a byte)
 */
#undef outp
#undef outpw
#undef inp
#undef inpw

#define HAS_ENV_H	0
#define HAS_SETENV	0
#endif

#ifdef __cplusplus
};
#endif
#endif

