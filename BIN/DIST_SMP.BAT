@echo off
setlocal
rem Copy Samples to dist. directory
rem
set DIST_DIR=%1
mkdir %DIST_DIR%\samples
mkdir %DIST_DIR%\samples\dos
mkdir %DIST_DIR%\samples\win

mkdir %DIST_DIR%\samples\dos\minimal
copy samples\dos\minimal\minimal.exe    %DIST_DIR%\samples\dos\minimal
copy samples\dos\minimal\minimal.c      %DIST_DIR%\samples\dos\minimal
copy samples\dos\minimal\std.pal		%DIST_DIR%\samples\dos\minimal
copy samples\dos\minimal\makefile.msc	%DIST_DIR%\samples\dos\minimal
copy samples\dos\minimal\makefile.bcc	%DIST_DIR%\samples\dos\minimal
copy samples\dos\minimal\makefile.wat	%DIST_DIR%\samples\dos\minimal

mkdir %DIST_DIR%\samples\dos\bis
copy samples\dos\bis\ast.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\barrel.pix         %DIST_DIR%\samples\dos\bis
copy samples\dos\bis\barrier.pix        %DIST_DIR%\samples\dos\bis
copy samples\dos\bis\brender.pix		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\bug-bod.pix		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\butt.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\earth2.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\flame.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\jupiter2.pix		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\lights.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\logo.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\rock.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\rosewood.pix		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\s-arm.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\s-body.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\s-helmet.pix		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\barrier.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\bullet.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\cube.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\game.exe			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\dish.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\disk.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\earth.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\flame.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\frame.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\lightbox.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\porthole.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\sp-ship.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\star.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\bis.mat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\s-hip.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\s-leg.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\shield.pix			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\wplastic.pix		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\arrow.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\game.pal			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\game.tab			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\aster.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\aster2.dat			%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\astrdm0.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\astrdm1.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\astrds0.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\astrds1.dat		%DIST_DIR%\samples\dos\bis
copy samples\dos\bis\astro.dat			%DIST_DIR%\samples\dos\bis

mkdir %DIST_DIR%\samples\dos\brview
copy samples\dos\brview\brview.exe      %DIST_DIR%\samples\dos\brview
copy samples\dos\brview\makefile.bcc	%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\brview.c        %DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example.c		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\venus.dat		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\std.pal			%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\torus.dat		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\shade.tab		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\rosewood.pix	%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example.act		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example.mat		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\makefile.wat	%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example.pix		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example.tab		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example.pal		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\makefile.msc	%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\marble.pix		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example2.mat	%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\example2.pix	%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\runme.bat		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\readme.txt		%DIST_DIR%\samples\dos\brview
copy samples\dos\brview\file			%DIST_DIR%\samples\dos\brview

mkdir %DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.exe			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\flight				%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\makefile.msc		%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\runme.bat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\makefile.wat		%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\test.c				%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\std.pal			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\makefile.bcc		%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.mat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\terrain5.dat		%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\terrain.pix		%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.h				%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\house.dat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\saucer.dat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\saucer.pix			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\roof.pix			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\planks.pix			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\x_chalet.pix		%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.pal			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.tab			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\logo.dat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\logo.pix			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\billb.dat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.dat			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\readme.txt			%DIST_DIR%\samples\dos\fly
copy samples\dos\fly\fly.c				%DIST_DIR%\samples\dos\fly

mkdir %DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\rdemo.exe		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\makefile.bcc		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\rdemo.c			%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\makefile.wat		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\std.pal			%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\makefile.msc		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\ast.pix			%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\shade.tab		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\moon.pix			%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\earth.pix		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\testenv1.pix		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\teapot.dat		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\sph32.dat		%DIST_DIR%\samples\dos\rdemo
copy samples\dos\rdemo\readme.txt		%DIST_DIR%\samples\dos\rdemo

mkdir %DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\sheet.exe		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\makefile.wat		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\makefile.bcc     %DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\sheet.mat		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\std.pal			%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\makefile.msc		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\wirscrn2.pix		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\shade.tab		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\cube.h			%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\sheet.c			%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\earth.pix		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\rivrock1.pix		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\sph32.dat		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\8ball.pix		%DIST_DIR%\samples\dos\sheet
copy samples\dos\sheet\readme.txt		%DIST_DIR%\samples\dos\sheet

mkdir %DIST_DIR%\samples\dos\techdemo

copy samples\dos\techdemo\press8.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\tech8.tga		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\logo8.tga		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\icons15.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\press15.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\tech15.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\logo15.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\icons24.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\press24.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\tech24.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\logo24.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\techdemo.mat	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\point.dat		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\spot.dat		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\direct.dat	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\grid.dat		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\std.pal		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\icons8.tga	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\shade.tab		%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\readme.txt	%DIST_DIR%\samples\dos\techdemo
copy samples\dos\techdemo\techdemo.exe	%DIST_DIR%\samples\dos\techdemo

mkdir %DIST_DIR%\samples\win\simpview

copy samples\win\simpview\simpview.exe	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\app.c			%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\app.h			%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\app.rc		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\argonaut.ico	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\mattab.c		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\buffer.c		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\buffer.h		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\dispatch.c	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\dispatch.h	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\main.c		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\makefile.wat	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\mattab.h		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\resource.h	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\simpview.ide	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\simpview.mak	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\wip.ico		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\world.c		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\world.h		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\mlibcmnt.c	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\earth.pix		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\mandrill.pix	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\teapot.dat	%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\cube2.dat		%DIST_DIR%\samples\win\simpview
copy samples\win\simpview\winshade.tab	%DIST_DIR%\samples\win\simpview

