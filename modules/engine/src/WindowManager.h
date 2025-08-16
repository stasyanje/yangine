#pragma once

#include "Renderer.h"
#include "input/InputController.h"
#include "pch.h"
#include "window/WindowStateReducer.h"

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();

    HWND Initialize(
        HINSTANCE hInstance,
        int nCmdShow,
        window::WindowStateReducer*,
        Canvas::Renderer*,
        Input::InputController*
    ) noexcept(false);

    void Idle();
    void Shutdown();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd;
    HINSTANCE m_hInstance;
    window::WindowStateReducer* m_stateReducer;
    Canvas::Renderer* m_renderer;
    Input::InputController* m_inputController;

    bool RegisterWindowClass();
    HWND CreateRendererWindow(int nCmdShow);

    void OnWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    Input::Message InputMessage(UINT message);
    Canvas::Message CanvasMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
