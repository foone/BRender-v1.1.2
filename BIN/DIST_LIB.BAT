@echo off
rem Copy Libraries to dist. directory
rem
set DIST_DIR=%1

rem Directories
rem
mkdir %DIST_DIR%\lib
mkdir %DIST_DIR%\inc
mkdir %DIST_DIR%\dosio
mkdir %DIST_DIR%\samples
mkdir %DIST_DIR%\samples\stdfile

rem Libraries
rem
rem Watcom
rem
copy lib\brfmwxrr.lib %DIST_DIR%\lib
copy lib\brfwwxrr.lib %DIST_DIR%\lib
copy lib\brstwxrr.lib %DIST_DIR%\lib
copy lib\brzbwxrr.lib %DIST_DIR%\lib
copy lib\brzswxrr.lib %DIST_DIR%\lib

copy lib\brfmwfrr.lib %DIST_DIR%\lib
copy lib\brfwwfrr.lib %DIST_DIR%\lib
copy lib\brstwfrr.lib %DIST_DIR%\lib
copy lib\brzbwfrr.lib %DIST_DIR%\lib
copy lib\brzswfrr.lib %DIST_DIR%\lib

copy lib\brfmwxrs.lib %DIST_DIR%\lib
copy lib\brfwwxrs.lib %DIST_DIR%\lib
copy lib\brstwxrs.lib %DIST_DIR%\lib
copy lib\brzbwxrs.lib %DIST_DIR%\lib
copy lib\brzswxrs.lib %DIST_DIR%\lib

copy lib\brfmwfrs.lib %DIST_DIR%\lib
copy lib\brfwwfrs.lib %DIST_DIR%\lib
copy lib\brstwfrs.lib %DIST_DIR%\lib
copy lib\brzbwfrs.lib %DIST_DIR%\lib
copy lib\brzswfrs.lib %DIST_DIR%\lib

rem Microsoft
rem
copy lib\brfmvxr.lib %DIST_DIR%\lib\brfmmxr.lib
copy lib\brfwvxr.lib %DIST_DIR%\lib\brfwmxr.lib
copy lib\brstvxr.lib %DIST_DIR%\lib\brstmxr.lib
copy lib\brzbvxr.lib %DIST_DIR%\lib\brzbmxr.lib
copy lib\brzsvxr.lib %DIST_DIR%\lib\brzsmxr.lib

copy lib\brfmmfr.lib %DIST_DIR%\lib
copy lib\brfwmfr.lib %DIST_DIR%\lib
copy lib\brstmfr.lib %DIST_DIR%\lib
copy lib\brzbmfr.lib %DIST_DIR%\lib
copy lib\brzsmfr.lib %DIST_DIR%\lib

rem Borland
rem
copy lib\brfmbxr.lib %DIST_DIR%\lib
copy lib\brfwbxr.lib %DIST_DIR%\lib
copy lib\brstbxr.lib %DIST_DIR%\lib
copy lib\brzbbxr.lib %DIST_DIR%\lib
copy lib\brzsbxr.lib %DIST_DIR%\lib

copy lib\brfmbfr.lib %DIST_DIR%\lib
copy lib\brfwbfr.lib %DIST_DIR%\lib
copy lib\brstbfr.lib %DIST_DIR%\lib
copy lib\brzbbfr.lib %DIST_DIR%\lib
copy lib\brzsbfr.lib %DIST_DIR%\lib

rem DOSIO Libraries
rem
rem Watcom
rem
copy lib\dio4gwrr.lib %DIST_DIR%\lib
copy lib\dio4gwrs.lib %DIST_DIR%\lib
copy lib\diox3wrr.lib %DIST_DIR%\lib
copy lib\diox3wrs.lib %DIST_DIR%\lib
copy lib\dioplwrr.lib %DIST_DIR%\lib
copy lib\dioplwrs.lib %DIST_DIR%\lib

rem Microsoft
rem
copy lib\dioplmr.lib %DIST_DIR%\lib

rem Borland
rem
copy lib\dioplbr.lib %DIST_DIR%\lib
copy lib\dioppbr.lib %DIST_DIR%\lib

rem Includes
rem

copy inc\actor.h %DIST_DIR%\inc
copy inc\angles.h %DIST_DIR%\inc
copy inc\blockops.h %DIST_DIR%\inc
copy inc\brassert.h %DIST_DIR%\inc
copy inc\brender.h %DIST_DIR%\inc
copy inc\brerr.h %DIST_DIR%\inc
copy inc\brdiag.h %DIST_DIR%\inc
copy inc\brfile.h %DIST_DIR%\inc
copy inc\brfont.h %DIST_DIR%\inc
copy inc\brhton.h %DIST_DIR%\inc
copy inc\brlimits.h %DIST_DIR%\inc
copy inc\brmem.h %DIST_DIR%\inc
copy inc\camera.h %DIST_DIR%\inc
copy inc\colour.h %DIST_DIR%\inc
copy inc\compend.h %DIST_DIR%\inc
copy inc\compiler.h %DIST_DIR%\inc
copy inc\custsupt.h %DIST_DIR%\inc
copy inc\fixed.h %DIST_DIR%\inc
copy inc\fmt.h %DIST_DIR%\inc
copy inc\fmtproto.h %DIST_DIR%\inc
copy inc\fwpextra.h %DIST_DIR%\inc
copy inc\fwproto.h %DIST_DIR%\inc
copy inc\g386ifix.h %DIST_DIR%\inc
copy inc\light.h %DIST_DIR%\inc
copy inc\material.h %DIST_DIR%\inc
copy inc\matrix.h %DIST_DIR%\inc
copy inc\model.h %DIST_DIR%\inc
copy inc\mscifix.h %DIST_DIR%\inc
copy inc\order.h %DIST_DIR%\inc
copy inc\pixelmap.h %DIST_DIR%\inc
copy inc\pool.h %DIST_DIR%\inc
copy inc\quantize.h %DIST_DIR%\inc
copy inc\quat.h %DIST_DIR%\inc
copy inc\scalar.h %DIST_DIR%\inc
copy inc\scale.h %DIST_DIR%\inc
copy inc\transfrm.h %DIST_DIR%\inc
copy inc\vecfns.h %DIST_DIR%\inc
copy inc\vector.h %DIST_DIR%\inc
copy inc\wtcifix.h %DIST_DIR%\inc
copy inc\zbproto.h %DIST_DIR%\inc
copy inc\zsproto.h %DIST_DIR%\inc


rem copy sys\segregs.h %DIST_DIR%\sys

copy dosio\dosio.h %DIST_DIR%\dosio
copy dosio\keyboard.h %DIST_DIR%\dosio
copy dosio\dosproto.h %DIST_DIR%\dosio
copy dosio\dospxtra.h %DIST_DIR%\dosio
copy dosio\eventq.h %DIST_DIR%\dosio
copy dosio\syshost.h %DIST_DIR%\dosio

rem Stdlib Handlers
rem
copy std\stdfile.c %DIST_DIR%\samples\stdfile
copy std\stdmem.c %DIST_DIR%\samples\stdfile
copy std\stddiag.c %DIST_DIR%\samples\stdfile
copy fw\ffhooks.c %DIST_DIR%\samples\stdfile



