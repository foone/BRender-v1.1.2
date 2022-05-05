@echo off
rem Build the tools
setlocal
cd tools
set TARGET_TYPE=EXE
set EXTENDER=X32VM
set BASE_TYPE=FIXED
set BUILD=RELEASE
make -f geoconv.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
make -f mkshades.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
make -f mkranges.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
make -f texconv.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
make -f 3ds2br.mak %1 %2 %3 %4 %5 %6 %7 %8 %9

set EXTENDER=DOS4GW

make -f brview.mak %1 %2 %3 %4 %5 %6 %7 %8 %9

endlocal

