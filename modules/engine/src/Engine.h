#pragma once

#include "pch.h"

#include "canvas/Renderer.h"
#include "common/StepTimer.h"
#include "device/DeviceResources.h"
#include "input/InputController.h"
#include "window/WindowStateReducer.h"

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
    std::unique_ptr<AsyncLogger> m_logger;
    std::unique_ptr<AsyncBuf> m_buf;
    std::unique_ptr<std::ostream> m_asyncOut;
    std::unique_ptr<window::WindowStateReducer> m_stateReducer;
    std::unique_ptr<DX::DeviceResources> m_deviceResources;
    std::unique_ptr<input::InputController> m_inputController;
    std::unique_ptr<canvas::Renderer> m_renderer;
    std::unique_ptr<WindowManager> m_windowManager;

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();
    int MessageLoop();
};
