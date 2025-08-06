@echo off
setlocal

:: Define source files and output names
set SOURCE_DIR=src
set INCLUDE_DIR=include\engine
set BUILD_DIR=build\engine
set LIBRARY_NAME=yangine.lib

:: Create build directory if it doesn't exist
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

:: Compile object files
echo Compiling object files...
cl /MDd /std:c++20 /W4 /EHsc ^
    /c "src\engine\*.cpp" ^
    /I "include\engine" ^
    /I "vendor\engine\directx" ^
    /I "vendor\engine\winpix" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt\winrt" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt\winrt\impl" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared\ndis" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared\netcx" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared\netcx\shared" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared\netcx\shared\1.0" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared\netcx\shared\1.0\net" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared\netcx\shared\1.0\net\wifi" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt\sys" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um\gl" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um\winsqlite" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt\wrl" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt\wrl\wrappers" ^
    :: Outside of engine
    /I "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\include" ^
    /Fo: build\
    
:: Create static library

lib /out:"%BUILD_DIR%\%LIBRARY_NAME%" "%BUILD_DIR%\*.obj"

echo Static library "%LIBRARY_NAME%" created successfully in "%BUILD_DIR%".

endlocal
pause

