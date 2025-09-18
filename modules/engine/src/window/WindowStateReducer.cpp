#include "WindowStateReducer.h"

using namespace window;

RECT WindowStateReducer::InitialWindowBounds()
{
    RECT rc = {0, 0, 800, 600};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    if (m_windowState.bounds.right == 0 && m_windowState.bounds.bottom == 0)
        m_windowState.bounds = rc;

    return rc;
}

WindowStateReducer::WindowStateReducer() :
    m_windowState{}
{
}

void WindowStateReducer::Initialize(HWND hwnd, int nCmdShow)
{
    m_hwnd = hwnd;
    ShowWindow(m_hwnd, nCmdShow);
    GetClientRect(m_hwnd, &m_windowState.bounds);
}

bool WindowStateReducer::Reduce(Action action)
{
    switch (action) {
    case Action::TOGGLE_FULLSCREEN:
        if (m_windowState.fullscreen) {
            SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
            SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, 0);

            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 800, 600, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            ShowCursor(TRUE);
        }
        else {
            SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP);
            SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
            SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);
            ShowCursor(FALSE);
        }

        m_windowState.fullscreen = !m_windowState.fullscreen;
        break;

    case Action::SET_MONITOR_BOUNDS:
        if (m_windowState.monitorBounds.bottom != 0)
            break;

        SystemParametersInfo(SPI_GETWORKAREA, 0, &m_windowState.monitorBounds, 0);
        PrintMonitorInfo();
        PrintWindowState();
        break;

    case Action::SET_MINIMIZED:
        std::cout << "WM_SIZE SIZE_MINIMIZED";

        if (m_windowState.minimized && m_windowState.in_suspend)
            break;

        m_windowState.minimized = true;
        m_windowState.in_suspend = true;
        PrintWindowState();
        break;

    case Action::SET_UNMINIMIZED:
        std::cout << "WM_SIZE UNMINIMIZED";
        m_windowState.minimized = false;

        if (!m_windowState.in_suspend)
            break;

        m_windowState.in_suspend = false;

        PrintWindowState();
        break;

    case Action::UPDATE_SIZE_BOUNDS:
        auto current = m_windowState.bounds;
        GetClientRect(m_hwnd, &m_windowState.bounds);
        return !EqualRect(&current, &m_windowState.bounds);

    case Action::ENTER_SIZEMOVE:
        std::cout << "WM_ENTERSIZEMOVE";
        m_windowState.in_sizemove = true;
        break;

    case Action::EXIT_SIZEMOVE:
        std::cout << "WM_EXITSIZEMOVE";
        m_windowState.in_sizemove = false;
        GetClientRect(m_hwnd, &m_windowState.bounds);
        PrintWindowState();
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

    return true;
}

void WindowStateReducer::PrintMonitorInfo()
{
    auto monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTOPRIMARY);
    assert(monitor);

    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX); // Set the size of the structure
    GetMonitorInfo(monitor, &monitorInfo);

    std::ostringstream message;
    message << "MonitorInfo"
            << " | work: " << monitorInfo.rcWork.right << "x" << monitorInfo.rcWork.bottom
            << " | full: " << monitorInfo.rcMonitor.right << "x" << monitorInfo.rcMonitor.bottom
            << " | primary: " << (monitorInfo.dwFlags & MONITORINFOF_PRIMARY);
    std::cout << message.str();
}

void WindowStateReducer::PrintWindowState()
{
    auto ws = m_windowState;

    std::ostringstream message;
    message << "WindowState"
            << " | size: " << ws.bounds.right << "x" << ws.bounds.bottom
            << " | max_size: " << ws.monitorBounds.right << "x" << ws.monitorBounds.bottom
            << " | fullscreen: " << ws.fullscreen
            << " | sizemove: " << ws.in_sizemove
            << " | suspend: " << ws.in_suspend
            << " | minimized: " << ws.minimized;
    std::cout << message.str();
}