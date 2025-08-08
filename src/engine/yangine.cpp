#include "pch.h"
#include "Application.h"
#include "../../include/engine/yangine.h"
#include <iostream>

extern "C"
{
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace yangine
{
Application* CreateApplication()
{
    return new ::Application();
}

void DestroyApplication(Application* app)
{
    delete app;
}

int RunApplication(Application* app, HINSTANCE hInstance, int nCmdShow)
{
    if (!app) return -1;
    return app->Run(hInstance, nCmdShow);
}

void ExitGame() noexcept
{
    PostQuitMessage(0);
}
} // namespace yangine
