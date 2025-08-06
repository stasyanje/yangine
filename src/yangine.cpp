#include "pch.h"
#include "Application.h"
#include "../include/yangine.h"
#include <iostream>

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