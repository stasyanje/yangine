file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.h")

set(MSVC_INCLUDE "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/include")
set(WINDOWS_SDK_INCLUDE "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0")

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine 
    PUBLIC
        include
    PRIVATE 
        vendor/directx
        vendor/directx/dxguids
        vendor/winpix
        ${MSVC_INCLUDE}
        ${MSVC_INCLUDE}/uwp
        ${WINDOWS_SDK_INCLUDE}/shared
        ${WINDOWS_SDK_INCLUDE}/cppwinrt
        ${WINDOWS_SDK_INCLUDE}/cppwinrt/winrt
        ${WINDOWS_SDK_INCLUDE}/cppwinrt/winrt/impl
        ${WINDOWS_SDK_INCLUDE}/ucrt
        ${WINDOWS_SDK_INCLUDE}/ucrt/sys
        ${WINDOWS_SDK_INCLUDE}/um
        ${WINDOWS_SDK_INCLUDE}/um/gl
        ${WINDOWS_SDK_INCLUDE}/um/winsqlite
        ${WINDOWS_SDK_INCLUDE}/winrt
        ${WINDOWS_SDK_INCLUDE}/winrt/wrl
        ${WINDOWS_SDK_INCLUDE}/winrt/wrl/wrappers
)

target_precompile_headers(engine PRIVATE
    pch.h
)

target_compile_definitions(engine PRIVATE
    UNICODE
    _UNICODE
    # Use the C++ standard templated min/max
    NOMINMAX
    # DirectX apps don't need GDI
    NODRAWTEXT
    NOGDI
    NOBITMAP
    # Include <mcx.h> if you need this
    NOMCX
    # Include <winsvc.h> if you need this
    NOSERVICE
    # WinHelp is deprecated
    NOHELP
    _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
)