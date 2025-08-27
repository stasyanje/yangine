//
// Main.cpp
//

#include "ExceptionFilter.cpp"

#include <IEngine.h>
#include <Windows.h>

void ExitGame() noexcept;

// Entry point
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    SetUnhandledExceptionFilter(ExceptionFilter);

    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    Engine* engine = IEngine::Create();
    int result = IEngine::Run(engine, hInstance, nCmdShow);
    IEngine::Destroy(engine);
    return result;
}

// Exit helper
void ExitGame() noexcept
{
    IEngine::ExitGame();
}
