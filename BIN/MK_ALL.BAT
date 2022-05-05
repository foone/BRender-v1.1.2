@echo off
rem Build All Libraries
rem
setlocal

rem Watcom
rem
call cwat10.bat
call mk_wtc.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
call mk_vtc.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

rem Microsoft
rem
call cmvc.bat
call mk_mvc.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

rem Borland
rem 
call cbc45.bat
call mk_bcc.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

endlocal
