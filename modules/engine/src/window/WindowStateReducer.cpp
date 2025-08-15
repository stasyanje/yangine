#include "WindowStateReducer.h"
#include "../common/Helpers.h"

using namespace window;

RECT WindowStateReducer::InitialWindowBounds()
{
    RECT rc = {0, 0, 800, 600};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    return rc;
}

WindowStateReducer::WindowStateReducer() :
    m_windowState{}
{
}

void WindowStateReducer::Initialize(HWND hwnd, int nCmdShow, RECT bounds)
{
    m_hwnd = hwnd;
    m_windowState.bounds = bounds;

    ShowWindow(m_hwnd, nCmdShow);
    GetClientRect(m_hwnd, &m_windowState.bounds);
}

void WindowStateReducer::Reduce(Action action)
{
    switch (action)
    {
    case Action::TOGGLE_FULLSCREEN:
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
        break;

    case Action::SET_MONITOR_BOUNDS:
        if (m_windowState.monitorBounds.bottom != 0)
            break;

        SystemParametersInfo(SPI_GETWORKAREA, 0, &m_windowState.monitorBounds, 0);
        Helpers::PrintMonitorInfo(m_hwnd);
        Helpers::PrintWindowState(m_windowState);
        break;

    case Action::SET_MINIMIZED:
        std::cout << "WM_SIZE SIZE_MINIMIZED";

        if (m_windowState.minimized && m_windowState.in_suspend)
            break;

        m_windowState.minimized = true;
        m_windowState.in_suspend = true;
        Helpers::PrintWindowState(m_windowState);
        break;

    case Action::SET_UNMINIMIZED:
        std::cout << "WM_SIZE UNMINIMIZED";
        m_windowState.minimized = false;

        if (!m_windowState.in_suspend)
            break;

        m_windowState.in_suspend = false;

        Helpers::PrintWindowState(m_windowState);
        break;

    case Action::UPDATE_SIZE_BOUNDS:
        GetClientRect(m_hwnd, &m_windowState.bounds);
        break;

    case Action::ENTER_SIZEMOVE:
        std::cout << "WM_ENTERSIZEMOVE";
        m_windowState.in_sizemove = true;
        break;

    case Action::EXIT_SIZEMOVE:
        std::cout << "WM_EXITSIZEMOVE";
        m_windowState.in_sizemove = false;
        GetClientRect(m_hwnd, &m_windowState.bounds);
        Helpers::PrintWindowState(m_windowState);
        break;

    case Action::SET_SUSPEND:
        std::cout << "SET_SUSPEND";

        m_windowState.in_suspend = true;
        break;

    case Action::SET_RESUME:
        std::cout << "SET_RESUME";

        m_windowState.in_suspend = false;
        break;
    }
}