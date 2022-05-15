@echo off
cd fw
dmake %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\dosio
dmake %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\zb
dmake %1 %2 %3 %4 %5 %6 %7 %8 %9
REM cd ..\zs
REM dmake %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\fmt
dmake %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\std
dmake %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..
