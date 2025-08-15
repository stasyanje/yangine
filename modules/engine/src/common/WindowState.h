#pragma once

struct WindowState
{
    SIZE monitorSize = {0, 0};
    RECT bounds = {0, 0, 0, 0};

    bool fullscreen = false;
    bool in_sizemove = false;
    bool in_suspend = false;
    bool minimized = false;
};

namespace Helpers
{
void PrintWindowState(const WindowState& ws)
{
    std::cout
        << "monitor: " << ws.monitorSize.cx << "x" << ws.monitorSize.cy
        << " | bounds: (" << ws.bounds.left << "," << ws.bounds.top
        << ")-(" << ws.bounds.right << "," << ws.bounds.bottom << ")"
        << " | fullscreen: " << ws.fullscreen
        << " | sizemove: " << ws.in_sizemove
        << " | suspend: " << ws.in_suspend
        << " | minimized: " << ws.minimized
        << '\n';
}

void PrintMonitorInfo(HWND hWnd)
{
    auto monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    assert(monitor);

    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX); // Set the size of the structure
    GetMonitorInfo(monitor, &monitorInfo);

    std::wostringstream message;

    message << "WM_ACTIVATEAPP" << std::endl;

    message << "Monitor Information:" << std::endl
            << "  Device Name: " << monitorInfo.szDevice << std::endl
            << "  Primary Monitor: " << ((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) ? "Yes" : "No") << std::endl
            << "  Monitor Rectangle (Virtual Screen Coordinates):" << std::endl
            << "    Left: " << monitorInfo.rcMonitor.left << std::endl
            << "    Top: " << monitorInfo.rcMonitor.top << std::endl
            << "    Right: " << monitorInfo.rcMonitor.right << std::endl
            << "    Bottom: " << monitorInfo.rcMonitor.bottom << std::endl
            << "  Work Area Rectangle (Virtual Screen Coordinates):" << std::endl
            << "    Left: " << monitorInfo.rcWork.left << std::endl
            << "    Top: " << monitorInfo.rcWork.top << std::endl
            << "    Right: " << monitorInfo.rcWork.right << std::endl
            << "    Bottom: " << monitorInfo.rcWork.bottom << std::endl;

    auto wide = message.str();
    std::cout << std::string(wide.begin(), wide.end());
}
} // namespace Helpers
