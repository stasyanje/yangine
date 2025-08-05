@echo off
setlocal

:: Define source files and output names
set SOURCE_DIR=src
set INCLUDE_DIR=include
set BUILD_DIR=build
set LIBRARY_NAME=yangine.lib

:: Create build directory if it doesn't exist
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

:: Compile object files
echo Compiling object files...
cl /MDd /std:c++20 /W4 /EHsc ^
    /I "include" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um" ^
    /c "src\*.cpp" ^
    /Fo: "build\yangine.o"

:: Create static library

lib /out:"%BUILD_DIR%\%LIBRARY_NAME%" "%BUILD_DIR%\*.o"

echo Static library "%LIBRARY_NAME%" created successfully in "%BUILD_DIR%".

endlocal
pause