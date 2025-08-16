#include "WindowManager.h"
#include "pch.h"

namespace App
{
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
} // namespace App