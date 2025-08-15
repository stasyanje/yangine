#include "Engine.h"
#include "pch.h"
#include <iostream>

extern "C"
{
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace IEngine
{
Engine* Create()
{
    return new ::Engine();
}

void Destroy(Engine* app)
{
    delete app;
}

int Run(Engine* engine, HINSTANCE hInstance, int nCmdShow)
{
    if (!engine) return -1;
    return engine->Run(hInstance, nCmdShow);
}

void ExitGame() noexcept
{
    PostQuitMessage(0);
}
} // namespace IEngine
