file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.h")

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine 
    PUBLIC
        include
    PRIVATE 
        vendor/directx
        vendor/directx/dxguids
        vendor/winpix
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