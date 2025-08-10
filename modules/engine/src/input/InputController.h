#pragma once

class InputController
{
public:
    InputController();

    enum class Message
    {
        MOUSEMOVE = WM_MOUSEMOVE,
        LBUTTONDOWN = WM_LBUTTONDOWN,
        LBUTTONUP = WM_LBUTTONUP,
        RBUTTONDOWN = WM_RBUTTONDOWN,
        RBUTTONUP = WM_RBUTTONUP,
        MBUTTONDOWN = WM_MBUTTONDOWN,
        MBUTTONUP = WM_MBUTTONUP,
        MOUSEWHEEL = WM_MOUSEWHEEL
    };

    void Tick(Message, WPARAM wParam, LPARAM lParam);

    int GetMouseX() const { return mouseX; }
    int GetMouseY() const { return mouseY; }

private:
    int mouseX;
    int mouseY;
};