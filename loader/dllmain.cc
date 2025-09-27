#include <Windows.h>
#include <processthreadsapi.h>

#include <cstdio>

#include "loader.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	if (fdwReason != DLL_PROCESS_ATTACH) {
		return TRUE;
	}

	AllocConsole();
	FILE* newStdout;
	FILE* newStdin;
	freopen_s(&newStdout, "CONOUT$", "w", stdout);
	freopen_s(&newStdin, "CONIN$", "r", stdin);

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Loader::init, NULL, NULL, NULL);

	return TRUE;
}
