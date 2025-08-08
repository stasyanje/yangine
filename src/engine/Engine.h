#pragma once

#include "pch.h"

class Renderer;
class WindowManager;

class Engine
{
public:
    Engine();
    ~Engine();

    int Run(HINSTANCE hInstance, int nCmdShow);

private:
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();
    int MessageLoop();

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<WindowManager> m_windowManager;
};
