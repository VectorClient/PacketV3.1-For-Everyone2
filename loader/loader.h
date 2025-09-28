#pragma once

#include <Windows.h>
#include <dxgiformat.h>
#include <minwindef.h>
#include <winnt.h>

#include <cstdint>
#include <optional>
#include <string>

#include "kiero/Kiero.h"

class ClientInstance;
class Loader {
	static inline HMODULE dllHandle = 0;
	static inline uint64_t clientHandle = 0;

	////////////////////////////////////////////////////////

	typedef HRESULT(__fastcall* PresentFunc)(IDXGISwapChain*, UINT, UINT);
	static inline PresentFunc originalPresent;
	static HRESULT present_detour(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

	typedef HRESULT(__fastcall* ResizeBuffersFunc)(IDXGISwapChain*, UINT, UINT, DXGI_FORMAT, UINT);
	static inline ResizeBuffersFunc originalResizeBuffers;
	static HRESULT resize_buffers_detour(IDXGISwapChain* swapChain, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);

	////////////////////////////////////////////////////////////

	typedef void(__fastcall* UpdateFunc)(ClientInstance*, bool);
	static inline UpdateFunc originalUpdate;
	static void update_detour(ClientInstance* ci, bool flag);

	static void storeVariables(uint64_t handle);

	static void remap(uint64_t handle);

public:
	static void init(HMODULE dllHandle);

private:
	static void loadDll();

	static std::optional<std::string> prepareDll();
};
