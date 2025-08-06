#pragma once

#include <Windows.h>

// Forward declaration of the actual Application class
class Application;

namespace yangine
{
// Library interface functions
Application* CreateApplication();
void DestroyApplication(Application* app);
int RunApplication(Application* app, HINSTANCE hInstance, int nCmdShow);

// Utility function
void ExitGame() noexcept;
} // namespace yangine