@echo off
cd fw
make %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\dosio
make %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\zb
make %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\zs
make %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\fmt
make %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\std
make %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..
