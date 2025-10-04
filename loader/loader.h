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
	static inline uint64_t packetv3Handle = 0;

	////////////////////////////////////////////////////////

	typedef HRESULT(__fastcall* PresentFunc)(IDXGISwapChain*, UINT, UINT);
	static inline PresentFunc originalPresent;
	static HRESULT presentDetour(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

	typedef HRESULT(__fastcall* ResizeBuffersFunc)(IDXGISwapChain*, UINT, UINT, DXGI_FORMAT, UINT);
	static inline ResizeBuffersFunc originalResizeBuffers;
	static HRESULT resizeBuffersDetour(IDXGISwapChain* swapChain, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);

	typedef HANDLE(WINAPI* CreateThreadFunc)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
	static inline CreateThreadFunc originalCreateThread;
	static HANDLE WINAPI createThreadDetour(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);

	typedef DWORD(WINAPI* SendFunc)(HANDLE, LPCSTR, DWORD, DWORD);
	static inline SendFunc originalSend;
	static DWORD WINAPI sendDetour(HANDLE, LPCSTR, DWORD, DWORD);

	////////////////////////////////////////////////////////////

	typedef void(__fastcall* UpdateFunc)(ClientInstance*, bool);
	static inline UpdateFunc originalUpdate;
	static void updateDetour(ClientInstance* ci, bool flag);

	static void storeVariables(uint64_t handle);

public:
	static void init(HMODULE dllHandle);

private:
	static void loadDll();

	static std::optional<std::string> prepareDll();
};
