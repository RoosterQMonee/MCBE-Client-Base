#include <Windows.h>
#include <thread>

#include <MCBase/MCBase.h>


BOOL APIENTRY DllMain(HINSTANCE inst, DWORD reason, [[maybe_unused]] void* reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);

        std::thread([=]() {
            FILE* fDummy;
            FreeLibrary(inst);
            
            // Create Console
            AllocConsole();
            AttachConsole(GetCurrentProcessId());
            
            // Open streams
            freopen_s(&fDummy, "CONIN$", "r", stdin);
            freopen_s(&fDummy, "CONOUT$", "w", stderr);
            freopen_s(&fDummy, "CONOUT$", "w", stdout);

            // Additional things
            HANDLE hConOut = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
            SetConsoleTitleA("Debug Console");

            Client.get()->Init(inst);
        }).detach();
    }

    return TRUE;
}