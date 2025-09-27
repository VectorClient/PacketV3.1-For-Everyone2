#pragma once

#include <Windows.h>
#include <minwindef.h>
#include <winnt.h>

#include <cstdint>

#include "kiero/Kiero.h"

class ClientInstance;
class Loader {
	typedef HRESULT(__fastcall* PresentFunc)(IDXGISwapChain*, UINT, UINT);
	static inline PresentFunc originalPresent;
	static HRESULT present_detour(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

	typedef void(__fastcall* UpdateFunc)(ClientInstance*, bool);
	static inline UpdateFunc originalUpdate;
	static void update_detour(ClientInstance* ci, bool flag);

	static void storeVariables(uint64_t handle);

public:
	static void init();

	static HMODULE getPacketHandle();
};
