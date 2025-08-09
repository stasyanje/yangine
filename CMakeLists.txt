cmake_minimum_required(VERSION 3.31)

project(
    app
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Engine

set(INCLUDE_ENGINE "src/engine/include")
set(SRC_ENGINE "src/engine")
set(VENDOR_WINPIX "vendor/winpix/bin")

set(MSVC_INCLUDE "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/include")
set(MSVC_LIB "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/lib/x64")

set(WINDOWS_SDK_INCLUDE "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0")
set(WINDOWS_SDK_LIB "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0")

add_subdirectory(./${SRC_ENGINE}/)

## Host application

set(SRC_APP "src/app")

add_executable(app ${SRC_APP}/main.cpp)

target_compile_definitions(app PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
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

## Linking park

target_link_directories(app
    PRIVATE 
        ${VENDOR_WINPIX}
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
    "${CMAKE_SOURCE_DIR}/vendor/winpix/bin/WinPixEventRuntime.dll"
    $<TARGET_FILE_DIR:app>
)

# Copy assets folder to output directory
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/src/engine/assets"
    "$<TARGET_FILE_DIR:app>/assets/engine"
    COMMENT "Copying assets to output directory"
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT app)
