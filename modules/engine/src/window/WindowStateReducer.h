#pragma once

#include "../common/WindowState.h"
#include <windows.h>

namespace window
{
enum class Action
{
    TOGGLE_FULLSCREEN,
    SET_MONITOR_BOUNDS,
    SET_MINIMIZED_AND_SUSPEND,
    SET_UNMINIMIZED,
    SET_RESUME,
    UPDATE_SIZE_BOUNDS,
    ENTER_SIZEMOVE,
    EXIT_SIZEMOVE,
    SET_SUSPEND,
    SET_RESUME_IF_NOT_MINIMIZED
};

class WindowStateReducer
{
public:
    WindowStateReducer();

    RECT InitialWindowBounds();

    RECT getBounds() { return m_windowState.bounds; };
    bool minimized() { return m_windowState.minimized; }
    bool suspended() { return m_windowState.in_suspend; }
    bool moving() { return m_windowState.in_sizemove; }

    void Initialize(HWND hwnd, int nCmdShow, RECT bounds);

    void Reduce(Action action);

private:
    HWND m_hwnd;
    WindowState m_windowState;
};
} // namespace window
