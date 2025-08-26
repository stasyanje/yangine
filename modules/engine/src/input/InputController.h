#pragma once

#include "../common/StepTimer.h"

namespace input
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
    // Disallow copy / assign
    InputController(const InputController&) = delete;
    InputController& operator=(const InputController&) = delete;

    InputController();

    POINT MousePosition() { return m_mousePos; }

    void OnWindowMessage(HWND hwnd, Message message, WPARAM wParam, LPARAM lParam);

private:
    DX::StepTimer m_timer;
    HWND m_hwnd;
    POINT m_mousePos;
};
} // namespace input