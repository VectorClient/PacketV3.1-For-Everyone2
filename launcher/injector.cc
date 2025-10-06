#include "injector.h"

#include <vector>

void Injector::grant(std::wstring path) {
	std::wstring cmd = L"cmd.exe /c icacls \"";
	cmd += path;
	cmd += L"\" /grant \"ALL APPLICATION PACKAGES\":(F)";
	std::vector<wchar_t> cmdLine(cmd.begin(), cmd.end());
	cmdLine.push_back(0);
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	BOOL ok = CreateProcessW(
		NULL,
		cmdLine.data(),
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi);
	if (!ok) {
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return;
}

void Injector::inject(HANDLE processHandle, std::wstring path) {
	size_t memSize = path.length() * 2 + 2;
	LPVOID pDllPath = VirtualAllocEx(processHandle, 0, memSize, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(processHandle, pDllPath, path.c_str(), memSize, 0);
	HANDLE hLoadThread = CreateRemoteThread(processHandle, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryW), pDllPath, 0, 0);
	WaitForSingleObject(hLoadThread, INFINITE);
}
