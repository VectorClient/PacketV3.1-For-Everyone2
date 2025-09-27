#pragma once

#include <Windows.h>

#include <cstdint>
#include <unordered_map>

class Hooks {
	typedef HANDLE(WINAPI* CreateThreadFunc)(LPSECURITY_ATTRIBUTES,
											 SIZE_T,
											 LPTHREAD_START_ROUTINE,
											 LPVOID,
											 DWORD,
											 LPDWORD);
	static inline CreateThreadFunc originalCreateThread;
	static HANDLE WINAPI create_thread_detour(LPSECURITY_ATTRIBUTES,
											  SIZE_T,
											  LPTHREAD_START_ROUTINE,
											  LPVOID,
											  DWORD,
											  LPDWORD);

public:
	static void init();

	static void intercept(uint64_t baseAddr);
};
