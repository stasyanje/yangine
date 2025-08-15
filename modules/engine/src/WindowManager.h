#pragma once

#include "Renderer.h"
#include "common/WindowState.h"
#include "input/InputController.h"
#include "pch.h"

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();

    bool Initialize(HINSTANCE hInstance, int nCmdShow, Canvas::Renderer* renderer, Input::InputController* inputController);
    void Idle();
    void Shutdown();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    WindowState m_windowState;

    HWND m_hwnd;
    HINSTANCE m_hInstance;
    Canvas::Renderer* m_renderer;
    Input::InputController* m_inputController;

    bool RegisterWindowClass();
    bool CreateRendererWindow(int nCmdShow);

    void OnWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    Input::Message InputMessage(UINT message);
    Canvas::Message CanvasMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void ToggleFullscreen();
};
