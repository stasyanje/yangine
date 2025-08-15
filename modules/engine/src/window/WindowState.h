#pragma once

namespace window
{
struct WindowState
{
    RECT monitorBounds = {0, 0, 0, 0};
    RECT bounds = {0, 0, 0, 0};

    bool fullscreen = false;
    bool in_sizemove = false;
    bool in_suspend = false;
    bool minimized = false;
};
} // namespace window
