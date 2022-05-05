# Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
#
# $Id: mkshades.wat 1.2 1995/02/22 22:15:40 sam Exp $
# $Locker:  $
#
# WMAKE makefile for Q3D test harness built with WATCOM C32
#
TARGET=mkshades.exe

BASE_DIR=..

# Watcom, Fixed, Release, Register calling
#
CALL_TYPE=r
LIB_TYPE=wxrr
BASED=FIXED
MODEL=REGISTER

CC=wcc386
CFLAGS=-omaxnet -zp4 -5$(CALL_TYPE) -fp3&
 -DMODEL_$(MODEL)=1&
 -DBASED_$(BASED)=1&
 -I$(BASE_DIR)\inc&

OBJS=&
	mkshades.obj

LIBS=&
	$(BASE_DIR)/lib/brst$(LIB_TYPE).lib&
	$(BASE_DIR)/lib/brfw$(LIB_TYPE).lib&
	$(BASE_DIR)/lib/wargstd$(CALL_TYPE).lib

# Default rules
#
# Cope with long command lines
#
.c.obj:
	set _ARGS=$(CFLAGS) $*.c
	$(CC) @_ARGS

$(TARGET): $(OBJS) wlink.rsp
	wlink @wlink.rsp

# Link response file
#
wlink.rsp: mkshades.wat
	echo name $(TARGET) >wlink.rsp
	echo system dos4g >>wlink.rsp
	echo option dosseg,caseexact,quiet,stack=64k >>wlink.rsp
	for %i in ($(OBJS)) do echo file %i >>wlink.rsp
	for %i in ($(LIBS)) do echo library %i >>wlink.rsp


