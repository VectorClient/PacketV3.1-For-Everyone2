#include <Windows.h>
#include <processthreadsapi.h>

#include <cstdio>

#include "loader.h"

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID) {
	if (fdwReason != DLL_PROCESS_ATTACH) {
		return TRUE;
	}

	AllocConsole();
	FILE* newStdout;
	FILE* newStdin;
	freopen_s(&newStdout, "CONOUT$", "w", stdout);
	freopen_s(&newStdin, "CONIN$", "r", stdin);

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Loader::init, hModule, NULL, NULL);

	return TRUE;
}
