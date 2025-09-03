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

    DirectX::XMFLOAT2 MousePosition() noexcept { return m_mousePos; }
    DirectX::XMFLOAT2 MousePositionNorm() noexcept;
    DirectX::XMFLOAT2 MouseDeltaNorm() noexcept;

    bool IsKeyPressed(int vkCode) noexcept;

    void OnWindowMessage(HWND hwnd, Message message, WPARAM wParam, LPARAM lParam);

private:
    DirectX::XMFLOAT2 m_mousePos;
    DirectX::XMFLOAT2 m_mouseDelta;

    window::WindowStateReducer* m_stateReducer;
};
} // namespace input