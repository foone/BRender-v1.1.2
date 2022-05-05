@echo off
rem Build Watcom Libraries
rem

setlocal
set BUILD=RELEASE
set MAKE_EXT=.wtc

set EXTENDER=DOS4GW

rem Build libraries
rem

set BASE_TYPE=FIXED

set LIB_SUFFIX=r
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

set LIB_SUFFIX=s
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

set BASE_TYPE=FLOAT

set LIB_SUFFIX=r
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

set LIB_SUFFIX=s
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

rem Build different extender versions of dosio
rem
cd dosio

set EXTENDER=X32VM

set LIB_SUFFIX=r
make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9
set LIB_SUFFIX=s
make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9

set EXTENDER=PHARLAP

set LIB_SUFFIX=r
make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9
set LIB_SUFFIX=s
make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9

cd ..

rem	Build demos
rem 
rem set BASE_TYPE=FIXED
rem cd demos\wld
rem make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9
rem cd ..\..
rem cd demos\tdemo
rem make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9
rem cd ..\..
endlocal

