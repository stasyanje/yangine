#pragma once

#include "canvas/Renderer.h"
#include "input/InputController.h"
#include "pch.h"
#include "window/WindowStateReducer.h"

class WindowManager
{
public:
    WindowManager();
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
    HWND m_hwnd;
    HINSTANCE m_hInstance;
    window::WindowStateReducer* m_stateReducer;
    canvas::Renderer* m_renderer;
    input::InputController* m_inputController;

    bool RegisterWindowClass();
    HWND CreateRendererWindow(int nCmdShow);

    input::Message InputMessage(UINT message);
    canvas::Message CanvasMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};