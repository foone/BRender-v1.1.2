# Copyright (c) 1993 Argonaut Software Ltd. All rights reserved.
#
# $Id: makedefs.pro 1.4 1995/03/16 12:06:25 sam Exp $
# $Locker: sam $
#
# General defines, rules and macros for BRender built with Intel Proton
#

# Pick up configuration from environment, if not defined, pick defaults
#
.IMPORT .IGNORE: LONG_ARGS

# Type of executable to produce - debug, profile, or release
#
.IF $(BUILD) ==
.IMPORT .IGNORE: BUILD 
.ENDIF

BUILD*:=RELEASE

# Target type
#
.IF $(TARGET_TYPE) ==
.IMPORT .IGNORE: TARGET_TYPE 
.END

TARGET_TYPE*:=LIB

# Extender
#
.IF $(EXTENDER) ==
.IMPORT .IGNORE: EXTENDER 
.END

EXTENDER*:=DOS4GW

# Library base type
#
.IF $(BASE_TYPE) ==
.IMPORT .IGNORE: BASE_TYPE 
.ENDIF

BASE_TYPE*:=FIXED

# Compiler part of library name - Proton
#
LIB_SUFFIX_C=p

# Pick a library suffix based on library type
#
.IF   $(BASE_TYPE) == FLOAT
LIB_SUFFIX_T=f
.ELIF $(BASE_TYPE) == FIXED
LIB_SUFFIX_T=x
.ELIF $(BASE_TYPE) == NONE
LIB_SUFFIX_T=
.ELSE
Unknown_BASE_TYPE:
.ENDIF

## Build flags
##
OBJ_EXT:=.obj
LIB_EXT:=.lib
AS:=ml
CC:=icc
H2I:=h2inc
MKDEPFLAGS= -s -t -I$(BLD_DIR) $(INCLUDES)

BASE_CFLAGS  =-Wp,-_m -ip -mem -w -Zp4 -DBASED_$(BASE_TYPE)=1\
 $(INCLUDES) -D__PROTONC__

BASE_ASFLAGS =-nologo -Cp -D__PROTONC__=1 \
 -DBASED_$(BASE_TYPE)=1 -I$(BLD_DIR) $(INCLUDES)

BASE_H2IFLAGS=/nologo /G3 /Zp4 /C /D__PROTONC__  /D__H2INC__\
 /D_NO_PROTOTYPES -DBASED_$(BASE_TYPE)=1 $(INCLUDES)

.IF   $(BUILD) == RELEASE
# Release Flags
#
BUILD_CFLAGS  =-DDEBUG=0 -DPARANOID=0 -DSTATIC=static
BUILD_LDFLAGS =$(RELEASE_LDFLAGS)
BUILD_ASFLAGS =
BUILD_H2IFLAGS=
LIB_SUFFIX_B:=r

.ELIF $(BUILD) == TEST
# Check Flags (Release library with validation checks for debugging user code)
#
BUILD_CFLAGS  =-DDEBUG=0 -DPARANOID=1 -DSTATIC=static
BUILD_LDFLAGS =$(TEST_LDFLAGS)
BUILD_ASFLAGS =-DDEBUG=0 -DCHECK=1
BUILD_H2IFLAGS=-DDEBUG=0 -DCHECK=1
LIB_SUFFIX_B:=t


.ELIF $(BUILD) == DEBUG
# Debug Flags
#
BUILD_CFLAGS  =-DDEBUG=1 -DPARANOID=1 -DSTATIC=
BUILD_LDFLAGS =$(DEBUG_LDFLAGS)
BUILD_ASFLAGS =-Zd
BUILD_H2IFLAGS=
LIB_SUFFIX_B:=d

.ELIF $(BUILD) == PROFILE
# Profile Flags
#
BUILD_CFLAGS  =-DDEBUG=0 -DPARANOID=0 -DSTATIC=
BUILD_LDFLAGS =$(PROFILE_LDFLAGS)
BUILD_ASFLAGS =-Zd
BUILD_H2IFLAGS=
LIB_SUFFIX_B:=p

.ELSE
Error - Unknown build type
.ENDIF

## The build directory
##
LIB_TYPE:=$(LIB_SUFFIX_C)$(LIB_SUFFIX_T)$(LIB_SUFFIX_B)$(LIB_SUFFIX)

BLD_BASE*:=bld

BLD_DIR:=$(BLD_BASE)_$(LIB_TYPE)

## The library name <part><compiler><type><build><optional>.lib
##
LIB_NAME:=$(BASE_DIR)/lib/$(LIB_PART)$(LIB_TYPE).lib

