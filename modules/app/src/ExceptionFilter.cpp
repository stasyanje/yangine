#include <Windows.h>

#include <dbghelp.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "Dbghelp.lib")

LPCWSTR g_szAppName = L"direct-3d-playground";

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo)
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