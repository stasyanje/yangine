//
// Main.cpp
//

#include "pch.h"
#include "include/yangine.h"
#include <iostream>
#include <fstream>

#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

extern "C"
{
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

LPCWSTR g_szAppName = L"direct-3d-playground";

void ExitGame() noexcept;

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
    HANDLE process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    // Сохраняем стек трейc
    void* stack[64];
    USHORT frames = CaptureStackBackTrace(0, 64, stack, nullptr);

    for (USHORT i = 0; i < frames; ++i)
    {
        DWORD64 address = (DWORD64)(stack[i]);

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;

        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        if (SymFromAddr(process, address, 0, symbol))
        {
            std::cout << "  " << symbol->Name << " at " << (void*)symbol->Address;
        }
        else
        {
            std::cout << "  (no symbol) at " << (void*)address;
        }

        IMAGEHLP_LINE64 line;
        DWORD displacementLine = 0;
        ZeroMemory(&line, sizeof(line));
        line.SizeOfStruct = sizeof(line);

        if (SymGetLineFromAddr64(process, address, &displacementLine, &line))
        {
            std::cout << " (" << line.FileName << ":" << std::dec << line.LineNumber << ")";
        }

        std::cout << std::endl;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

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

    log("wWinMain");

    SetUnhandledExceptionFilter(CrashHandler);

    Application* app = yangine::CreateApplication();
    int result = yangine::RunApplication(app, hInstance, nCmdShow);
    yangine::DestroyApplication(app);
    return result;
}

// Exit helper
void ExitGame() noexcept
{
    yangine::ExitGame();
}
