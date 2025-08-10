#include "InputController.h"

#include <windowsx.h>

InputController::InputController()
{
}

void InputController::Tick(Message message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case Message::LBUTTONDOWN:
    case Message::LBUTTONUP:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        // std::cout << "LBUTTON " + std::to_string(xPos);
    }
    break;

    case Message::RBUTTONDOWN:
    case Message::RBUTTONUP:
    {
        std::cout << ("RBUTTON " + std::to_string(mouseX));
    }
    break;

    case Message::MOUSEMOVE:
    {
        mouseX = GET_X_LPARAM(lParam);
        mouseY = GET_Y_LPARAM(lParam);
    }
    break;

    case Message::MOUSEWHEEL:
    {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        // std::cout << "MOUSEWHEEL" + std::to_string(zDelta);
    }
    break;

    default:
        break;
    }
}