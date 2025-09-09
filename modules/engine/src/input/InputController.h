#pragma once

#include "../canvas/Models.h"
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
    MOUSEWHEEL,
    INPUT
};

class InputController final
{
public:
    // Disallow copy / assign
    InputController(const InputController&) = delete;
    InputController& operator=(const InputController&) = delete;

    InputController(window::WindowStateReducer*) noexcept;

    void Initialize(HWND hwnd);

    Int2 MousePosition() noexcept { return m_mousePos; }
    Int2 MouseDelta() noexcept;
    DirectX::XMFLOAT2 MousePositionNorm() noexcept;

    bool IsKeyPressed(int vkCode) noexcept;

    void OnWindowMessage(HWND hwnd, Message message, WPARAM wParam, LPARAM lParam);

private:
    Int2 m_mousePos;
    Int2 m_mouseDelta;

    window::WindowStateReducer* m_stateReducer;
};
} // namespace input