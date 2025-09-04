#pragma once

#include "pch.h"

#include "WindowManager.h"
#include "canvas/Camera.h"
#include "canvas/ConstantBuffer.h"
#include "canvas/Pipeline.h"
#include "canvas/Renderer.h"
#include "canvas/ResourceHolder.h"
#include "common/AsyncBuf.h"
#include "device/DeviceResources.h"
#include "input/InputController.h"
#include "window/WindowStateReducer.h"

class AsyncLogger;

class Engine
{
public:
    // Disallow copy / assign
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine() noexcept = default;
    ~Engine() noexcept = default;

    int Run(HINSTANCE hInstance, int nCmdShow);

private:
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int MessageLoop();

    std::unique_ptr<AsyncLogger> m_logger;
    std::unique_ptr<AsyncBuf> m_buf;
    std::unique_ptr<std::ostream> m_asyncOut;

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<DX::DeviceResources> m_deviceResources;
    std::unique_ptr<DX::Pipeline> m_pipeline;
    std::unique_ptr<canvas::Camera> m_camera;
    std::unique_ptr<canvas::ConstantBuffer> m_constantBuffer;
    std::unique_ptr<canvas::ResourceHolder> m_resourceHolder;
    std::unique_ptr<canvas::Renderer> m_renderer;
    std::unique_ptr<window::WindowStateReducer> m_stateReducer;
    std::unique_ptr<input::InputController> m_inputController;
};
