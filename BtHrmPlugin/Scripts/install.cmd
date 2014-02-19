@echo off

REM Simple script to copy a built libBlHrmPlugin.so file to the 
REM correct location in the Unity project

set LIB=libBtHrmPlugin.so

set LIBDIR=c:\NDK-workspaces\bbndk-10-workspaces\bbndk-10-2-1-games\BtHrmPlugin\Device-Debug
REM LIBDIR=c:\NDK-workspaces\bbndk-10-workspaces\bbndk-10-2-1-games\BtHrmPlugin\Device-Release

set TARGETDIR=C:\Users\jomurray\Documents\Gaming\Unity\HRM-Plugin-Example\Assets\Plugins\BlackBerry\libs
set PROJECTDIR=c:\NDK-workspaces\bbndk-10-workspaces\bbndk-10-2-1-games\BtHrmPlugin\Unity\HRM-Plugin-Example\Assets\Plugins\BlackBerry\libs

set LIBPATH=%LIBDIR%\%LIB%
set TARGETPATH=%TARGETDIR%\%LIB%
set PROJECTPATH=%PROJECTDIR%\%LIB%

COPY /Y %LIBPATH% %TARGETPATH%
COPY /Y %LIBPATH% %PROJECTPATH%

:end
