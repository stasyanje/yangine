#include "pch.h"
#include "Engine.h"
#include "Renderer.h"
#include "WindowManager.h"

using namespace DirectX;

Engine::Engine() :
    m_renderer(nullptr),
    m_windowManager(nullptr)
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
    m_renderer = std::make_unique<Renderer>();
    m_windowManager = std::make_unique<WindowManager>();

    if (!m_windowManager->Initialize(hInstance, nCmdShow, m_renderer.get()))
        return false;

    return true;
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
            m_renderer->Tick();
        }
    }

    return static_cast<int>(msg.wParam);
}
