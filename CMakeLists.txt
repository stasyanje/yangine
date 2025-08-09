cmake_minimum_required(VERSION 3.31)

project(
    app
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Engine

set(INCLUDE_ENGINE "include/engine")
set(SRC_ENGINE "src/engine")
set(VENDOR_ENGINE "vendor/engine")

set(MSVC_INCLUDE "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/include")
set(MSVC_LIB "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/lib/x64")

set(WINDOWS_SDK_INCLUDE "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0")
set(WINDOWS_SDK_LIB "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0")

file(GLOB_RECURSE ENGINE_SOURCES "${SRC_ENGINE}/*.cpp" "${SRC_ENGINE}/*.h")
add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine 
    PUBLIC
        ${INCLUDE_ENGINE}
    PRIVATE 
        ${SRC_ENGINE}
        ${VENDOR_ENGINE}/directx 
        ${VENDOR_ENGINE}/winpix
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

target_precompile_headers(engine PRIVATE ${SRC_ENGINE}/pch.h)

## Host application

set(SRC_APP "src/app")

add_executable(app ${SRC_APP}/main.cpp)

target_compile_definitions(app PRIVATE
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN
)

target_include_directories(app
    PRIVATE 
        ${INCLUDE_ENGINE}
        ${SRC_APP}
        ${SRC_ENGINE}
        ${MSVC_INCLUDE}
        ${MSVC_INCLUDE}/uwp

)
target_link_directories(app
    PRIVATE 
        ${VENDOR_ENGINE}/bin
        ${MSVC_LIB}
        ${MSVC_LIB}/uwp
        ${WINDOWS_SDK_LIB}/um/x64
        ${WINDOWS_SDK_LIB}/ucrt/x64
)
target_link_libraries(app 
    engine
    
    # DirectX libraries
    d3d12
    dxgi
    dxguid
    d3dcompiler
    
    # PIX runtime library
    WinPixEventRuntime
    
    # Windows libraries
    kernel32
    user32
    advapi32
    shell32
    uuid
)

# Set Windows subsystem for GUI application
set_target_properties(app PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

# Copy WinPixEventRuntime.dll to output directory
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/${VENDOR_ENGINE}/bin/WinPixEventRuntime.dll"
    $<TARGET_FILE_DIR:app>
)

# Copy assets folder to output directory
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:app>/assets"
    COMMENT "Copying assets to output directory"
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT app)
