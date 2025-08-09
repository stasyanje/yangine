cmake_minimum_required(VERSION 3.31)

project(
    app
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(modules/engine)

add_executable(app modules/app/main.cpp)

target_compile_definitions(app PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_include_directories(app
    PRIVATE 
        modules/engine/include
        modules/app
)

## Linking park

target_link_directories(app
    PRIVATE 
        vendor/winpix/bin
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
    "${CMAKE_SOURCE_DIR}/modules/engine/assets"
    "$<TARGET_FILE_DIR:app>/assets/engine"
    COMMENT "Copying assets to output directory"
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT app)
