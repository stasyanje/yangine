#include "Engine.h"
#include "Renderer.h"
#include "WindowManager.h"
#include "common/AsyncBuf.h"
#include "common/AsyncLogger.h"
#include "input/InputController.h"
#include "pch.h"

using namespace DirectX;
using namespace Canvas;
using namespace Input;
using namespace DX;

Engine::Engine() :
    m_inputController(nullptr),
    m_renderer(nullptr),
    m_windowManager(nullptr),
    m_logger(nullptr),
    m_buf(nullptr),
    m_asyncOut(nullptr)
{
}

Engine::~Engine()
{
    Shutdown();
}

int Engine::Run(HINSTANCE hInstance, int nCmdShow)
{
    if (!XMVerifyCPUSupport())
        return 1;

    if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
        return 1;

    if (!Initialize(hInstance, nCmdShow))
        return 1;

    int result = MessageLoop();

    Shutdown();

    return result;
}

bool Engine::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    m_deviceResources = std::make_unique<DeviceResources>();
    m_inputController = std::make_unique<InputController>();
    m_renderer = std::make_unique<Renderer>(m_inputController.get(), m_deviceResources.get());
    m_windowManager = std::make_unique<WindowManager>();

    m_logger = std::make_unique<AsyncLogger>();
    m_buf = std::make_unique<AsyncBuf>(*m_logger);
    m_asyncOut = std::make_unique<std::ostream>(m_buf.get());

    std::cout.rdbuf(m_asyncOut->rdbuf()); // <-- теперь cout пишет в очередь
    std::cout.setf(std::ios::unitbuf);    // авто-flush на каждую запись

    return m_windowManager->Initialize(hInstance, nCmdShow, m_renderer.get(), m_inputController.get());
}

void Engine::Shutdown()
{
    if (m_windowManager)
    {
        m_windowManager->Shutdown();
        m_windowManager.reset();
    }

    if (m_renderer)
    {
        m_renderer.reset();
    }
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
