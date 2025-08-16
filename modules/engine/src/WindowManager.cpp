#include "WindowManager.h"
#include "canvas/Renderer.h"
#include "common/AsyncLogger.h"
#include "input/InputController.h"
#include "pch.h"

extern LPCWSTR g_szAppName;

WindowManager::WindowManager() :
    m_hwnd(nullptr),
    m_hInstance(nullptr),
    m_renderer(nullptr),
    m_stateReducer(nullptr)
{
}

WindowManager::~WindowManager()
{
    Shutdown();
}

HWND WindowManager::Initialize(
    HINSTANCE hInstance,
    int nCmdShow,
    window::WindowStateReducer* stateReducer,
    canvas::Renderer* renderer,
    input::InputController* inputController
)
{
    m_hInstance = hInstance;
    m_inputController = inputController;
    m_renderer = renderer;
    m_stateReducer = stateReducer;

    if (!RegisterWindowClass())
        throw GetLastError();

    return CreateRendererWindow(nCmdShow);
}

void WindowManager::Idle()
{
    m_renderer->OnWindowMessage(canvas::Message::PAINT, m_stateReducer->getBounds());
}

void WindowManager::Shutdown()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool WindowManager::RegisterWindowClass()
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIconW(m_hInstance, L"IDI_ICON");
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"direct_3d_playgroundWindowClass";
    wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");

    return RegisterClassExW(&wcex) != 0;
}

HWND WindowManager::CreateRendererWindow(int nCmdShow) noexcept(false)
{
    RECT windowBounds = m_stateReducer->InitialWindowBounds();

    m_hwnd = CreateWindowExW(
        0,
        L"direct_3d_playgroundWindowClass",
        g_szAppName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowBounds.right - windowBounds.left,
        windowBounds.bottom - windowBounds.top,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hwnd)
        throw GetLastError();

    return m_hwnd;
}

canvas::Message WindowManager::CanvasMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ACTIVATEAPP:
    {
        if (wParam)
        {
            m_stateReducer->Reduce(window::Action::SET_MONITOR_BOUNDS);
            return canvas::Message::ACTIVATED;
        }
        return canvas::Message::DEACTIVATED;
    }
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            m_stateReducer->Reduce(window::Action::SET_MINIMIZED);
            return canvas::Message::SUSPENDED;
        }
        else if (m_stateReducer->minimized())
        {
            m_stateReducer->Reduce(window::Action::SET_UNMINIMIZED);
            return canvas::Message::RESUMED;
        }
        else if (m_stateReducer->Reduce(window::Action::UPDATE_SIZE_BOUNDS))
        {
            return canvas::Message::SIZE_CHANGED;
        }
        break;
    }
    case WM_DISPLAYCHANGE:
        return canvas::Message::DISPLAY_CHANGED;

    case WM_ENTERSIZEMOVE:
        m_stateReducer->Reduce(window::Action::ENTER_SIZEMOVE);
        break;

    case WM_EXITSIZEMOVE:
    {
        m_stateReducer->Reduce(window::Action::EXIT_SIZEMOVE);
        return canvas::Message::SIZE_CHANGED;
    }
    case WM_POWERBROADCAST:
    {
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
        {
            if (m_stateReducer->suspended())
                break;

            m_stateReducer->Reduce(window::Action::SET_SUSPEND);
            return canvas::Message::SUSPENDED;
        }
        case PBT_APMRESUMESUSPEND:
        {
            if (!m_stateReducer->suspended())
                break;

            m_stateReducer->Reduce(window::Action::SET_RESUME);

            if (m_stateReducer->minimized())
                break;

            return canvas::Message::RESUMED;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return canvas::Message::IDLE;
}

input::Message WindowManager::InputMessage(UINT message)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        return input::Message::LBUTTONDOWN;
    case WM_LBUTTONUP:
        return input::Message::LBUTTONUP;
    case WM_RBUTTONDOWN:
        return input::Message::RBUTTONDOWN;
    case WM_RBUTTONUP:
        return input::Message::RBUTTONUP;
    case WM_MBUTTONDOWN:
        return input::Message::MBUTTONDOWN;
    case WM_MBUTTONUP:
        return input::Message::MBUTTONUP;
    case WM_MOUSEWHEEL:
        return input::Message::MOUSEWHEEL;
    case WM_MOUSEMOVE:
        return input::Message::MOUSEMOVE;
    default:
        return input::Message::IDLE;
    }
}

void WindowManager::OnWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    m_inputController->OnWindowMessage(hWnd, InputMessage(message), wParam, lParam);
    m_renderer->OnWindowMessage(CanvasMessage(hWnd, message, wParam, lParam), m_stateReducer->getBounds());

    switch (message)
    {
    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;
    case WM_PAINT:
        if (m_stateReducer->moving())
        {
            m_renderer->OnWindowMessage(canvas::Message::PAINT, m_stateReducer->getBounds());
        }
        else
        {
            PAINTSTRUCT ps;
            std::ignore = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
            m_stateReducer->Reduce(window::Action::TOGGLE_FULLSCREEN);

        break;

    default:
        break;
    }
}

LRESULT CALLBACK WindowManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NCCREATE:
    {
        auto create_struct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        auto self = static_cast<WindowManager*>(create_struct->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));

        std::cout << "WM_NCCREATE, attached WindowManager to HWND";

        return true;
    }
    case WM_DESTROY:
    {
        std::cout << "WM_DESTROY -> PostQuitMessage(0)";
        PostQuitMessage(0);
        break;
    }
    case WM_POWERBROADCAST: // also defined at Canvas::Message
    {
        if (wParam == PBT_APMQUERYSUSPEND || wParam == PBT_APMRESUMESUSPEND)
            return true; // always agree to power management requests

        break;
    }
    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);

    default:
        break;
    }

    auto windowManager = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (windowManager)
        windowManager->OnWindowMessage(hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}
