@echo off
rem Build Borland C++ 4.5 Libraries
setlocal
set BUILD=RELEASE

set EXTENDER=POWERPACK

rem Build release libraries
rem

set BASE_TYPE=FIXED
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

set BASE_TYPE=FLOAT
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

rem Build different extender versions of dosio
rem
cd dosio

make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9

set EXTENDER=PHARLAP
make -f makefile %1 %2 %3 %4 %5 %6 %7 %8 %9

endlocal

