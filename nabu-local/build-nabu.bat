@echo off
setlocal

rem Toolchain setup
set Z88DK_DIR=c:\z88dk\
set ZCCCFG=%Z88DK_DIR%lib\config\
set PATH=%Z88DK_DIR%bin;%PATH%

rem Deploy paths
set STORE_DIR=D:\NIA\NABU Internet Adapter\Store
set DRIVE_DIR=D:\NIA\NABU Internet Adapter\Store\CPM\D\0
set SCRIPT_DIR=%~dp0
set ROOT_DIR=%SCRIPT_DIR%..
set OUTPUT_DIR=%SCRIPT_DIR%r2r\nabu_cpm
set SRC_DIR=%ROOT_DIR%\src\nabu

if /I "%~1"=="clean" goto :clean

echo.
echo ****************************************************************************
echo  Building FujiNet Battleship NABU Local Client
echo ****************************************************************************

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

pushd "%SCRIPT_DIR%"

zcc +cpm -subtype=nabu -vn -create-app -compiler=sdcc -O3 --opt-code-size ^
 -DBUILD_NABU ^
 -I..\src ^
 -I..\src\nabu ^
 ..\src\nabu\main.c ^
 ..\src\misc.c ^
 ..\src\stateclient.c ^
 -o r2r\nabu_cpm\NBATTLE

if errorlevel 1 goto :fail

if not exist "%STORE_DIR%" mkdir "%STORE_DIR%"
if not exist "%DRIVE_DIR%" mkdir "%DRIVE_DIR%"

copy /Y "r2r\nabu_cpm\NBATTLE.com" "%DRIVE_DIR%\"

call :clean_temp

echo.
echo ****************************************************************************
echo  Done.
echo    NBATTLE.com copied to %DRIVE_DIR%
echo ****************************************************************************

popd
exit /b 0

:fail
echo.
echo Build failed.
popd
exit /b 1

:clean
echo.
echo ****************************************************************************
echo  Cleaning FujiNet Battleship NABU Local Client
echo ****************************************************************************
call :clean_all
echo.
echo Done.
exit /b 0

:clean_temp
if exist "%OUTPUT_DIR%\NBATTLE" del /Q "%OUTPUT_DIR%\NBATTLE"
if exist "%OUTPUT_DIR%\NBATTLE.img" del /Q "%OUTPUT_DIR%\NBATTLE.img"
if exist "%OUTPUT_DIR%\NBATTLE.lis" del /Q "%OUTPUT_DIR%\NBATTLE.lis"
if exist "%OUTPUT_DIR%\NBATTLE.map" del /Q "%OUTPUT_DIR%\NBATTLE.map"
if exist "%SRC_DIR%\graphics.c.lis" del /Q "%SRC_DIR%\graphics.c.lis"
if exist "%SRC_DIR%\input.c.lis" del /Q "%SRC_DIR%\input.c.lis"
if exist "%SRC_DIR%\network.c.lis" del /Q "%SRC_DIR%\network.c.lis"
if exist "%SRC_DIR%\sound.c.lis" del /Q "%SRC_DIR%\sound.c.lis"
if exist "%SRC_DIR%\util.c.lis" del /Q "%SRC_DIR%\util.c.lis"
exit /b 0

:clean_all
if exist "%OUTPUT_DIR%\NBATTLE" del /Q "%OUTPUT_DIR%\NBATTLE"
if exist "%OUTPUT_DIR%\NBATTLE.com" del /Q "%OUTPUT_DIR%\NBATTLE.com"
if exist "%OUTPUT_DIR%\NBATTLE.img" del /Q "%OUTPUT_DIR%\NBATTLE.img"
if exist "%OUTPUT_DIR%\NBATTLE.lis" del /Q "%OUTPUT_DIR%\NBATTLE.lis"
if exist "%OUTPUT_DIR%\NBATTLE.map" del /Q "%OUTPUT_DIR%\NBATTLE.map"
if exist "%SRC_DIR%\graphics.c.lis" del /Q "%SRC_DIR%\graphics.c.lis"
if exist "%SRC_DIR%\input.c.lis" del /Q "%SRC_DIR%\input.c.lis"
if exist "%SRC_DIR%\network.c.lis" del /Q "%SRC_DIR%\network.c.lis"
if exist "%SRC_DIR%\sound.c.lis" del /Q "%SRC_DIR%\sound.c.lis"
if exist "%SRC_DIR%\util.c.lis" del /Q "%SRC_DIR%\util.c.lis"
exit /b 0
