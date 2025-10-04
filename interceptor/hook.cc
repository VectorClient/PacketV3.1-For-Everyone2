#include "hook.h"

#include <Windows.h>
#include <libloaderapi.h>
#include <stdint.h>

#include <cstdint>
#include <cstdio>
#include <thread>
#include <unordered_map>

#include "MinHook.h"

constexpr uint32_t start_address_offset = 0x4532A0;
constexpr uint32_t func_map_offset = 0x51E358;
constexpr uint32_t offset_map_offset = 0x5EF5E8;

static bool setup;

struct FunctionEntry {
	void* unknown_1;
	void* unknown_2;
	uint64_t value;
};

struct OffsetEntry {
	void* unknown_1;
	void* unknown_2;
	uint32_t value;
};

HANDLE WINAPI Hooks::create_thread_detour(LPSECURITY_ATTRIBUTES lpThreadAttributes,
										  SIZE_T dwStackSize,
										  LPTHREAD_START_ROUTINE lpStartAddress,
										  LPVOID lpParameter,
										  DWORD dwCreationFlags,
										  LPDWORD lpThreadId) {
	uint64_t predicatedBaseAddress = reinterpret_cast<uint64_t>(lpStartAddress) - start_address_offset;
	if (predicatedBaseAddress < 0x700000000000 && !setup) {
		printf("found the dll: %llx\n", predicatedBaseAddress);
		intercept(predicatedBaseAddress);
		setup = true;
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

		uint64_t mcBaseAddress = reinterpret_cast<uint64_t>(GetModuleHandleA("Minecraft.Windows.exe"));

		printf("\n\rfunc:\n");
		auto funcMapPtr = reinterpret_cast<std::unordered_map<uint32_t, FunctionEntry*>*>(baseAddr + func_map_offset);
		for (const auto& pair : *funcMapPtr) {
			pair.second->unknown_1 = 0;
			pair.second->unknown_2 = 0;
			printf("%d %llx\n", pair.first, pair.second->value - mcBaseAddress);
		}

		printf("\n\roffset:\n");
		auto offsetMapPtr = reinterpret_cast<std::unordered_map<uint32_t, OffsetEntry*>*>(baseAddr + offset_map_offset);
		for (const auto& pair : *offsetMapPtr) {
			pair.second->unknown_1 = 0;
			pair.second->unknown_2 = 0;
			printf("%d %x\n", pair.first, pair.second->value);
		}
	});
	t.detach();
	printf("detached\n");
}
