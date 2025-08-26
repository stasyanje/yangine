#pragma once

#include "canvas/Renderer.h"
#include "input/InputController.h"
#include "pch.h"
#include "window/WindowStateReducer.h"

class WindowManager
{
public:
    WindowManager() noexcept = default;
    ~WindowManager() noexcept;

    HWND Initialize(
        HINSTANCE hInstance,
        window::WindowStateReducer*,
        canvas::Renderer*,
        input::InputController*,
        int nCmdShow
    ) noexcept(false);

    void Idle();

    void OnWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    bool RegisterWindowClass();
    HWND CreateRendererWindow(int nCmdShow);

    input::Message InputMessage(UINT message);
    canvas::Message CanvasMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;

    window::WindowStateReducer* m_stateReducer = nullptr;
    canvas::Renderer* m_renderer = nullptr;
    input::InputController* m_inputController = nullptr;
};