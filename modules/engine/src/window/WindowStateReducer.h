#pragma once

#include "WindowState.h"
#include <windows.h>

namespace window
{
enum class Action
{
    SET_MONITOR_BOUNDS,
    SET_MINIMIZED,
    SET_UNMINIMIZED,
    SET_RESUME,
    UPDATE_SIZE_BOUNDS,
    ENTER_SIZEMOVE,
    EXIT_SIZEMOVE,
    SET_SUSPEND,
    TOGGLE_FULLSCREEN,
};

class WindowStateReducer
{
public:
    // Disallow copy / assign
    WindowStateReducer(const WindowStateReducer&) = delete;
    WindowStateReducer& operator=(const WindowStateReducer&) = delete;

    WindowStateReducer();

    RECT InitialWindowBounds();

    int getWidth() { return m_windowState.bounds.right - m_windowState.bounds.left; };
    int getHeight() { return m_windowState.bounds.bottom - m_windowState.bounds.top; };

    float getAspectRatio()
    {
        int width = getWidth();
        int height = getHeight();
        return (height != 0) ? static_cast<float>(width) / height : 0.0f;
    }

    RECT getBounds() { return m_windowState.bounds; };
    bool minimized() { return m_windowState.minimized; }
    bool suspended() { return m_windowState.in_suspend; }
    bool moving() { return m_windowState.in_sizemove; }

    void Initialize(HWND hwnd, int nCmdShow);

    bool Reduce(Action action);

private:
    void PrintMonitorInfo();
    void PrintWindowState();

    HWND m_hwnd;
    WindowState m_windowState;
};
} // namespace window
