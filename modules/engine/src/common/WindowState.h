#pragma once

struct WindowState
{
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    bool in_sizemove = false;
    bool in_suspend = false;
    bool minimized = false;
};