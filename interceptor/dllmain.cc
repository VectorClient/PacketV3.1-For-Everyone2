#include <Windows.h>
#include <processthreadsapi.h>

#include <cstdio>

#include "hook.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	if (fdwReason != DLL_PROCESS_ATTACH) {
		return TRUE;
	}

	AllocConsole();
	FILE* newStdout;
	FILE* newStdin;
	freopen_s(&newStdout, "CONOUT$", "w", stdout);
	freopen_s(&newStdin, "CONIN$", "r", stdin);

	CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Hooks::init), NULL, NULL, NULL);

	return TRUE;
}
