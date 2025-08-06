cmake_minimum_required(VERSION 3.31)

project(
    yangine
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

# Windows SDK paths
set(WINDOWS_SDK_INCLUDE "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0")
set(WINDOWS_SDK_LIB "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0")

# Yangine static library
file(GLOB_RECURSE YANGINE_SOURCES "src/engine/*.cpp" "src/engine/*.h")
add_library(yangine STATIC ${YANGINE_SOURCES})

target_include_directories(yangine 
    PUBLIC include
    PRIVATE 
        src 
        vendor/engine/directx 
        vendor/engine/winpix
        "${WINDOWS_SDK_INCLUDE}/cppwinrt"
        "${WINDOWS_SDK_INCLUDE}/cppwinrt/winrt"
        "${WINDOWS_SDK_INCLUDE}/cppwinrt/winrt/impl"
        "${WINDOWS_SDK_INCLUDE}/shared"
        "${WINDOWS_SDK_INCLUDE}/shared/ndis"
        "${WINDOWS_SDK_INCLUDE}/shared/netcx"
        "${WINDOWS_SDK_INCLUDE}/shared/netcx/shared"
        "${WINDOWS_SDK_INCLUDE}/shared/netcx/shared/1.0"
        "${WINDOWS_SDK_INCLUDE}/shared/netcx/shared/1.0/net"
        "${WINDOWS_SDK_INCLUDE}/shared/netcx/shared/1.0/net/wifi"
        "${WINDOWS_SDK_INCLUDE}/ucrt"
        "${WINDOWS_SDK_INCLUDE}/ucrt/sys"
        "${WINDOWS_SDK_INCLUDE}/um"
        "${WINDOWS_SDK_INCLUDE}/um/gl"
        "${WINDOWS_SDK_INCLUDE}/um/winsqlite"
        "${WINDOWS_SDK_INCLUDE}/winrt"
        "${WINDOWS_SDK_INCLUDE}/winrt/wrl"
        "${WINDOWS_SDK_INCLUDE}/winrt/wrl/wrappers"
)

target_precompile_headers(yangine PRIVATE src/engine/pch.h)

# Yangine is a static library with no external link dependencies
# (all dependencies are header-only or linked at the application level)

# Host application
add_executable(yangine_app main.cpp)
target_include_directories(yangine_app PRIVATE 
    src 
    vendor/engine/directx 
    vendor/engine/winpix
    "${WINDOWS_SDK_INCLUDE}/um"
    "${WINDOWS_SDK_INCLUDE}/shared"
    "${WINDOWS_SDK_INCLUDE}/winrt"
    "${WINDOWS_SDK_INCLUDE}/cppwinrt"
)
target_link_directories(yangine_app PRIVATE 
    "${WINDOWS_SDK_LIB}/um/x64"
    "${WINDOWS_SDK_LIB}/ucrt/x64"
    vendor/engine/bin
)
target_link_libraries(yangine_app 
    yangine
    
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
set_target_properties(yangine_app PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

# Copy WinPixEventRuntime.dll to output directory
add_custom_command(TARGET yangine_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/vendor/engine/bin/WinPixEventRuntime.dll"
    $<TARGET_FILE_DIR:yangine_app>
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT yangine_app)