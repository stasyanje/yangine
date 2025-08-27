#include "Engine.h"
#include "WindowManager.h"
#include "canvas/Renderer.h"
#include "common/AsyncBuf.h"
#include "common/AsyncLogger.h"
#include "input/InputController.h"
#include "pch.h"

using namespace DirectX;
using namespace canvas;
using namespace input;
using namespace DX;

int Engine::Run(HINSTANCE hInstance, int nCmdShow)
{
    if (!XMVerifyCPUSupport())
        return 1;

    if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
        return 1;

    if (!Initialize(hInstance, nCmdShow))
        return 1;

    return MessageLoop();
}

bool Engine::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    m_logger = std::make_unique<AsyncLogger>();
    m_buf = std::make_unique<AsyncBuf>(*m_logger);
    m_asyncOut = std::make_unique<std::ostream>(m_buf.get());

    std::cout.rdbuf(m_asyncOut->rdbuf()); // <-- теперь cout пишет в очередь
    std::cout.setf(std::ios::unitbuf);    // авто-flush на каждую запись

    m_stateReducer = std::make_unique<window::WindowStateReducer>();
    m_deviceResources = std::make_unique<DeviceResources>(m_stateReducer.get());
    m_inputController = std::make_unique<InputController>(m_stateReducer.get());
    m_pipeline = std::make_unique<canvas::Pipeline>(m_inputController.get());
    m_renderer = std::make_unique<Renderer>(m_deviceResources.get(), m_pipeline.get());
    m_windowManager = std::make_unique<WindowManager>();

    auto hwnd = m_windowManager->Initialize(
        hInstance,
        m_stateReducer.get(),
        m_renderer.get(),
        m_inputController.get(),
        nCmdShow
    );

    m_deviceResources->Initialize(hwnd, m_renderer.get());
    m_stateReducer->Initialize(hwnd, nCmdShow);

#ifdef _DEBUG
    m_deviceResources->HandleDeviceLost(); // restart the resources to trigger memory warnings
#endif

    return true;
}

int Engine::MessageLoop()
{
    MSG msg = {};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_windowManager->Idle();
        }
    }

    return static_cast<int>(msg.wParam);
}
