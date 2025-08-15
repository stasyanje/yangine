#pragma once

#include "WindowState.h"
#include <string.h>

namespace Helpers
{
void PrintWindowState(const WindowState& ws)
{
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

void PrintMonitorInfo(HWND hWnd)
{
    auto monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
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
} // namespace Helpers