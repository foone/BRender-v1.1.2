@echo off
rem Build all the samples
setlocal

cd samples\dos\brview
del *.obj
del *.exe
del wlink.rsp
wmake -f makefile.dst

cd ..\fly
del *.obj
del *.exe
del wlink.rsp
wmake -f makefile.dst

cd ..\minimal
del *.obj
del *.exe
del wlink.rsp
wmake -f makefile.dst

cd ..\rdemo
del *.obj
del *.exe
del wlink.rsp
wmake -f makefile.dst

cd ..\sheet
del *.obj
del *.exe
del wlink.rsp
wmake -f makefile.dst

cd ..\..\win\simpview
del *.obj
del *.exe
del *.res
del wlink.rsp
wmake -f makefile.wat

endlocal

