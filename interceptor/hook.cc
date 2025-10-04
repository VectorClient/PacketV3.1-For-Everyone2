#include "hook.h"

#include <Windows.h>
#include <libloaderapi.h>
#include <stdint.h>

#include <cstdint>
#include <cstdio>
#include <thread>
#include <unordered_map>

#include "MinHook.h"

constexpr uint32_t start_address_offset = 0x247E70;
constexpr uint32_t func_map_offset = 0x2FA340;
constexpr uint32_t offset_map_offset = 0x2FA380;

HANDLE WINAPI Hooks::create_thread_detour(LPSECURITY_ATTRIBUTES lpThreadAttributes,
										  SIZE_T dwStackSize,
										  LPTHREAD_START_ROUTINE lpStartAddress,
										  LPVOID lpParameter,
										  DWORD dwCreationFlags,
										  LPDWORD lpThreadId) {
	uint64_t predicatedBaseAddress = reinterpret_cast<uint64_t>(lpStartAddress) - start_address_offset;
	if (predicatedBaseAddress == reinterpret_cast<uint64_t>(lpParameter)) {	 // CreateThread(0LL, 0LL, StartAddress, hinstDLL, 0, 0LL);
		printf("found the dll: %llx\n", predicatedBaseAddress);
		intercept(predicatedBaseAddress);
	}
	return originalCreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

void Hooks::init() {
	MH_Initialize();

	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	void* createThread = reinterpret_cast<void*>(GetProcAddress(kernel32, "CreateThread"));
	if (!createThread) {
		printf("error\n");
		return;
	}
	MH_CreateHook(createThread, reinterpret_cast<void*>(create_thread_detour), reinterpret_cast<void**>(&originalCreateThread));
	MH_EnableHook(createThread);
	printf("waiting\n");
}

void Hooks::intercept(uint64_t baseAddr) {
	std::thread t([=]() {
		std::this_thread::sleep_for(std::chrono::seconds(5));

		printf("\n\rfunc:\n");
		uint64_t mcBaseAddress = reinterpret_cast<uint64_t>(GetModuleHandleA("Minecraft.Windows.exe"));
		auto funcMapPtr = reinterpret_cast<std::unordered_map<uint32_t, uint64_t>*>(baseAddr + func_map_offset);
		for (const auto& pair : *funcMapPtr) {
			printf("%d %llx\n", pair.first, pair.second - mcBaseAddress);
		}

		printf("\n\roffset:\n");
		auto offsetMapPtr = reinterpret_cast<std::unordered_map<uint32_t, int32_t>*>(baseAddr + offset_map_offset);
		for (const auto& pair : *offsetMapPtr) {
			printf("%d %x\n", pair.first, pair.second);
		}
	});
	t.detach();
	printf("detached\n");
}
