@echo off
rem Build A Watcom Library Targeted to Visual C distribution
rem
setlocal
set MAKE_EXT=.vtc
set EXTENDER=PHARLAP
set BUILD=RELEASE

rem Build libraries
rem

set BASE_TYPE=FIXED
call makelibs %1 %2 %3 %4 %5 %6 %7 %8 %9

endlocal


