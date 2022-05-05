@echo off
rem Build Microsoft Visual C++ 4.5 Libraries
setlocal
set BUILD=RELEASE
set EXTENDER=PHARLAP

rem Build libraries
rem

set BASE_TYPE=FIXED
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

set BASE_TYPE=FLOAT
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

endlocal