# Keep proper dependencies (per build type)
#
.KEEP_STATE:=$(BLD_DIR)/state.out

## Generic rule for building objects and ASM header files
##
.IF $(LONG_ARGS)X != X

	# Use environment variables to cut down the cmmand line size
	#
$(BLD_DIR)/%.obj : %.c
	@echo Compiling $<
	$(CC) @$(mktmp,icc.rsp $< -c $(BASE_CFLAGS:s,\,\\,) $(BUILD_CFLAGS) $(EXTRA_CFLAGS) -o $@\n)

$(BLD_DIR)/%.obj : %.asm
	@[
	set ML=$(BASE_ASFLAGS) $(BUILD_ASFLAGS) $(EXTRA_ASFLAGS) -c -Fo$(@:s,/,\,) $<
	$(AS)
	]

%.obj : %.c
	$(CC) @$(mktmp,icc.rsp $< -c -o $@ $(BASE_CFLAGS:s,\,\\,) $(BUILD_CFLAGS) $(EXTRA_CFLAGS)\n)

%.pp : %.c
	$(CC) @$(mktmp,icc.rsp -E $< $(BASE_CFLAGS:s,\,\\,) $(BUILD_CFLAGS) $(EXTRA_CFLAGS)\n) >$@

%.cod : %.c
	$(CC) @$(mktmp,icc.rsp $< -o $@ -S $(BASE_CFLAGS:s,\,\\,) $(BUILD_CFLAGS) $(EXTRA_CFLAGS)\n)

%.obj : %.asm
	@[
	set ML=$(BASE_ASFLAGS) $(BUILD_ASFLAGS) $(EXTRA_ASFLAGS) -Fo$(@:s,/,\,) $<
	$(AS)
	]

$(BLD_DIR)/%.inc : %.h
	@[
	set H2INC=$(BASE_H2IFLAGS) $(BUILD_H2IFLAGS) $(EXTRA_H2IFLAGS) /Fa$@ $<
	$(H2I) 
	]

$(BLD_DIR)\%.inc : %.h
	@[
	set H2INC=$(BASE_H2IFLAGS) $(BUILD_H2IFLAGS) $(EXTRA_H2IFLAGS) /Fa$@ $<
	$(H2I) 
	]
.ELSE
	# Pass argumants normally
	#
$(BLD_DIR)/%.obj : %.c
	$(CC) $(BASE_CFLAGS) $(BUILD_CFLAGS) $(EXTRA_CFLAGS) -fo$(@:s,/,\,) $<

$(BLD_DIR)/%.obj : %.asm
	$(AS) $(BASE_ASFLAGS) $(BUILD_ASFLAGS) $(EXTRA_ASFLAGS) -c -Fo$(@:s,/,\,) $<

%.obj : %.c
	$(CC) $(BASE_CFLAGS) $(BUILD_CFLAGS) $(EXTRA_CFLAGS) -fo$(@:s,/,\,) $<

%.obj : %.asm
	$(AS) $(BASE_ASFLAGS) $(BUILD_ASFLAGS) $(EXTRA_ASFLAGS) -c -Fo$(@:s,/,\,) $<

$(BLD_DIR)/%.inc : %.h
	$(H2I) $(BASE_H2IFLAGS) $(BUILD_H2IFLAGS) $(EXTRA_H2IFLAGS) /Fa$@ $<

$(BLD_DIR)\%.inc : %.h
	$(H2I) $(BASE_H2IFLAGS) $(BUILD_H2IFLAGS) $(EXTRA_H2IFLAGS) /Fa$@ $<

.ENDIF

## The final target
##
OBJS := $(OBJS_C) $(OBJS_ASM)
LDLIBS=$(BASE_LDLIBS) $(EXTRA_LDLIBS)

.IF $(TARGET_TYPE) == LIB

# Library
#
"$(LIB_NAME)": $(OBJS)
	-rm -f $@
	wlib -n -q $(@:s,/,\\) @$(mktmp,lib.rsp +$(<:t" +":s,\,\\,:s,/,\\)\n)

.ELIF $(TARGET_TYPE) == EXE_DOS4GW

.ELSE

.IF $(EXTENDER) == DOS4GW

# Executable - DOS4GW
#
BASE_LDFLAGS:=option dosseg,caseexact,quiet,stack=128k
DEBUG_LDFLAGS:=debug all
PROFILE_LDFLAGS:=debug all
RELEASE_LDFLAGS:=
TEST_LDFLAGS:=

BASE_LDLIBS=math387s.lib emu387.lib clib3s.lib 

LIB_SUFFIX_EXTENDER:=4g

