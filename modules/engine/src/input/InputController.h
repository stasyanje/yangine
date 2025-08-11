#pragma once

namespace Input
{
enum class Message
{
    IDLE,
    MOUSEMOVE,
    LBUTTONDOWN,
    LBUTTONUP,
    RBUTTONDOWN,
    RBUTTONUP,
    MBUTTONDOWN,
    MBUTTONUP,
    MOUSEWHEEL
};

class InputController final
{
public:
    InputController();

    void OnWindowMessage(Message message, WPARAM wParam, LPARAM lParam);

    int GetMouseX() const { return mouseX; }
    int GetMouseY() const { return mouseY; }

private:
    int mouseX;
    int mouseY;
};
} // namespace Input