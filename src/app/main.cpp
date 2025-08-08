//
// Main.cpp
//

#include "ExceptionFilter.cpp"

#include <IEngine.h>
#include <Windows.h>

#define WIN32_LEAN_AND_MEAN

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

    Engine* app = IEngine::Create();
    int result = IEngine::Run(app, hInstance, nCmdShow);
    IEngine::Destroy(app);
    return result;
}

// Exit helper
void ExitGame() noexcept
{
    IEngine::ExitGame();
}
