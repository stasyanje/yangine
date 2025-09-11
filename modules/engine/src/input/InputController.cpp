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

void InputController::Initialize(HWND hwnd)
{
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.hwndTarget = hwnd;
    auto success = RegisterRawInputDevices(&rid, 1, sizeof(rid));
    assert(success && "RegisterRawInputDevices");
}

XMFLOAT2 InputController::MousePositionNorm() noexcept
{
    // Get window size to convert pixels to normalized device coordinates
    auto outputRect = m_stateReducer->getBounds();
    float windowWidth = static_cast<float>(outputRect.right - outputRect.left);
    float windowHeight = static_cast<float>(outputRect.bottom - outputRect.top);

    // Convert mouse position from pixels to NDC (-1 to 1 range)

    XMFLOAT2 point;

    point.x = (m_mousePos.x / windowWidth) * 2.0 - 1.0;
    point.y = (m_mousePos.y / windowHeight) * 2.0 - 1.0;

    return point;
};

Int2 InputController::CollectMouseDelta() noexcept
{
    if (IsZero(m_mouseDelta))
        return m_mouseDelta;

    Int2 point = m_mouseDelta;

    // Reset delta since it's been collected
    m_mouseDelta.x = 0;
    m_mouseDelta.y = 0;

    return point;
}

bool InputController::IsKeyPressed(int vkCode) noexcept
{
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
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

    case Message::INPUT:
    {
        UINT size = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
        std::vector<BYTE> buf(size);

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf.data(), &size, sizeof(RAWINPUTHEADER)) != size)
        {
            return;
        }

        RAWINPUT* ri = reinterpret_cast<RAWINPUT*>(buf.data());
        if (ri->header.dwType != RIM_TYPEMOUSE)
        {
            return;
        }

        if (ri->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
        {
            return;
        }

        m_mouseDelta.x += ri->data.mouse.lLastX;
        m_mouseDelta.y += ri->data.mouse.lLastY;
    }

    case Message::MOUSEMOVE:
    {
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(hwnd, &mousePos);

        m_mousePos.x = mousePos.x;
        m_mousePos.y = mousePos.y;
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