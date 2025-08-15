#include "InputController.h"

#include <windowsx.h>

using namespace Input;

InputController::InputController()
{
}

void InputController::OnWindowMessage(HWND hwnd, Message message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case Message::IDLE:
        break;
    case Message::LBUTTONDOWN:
    case Message::LBUTTONUP:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        std::cout << "LBUTTON";
    }
    break;

    case Message::RBUTTONDOWN:
    case Message::RBUTTONUP:
    {
        std::cout << "RBUTTON";
    }
    break;

    case Message::MOUSEMOVE:
    {
        GetCursorPos(&m_mousePos);
        ScreenToClient(hwnd, &m_mousePos);
    }
    break;

    case Message::MOUSEWHEEL:
    {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        std::cout << "MOUSEWHEEL ";
    }
    break;

    default:
        break;
    }
}