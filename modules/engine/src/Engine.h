#pragma once

#include "pch.h"

class InputController;
class Renderer;
class WindowManager;
class AsyncLogger;
class AsyncBuf;

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
    void QueryGPUMemoryInfo();

    std::unique_ptr<InputController> m_inputController;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<AsyncLogger> m_logger;
    std::unique_ptr<AsyncBuf> m_buf;
    std::unique_ptr<std::ostream> m_asyncOut;
};
