#include <Windows.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>

#include <cstdio>
#include <filesystem>
#include <string>

#include "injector.h"

#define LOADER_FILE_NAME "loader.dll"
#define PACKET_V3_FILE_NAME "packetv3.dll"

bool isAvx2Supported() {
	int info[4];
	__cpuidex(info, 0, 0);
	if (info[0] < 7) {
		return false;
	}
	return (info[1] & (1 << 5)) != 0;
}

DWORD findPidByName(const std::wstring& processName) {
	DWORD processId = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}
	PROCESSENTRY32W processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32W);
	if (Process32FirstW(snapshot, &processEntry)) {
		do {
			if (processName == processEntry.szExeFile) {
				processId = processEntry.th32ProcessID;
				break;
			}
		} while (Process32NextW(snapshot, &processEntry));
	}
	CloseHandle(snapshot);
	return processId;
}

bool launch() {
	if (isAvx2Supported()) {
		printf("[-] Your processor doesn't support AVX-2\n");
		return false;
	}

	DWORD pid = findPidByName(L"Minecraft.Windows.exe");
	if (!pid) {
		printf("[+] Failed to find minecraft process\n");
		return false;
	}
	printf("[+] Found minecraft pid: %lu\n", pid);

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!handle) {
		printf("[-] Failed to open the minecraft process\n");
		return false;
	}

	printf("[*] Loading\n");

	// Loader
	std::filesystem::path loaderPath = std::filesystem::canonical(LOADER_FILE_NAME);
	if (!std::filesystem::exists(loaderPath)) {
		printf("[-] Loader dll doesn't exists\n");
		return false;
	}
	Injector::grant(loaderPath.wstring());
	Injector::inject(handle, loaderPath.wstring());
	printf("[+] Loaded the loader\n");

	Sleep(1000);

	// PacketV3
	std::filesystem::path packetv3Path = std::filesystem::canonical(PACKET_V3_FILE_NAME);
	if (!std::filesystem::exists(packetv3Path)) {
		printf("[-] PacketV3 dll doesn't exists\n");
		getchar();
		return false;
	}
	Injector::grant(packetv3Path.wstring());
	Injector::inject(handle, packetv3Path.wstring());
	printf("[+] Loaded the client\n");

	printf("\n\rDone! Press enter to close this window\n");
	return true;
}

int main(void) {
	bool result = launch();
	getchar();
	return result ? 0 : -1;
}
