#pragma once

#include <Windows.h>

// Forward declaration of the actual Application class
class Engine;

namespace IEngine
{
// Library interface functions
Engine* Create();
void Destroy(Engine* app);
int Run(Engine* app, HINSTANCE hInstance, int nCmdShow);

// Utility function
void ExitGame() noexcept;
} // namespace IEngine
