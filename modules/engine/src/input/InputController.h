#pragma once

#include "../common/StepTimer.h"
#include "../pch.h"
#include "../window/WindowStateReducer.h"

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

    InputController(window::WindowStateReducer*) noexcept;

    POINT MousePosition() noexcept { return m_mousePos; }
    DirectX::XMFLOAT2 MousePositionNorm() noexcept;

    void OnWindowMessage(HWND hwnd, Message message, WPARAM wParam, LPARAM lParam);

private:
    DX::StepTimer m_timer;
    HWND m_hwnd;
    POINT m_mousePos;

    window::WindowStateReducer* m_stateReducer;
};
} // namespace input