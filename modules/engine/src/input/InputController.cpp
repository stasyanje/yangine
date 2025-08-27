#include "InputController.h"
#include "../window/WindowStateReducer.h"

#include <windowsx.h>

using namespace input;
using namespace DirectX;

InputController::InputController(
    window::WindowStateReducer* stateReducer
) noexcept :
    m_stateReducer(stateReducer)
{
}

XMFLOAT2 InputController::MousePositionNorm() noexcept
{
    // Get window size to convert pixels to normalized device coordinates
    auto outputSize = m_stateReducer->getBounds();
    float windowWidth = static_cast<float>(outputSize.right - outputSize.left);
    float windowHeight = static_cast<float>(outputSize.bottom - outputSize.top);

    // Convert mouse position from pixels to NDC (-1 to 1 range)

    XMFLOAT2 point;

    point.x = (m_mousePos.x / windowWidth) * 2.0 - 1.0;
    point.y = -((m_mousePos.y / windowHeight) * 2.0 - 1.0);

    return point;
};

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