$(TARGET): $(EXTRA_LDLIBS)
$(TARGET): $(OBJS)
	wlink @$(mktmp,link.rsp name $@\n\
system dos4g\n\
$(BASE_LDFLAGS)\n\
$(BUILD_LDFLAGS)\n\
$(EXTRA_LDFLAGS)\n\
file wcrt0.obj\n\
file $(<:t"\nfile ":s,\,/,)\n\
library $(LDLIBS:t"\nlibrary ":s,\,/,)\n\
)

.ELIF $(EXTENDER) == DOS4GWPRO

# Executable - DOS4GW Professional
#
BASE_LDFLAGS:=option dosseg,caseexact,quiet,stack=128k
DEBUG_LDFLAGS:=debug all
PROFILE_LDFLAGS:=debug all
RELEASE_LDFLAGS:=
TEST_LDFLAGS:=

BASE_LDLIBS=

LIB_SUFFIX_EXTENDER:=4g

$(TARGET): $(EXTRA_LDLIBS)
$(TARGET): $(OBJS)
	wlink @$(mktmp,link.rsp name _tmp.exe\n\
system dos4g\n\
$(BASE_LDFLAGS)\n\
$(BUILD_LDFLAGS)\n\
$(EXTRA_LDFLAGS)\n\
file wcrt0.obj\n\
file $(<:t"\nfile ":s,\,/,)\n\
library $(LDLIBS:t"\nlibrary ":s,\,/,)\n\
)
	4gwbind 4gwpro.exe _tmp.exe $(TARGET) -Q
	rm _tmp.exe

.ELIF $(EXTENDER) == X32

# Executable - Flashtek X-32
#
BASE_LDFLAGS:=option dosseg,caseexact,quiet
DEBUG_LDFLAGS:=debug all
PROFILE_LDFLAGS:=debug all
RELEASE_LDFLAGS:=
TEST_LDFLAGS:=

BASE_LDLIBS=

LIB_SUFFIX_EXTENDER:=x3

$(TARGET): $(EXTRA_LDLIBS)
$(TARGET): $(OBJS)
	wlink @$(mktmp,link.rsp name $@\n\
$(BASE_LDFLAGS)\n\
$(BUILD_LDFLAGS)\n\
$(EXTRA_LDFLAGS)\n\
system x32$(LIB_SUFFIX)\n\
file wcrt0.obj\n\
file $(<:t"\nfile ":s,\,/,)\n\
library $(LDLIBS:t"\nlibrary ":s,\,/,)\n\
)
	x32fix $(TARGET)

.ELIF $(EXTENDER) == PHARLAP

# Executable - Pharlap TNT
#
BASE_LDFLAGS:=option dosseg,caseexact,quiet
DEBUG_LDFLAGS:=debug all
PROFILE_LDFLAGS:=debug all
RELEASE_LDFLAGS:=
TEST_LDFLAGS:=

BASE_LDLIBS=

LIB_SUFFIX_EXTENDER:=pl

$(TARGET): $(EXTRA_LDLIBS)
$(TARGET): $(OBJS)
	wlink @$(mktmp,link.rsp name $@\n\
$(BASE_LDFLAGS)\n\
$(BUILD_LDFLAGS)\n\
$(EXTRA_LDFLAGS)\n\
system tnt\n\
file wcrt0.obj\n\
file $(<:t"\nfile ":s,\,/,)\n\
library $(LDLIBS:t"\nlibrary ":s,\,/,)\n\
)
	rebind $(TARGET)

.ELSE
Unknown_EXTENDER:
.ENDIF
.ENDIF

## Object directory
##
$(BLD_DIR)/exists:
	-mkdir $(BLD_DIR)
	echo $(BLD_DIR) >$(BLD_DIR)\exists
.IF $(LIB_INC)
	%$(MAKE) $(MAKEFILE) $(BLD_DIR)/$(LIB_INC).inc
.ENDIF



# Dependencies
#
SRCS:=$(OBJS_C:s/.obj/.c/) $(OBJS_ASM:s/.obj/.asm/)

depend: $(BLD_DIR)/exists
	
depend:
	sed "s/^$(BLD_DIR)\///" <$(mktmp,_mkdep.tmp $(SRCS:t"\n":s,\,/,)\n) >_mkdep.rsp
	$(MKDEP) $(MKDEPFLAGS) @_mkdep.rsp -o_depend.tmp
	sed "s/^/$(BLD_DIR)\//" <_depend.tmp >$(BLD_DIR)\\depend.out
	$(RM) _mkdep.rsp
	$(RM) _depend.tmp

.INCLUDE .IGNORE: $(BLD_DIR)/depend.out

