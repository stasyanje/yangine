#include "WindowManager.h"
#include "Renderer.h"
#include "common/AsyncLogger.h"
#include "common/Helpers.cpp"
#include "common/WindowState.h"
#include "input/InputController.h"
#include "pch.h"

extern LPCWSTR g_szAppName;

WindowManager::WindowManager() :
    m_hwnd(nullptr),
    m_hInstance(nullptr),
    m_renderer(nullptr),
    m_windowState()
{
}

WindowManager::~WindowManager()
{
    Shutdown();
}

bool WindowManager::Initialize(
    HINSTANCE hInstance,
    int nCmdShow,
    Canvas::Renderer* renderer,
    Input::InputController* inputController
)
{
    m_hInstance = hInstance;
    m_inputController = inputController;
    m_renderer = renderer;

    if (!RegisterWindowClass())
        return false;

    if (!CreateRendererWindow(nCmdShow))
        return false;

    return true;
}

void WindowManager::Idle()
{
    m_renderer->OnWindowMessage(Canvas::Message::PAINT, m_windowState);
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

bool WindowManager::CreateRendererWindow(int nCmdShow)
{
    RECT rc = {0, 0, 800, 600};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        L"direct_3d_playgroundWindowClass",
        g_szAppName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, nCmdShow);
    GetClientRect(m_hwnd, &rc);
    m_renderer->Initialize(m_hwnd, rc.right - rc.left, rc.bottom - rc.top);

    return true;
}

void WindowManager::ToggleFullscreen()
{
    if (m_windowState.fullscreen)
    {
        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, 0);

        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 800, 600, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else
    {
        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP);
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);
    }

    m_windowState.fullscreen = !m_windowState.fullscreen;
}

Canvas::Message WindowManager::CanvasMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ACTIVATEAPP:
    {
        if (wParam)
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, &m_windowState.monitorBounds, 0);
            Helpers::PrintMonitorInfo(hWnd);
            return Canvas::Message::ACTIVATED;
        }
        return Canvas::Message::DEACTIVATED;
    }
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            if (m_windowState.minimized && m_windowState.in_suspend)
                break;

            m_windowState.minimized = true;
            m_windowState.in_suspend = true;

            return Canvas::Message::SUSPENDED;
        }
        else if (m_windowState.minimized)
        {
            m_windowState.minimized = false;

            if (!m_windowState.in_suspend)
                break;

            m_windowState.in_suspend = false;

            return Canvas::Message::RESUMED;
        }
        else
        {
            m_windowState.bounds.right = LOWORD(lParam);
            m_windowState.bounds.bottom = HIWORD(lParam);
            GetClientRect(hWnd, &m_windowState.bounds);

            return Canvas::Message::SIZE_CHANGED;
        }
        break;
    }
    case WM_DISPLAYCHANGE:
        return Canvas::Message::DISPLAY_CHANGED;

    case WM_MOVE:
        return Canvas::Message::MOVED;

    case WM_ENTERSIZEMOVE:
        m_windowState.in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
    {
        m_windowState.in_sizemove = false;
        GetClientRect(hWnd, &m_windowState.bounds);

        return Canvas::Message::SIZE_CHANGED;
    }
    case WM_POWERBROADCAST:
    {
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
        {
            if (m_windowState.in_suspend)
                break;

            m_windowState.in_suspend = true;
            return Canvas::Message::SUSPENDED;
        }
        case PBT_APMRESUMESUSPEND:
        {
            if (!m_windowState.in_suspend)
                break;

            m_windowState.in_suspend = false;

            if (m_windowState.minimized)
                break;

            return Canvas::Message::RESUMED;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return Canvas::Message::IDLE;
}

Input::Message WindowManager::InputMessage(UINT message)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        return Input::Message::LBUTTONDOWN;
    case WM_LBUTTONUP:
        return Input::Message::LBUTTONUP;
    case WM_RBUTTONDOWN:
        return Input::Message::RBUTTONDOWN;
    case WM_RBUTTONUP:
        return Input::Message::RBUTTONUP;
    case WM_MBUTTONDOWN:
        return Input::Message::MBUTTONDOWN;
    case WM_MBUTTONUP:
        return Input::Message::MBUTTONUP;
    case WM_MOUSEWHEEL:
        return Input::Message::MOUSEWHEEL;
    case WM_MOUSEMOVE:
        return Input::Message::MOUSEMOVE;
    default:
        return Input::Message::IDLE;
    }
}

void WindowManager::OnWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    m_inputController->OnWindowMessage(hWnd, InputMessage(message), wParam, lParam);
    m_renderer->OnWindowMessage(CanvasMessage(hWnd, message, wParam, lParam), m_windowState);

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
        if (m_windowState.in_sizemove && m_renderer)
        {
            m_renderer->OnWindowMessage(Canvas::Message::PAINT, m_windowState);
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
            ToggleFullscreen();

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
