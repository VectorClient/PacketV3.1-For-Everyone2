#include "loader.h"

#include <Minhook.h>
#include <Windows.h>
#include <consoleapi3.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <winuser.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "MinHook.h"
#include "files.h"
#include "kiero/Kiero.h"
#include "remapper.hpp"
#include "resource/resource.h"
#include "scanner.h"

class ClientInstance;
ClientInstance* clientInstance;

static bool initialized = false;
static bool paused = false;

/* dx11 vtable index */
constexpr uint32_t present_table_index = 8;
constexpr uint32_t resize_buffers_table_index = 13;
/* internal function */
constexpr uint32_t start_address_offset = 0x4532A0;
constexpr uint32_t minhook_initialize_func_offset = 0x4575E0;
/* rendering */
constexpr uint32_t present_hook_offset = 0x448450;
constexpr uint32_t resize_buffers_hook_offset = 0x44CD40;
constexpr uint32_t present_original_callback_offset = 0x5F2228;
constexpr uint32_t resize_buffers_original_callback_offset = 0x5F2220;
/* variables */
constexpr uint32_t user_info_offset = 0x527220;
constexpr uint32_t client_instance_offset = 0x5EF7B8;
constexpr uint32_t func_map_offset = 0x51E358;
constexpr uint32_t offset_map_offset = 0x5EF5E8;
/* connection shit collection */
constexpr uint32_t connection_status_offset = 0x5F14C8;
constexpr uint32_t connection_status_2_offset = 0x5F14D0;
constexpr uint32_t connection_status_3_offset = 0x528F88;

constexpr uint64_t fake_socket_handle = 0xfacefeedb105f00d;

HRESULT Loader::presentDetour(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
	if (!packetv3Handle || !clientInstance) {
		return originalPresent(swapChain, syncInterval, flags);
	}

	PresentFunc func = reinterpret_cast<PresentFunc>(packetv3Handle + present_hook_offset);
	if (!initialized) {
		printf("[+] Caught the handle: handle=%llu, target=%p\n", packetv3Handle, func);

		// hooks
		typedef void(__fastcall * MinhookInitializeFunc)();
		reinterpret_cast<MinhookInitializeFunc>(packetv3Handle + minhook_initialize_func_offset)();
		printf("[+] Initialized hook\n");

		// variables
		storeVariables(packetv3Handle);
		printf("[+] Stored globals\n");

		// remap
		Remapper::remapFunction(reinterpret_cast<UnorderedMap>(packetv3Handle + func_map_offset));
		Remapper::remapOffset(reinterpret_cast<UnorderedMap>(packetv3Handle + offset_map_offset));
		printf("[+] Remapped\n");

		initialized = true;
		ShowWindow(GetConsoleWindow(), SW_HIDE);
	}

	return func(swapChain, syncInterval, flags);
}

HRESULT Loader::resizeBuffersDetour(IDXGISwapChain* swapChain, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
	if (!packetv3Handle) {
		return originalResizeBuffers(swapChain, width, height, newFormat, swapChainFlags);
	}
	ResizeBuffersFunc func = reinterpret_cast<ResizeBuffersFunc>(packetv3Handle + resize_buffers_hook_offset);
	return func(swapChain, width, height, newFormat, swapChainFlags);
}

HANDLE WINAPI Loader::createThreadDetour(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
	uint64_t predicatedBaseAddress = reinterpret_cast<uint64_t>(lpStartAddress) - start_address_offset;
	if (predicatedBaseAddress < 0x700000000000 && !paused) {
		printf("[!] Found the dll: %llx\n", predicatedBaseAddress);
		printf("[!] Cancelled the thread\n");
		packetv3Handle = predicatedBaseAddress;
		paused = true;
		return NULL;
	}
	return originalCreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

DWORD WINAPI Loader::sendDetour(HANDLE socket, LPCSTR content, DWORD len, DWORD flags) {
	if (reinterpret_cast<uint64_t>(socket) == fake_socket_handle) {
		return 0;
	}
	return originalSend(socket, content, len, flags);
}

void Loader::updateDetour(ClientInstance* clientInstance, bool flag) {
	if (!::clientInstance) {
		printf("[+] Retrieved CI Instance: %p\n", clientInstance);
		::clientInstance = clientInstance;
	}
	return originalUpdate(clientInstance, flag);
}

void Loader::storeVariables(uint64_t handle) {
	// original present
	{
		void** target = reinterpret_cast<void**>(handle + present_original_callback_offset);
		*target = reinterpret_cast<void*>(originalPresent);
	}
	// original resize buffers
	{
		void** target = reinterpret_cast<void**>(handle + resize_buffers_original_callback_offset);
		*target = reinterpret_cast<void*>(originalResizeBuffers);
	}
	// user info
	{
		void** target = reinterpret_cast<void**>(handle + user_info_offset);
		char* userInfo = static_cast<char*>(malloc(0x1337));
		new (userInfo + 0x140) std::string("1337");
		*target = userInfo;
	}
	// client instance
	{
		void** target = reinterpret_cast<void**>(handle + client_instance_offset);
		*target = reinterpret_cast<void*>(clientInstance);
	}
	// connection info 1
	{
		int8_t* status = static_cast<int8_t*>(malloc(0x1337));
		*reinterpret_cast<uint64_t*>(status + 0x18) = 0x1337;
		*reinterpret_cast<bool*>(status + 0x20) = 1;
		*reinterpret_cast<bool*>(status + 0x21) = 1;
		*reinterpret_cast<int32_t*>(status + 0x28) = 0x1337;

		int8_t* status2 = static_cast<int8_t*>(malloc(0xbeef));
		*reinterpret_cast<void**>(status2 + 0x3A8) = status;

		void** target = reinterpret_cast<void**>(handle + connection_status_offset);
		*target = status2;
	}
	// connection info 2
	{
		uint64_t* target = reinterpret_cast<uint64_t*>(handle + connection_status_2_offset);
		*target = 0xcafebabe8badf00d;
	}
	// connection info 3
	{
		int8_t* container = static_cast<int8_t*>(malloc(0x1337));
		*reinterpret_cast<uint64_t*>(container + 0x98) = fake_socket_handle;
		*reinterpret_cast<bool*>(container + 0xA0) = 1;
		*reinterpret_cast<bool*>(container + 0xA1) = 1;
		*reinterpret_cast<int32_t*>(container + 0xA8) = 0x1337;

		int8_t* container2 = static_cast<int8_t*>(malloc(0xbeef));
		*reinterpret_cast<void**>(container2 + 0xB68) = container;

		void** target = reinterpret_cast<void**>(handle + connection_status_3_offset);
		*target = container2;
	}
}

void Loader::init(HMODULE dllHandle) {
	std::filesystem::create_directory(Files::getRoamingPath() + "\\Packet 3.1");
	Loader::dllHandle = dllHandle;

	MH_Initialize();
	// CreateThread
	{
		HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
		void* createThread = reinterpret_cast<void*>(GetProcAddress(kernel32, "CreateThread"));
		if (!createThread) {
			printf("[-] Unknown Error\n");
			return;
		}
		MH_CreateHook(createThread, reinterpret_cast<void*>(createThreadDetour), reinterpret_cast<void**>(&originalCreateThread));
		MH_EnableHook(createThread);
		printf("[!] Created thread hook\n");
	}
	// Socket
	{
		HMODULE winsocks = GetModuleHandleA("ws2_32.dll");
		void* send = reinterpret_cast<void*>(GetProcAddress(winsocks, "send"));
		if (!send) {
			printf("[-] Unknown Error\n");
			return;
		}
		MH_CreateHook(send, reinterpret_cast<void*>(sendDetour), reinterpret_cast<void**>(&originalSend));
		MH_EnableHook(send);
		printf("[!] Created send hook\n");
	}
	// DX11
	{
		auto kieroStatus = Kiero::initialize(Kiero::RenderType::D3D11);
		if (kieroStatus != Kiero::Status::Success) {
			printf("[-] Couldn't initialize kiero: code=%d\n", kieroStatus);
			return;
		}
		void* presentPtr = reinterpret_cast<void*>(Kiero::getMethodsTable()[present_table_index]);
		MH_CreateHook(presentPtr, reinterpret_cast<void*>(&presentDetour), reinterpret_cast<void**>(&originalPresent));
		MH_EnableHook(presentPtr);

		void* resizePtr = reinterpret_cast<void*>(Kiero::getMethodsTable()[resize_buffers_table_index]);
		MH_CreateHook(resizePtr, reinterpret_cast<void*>(&resizeBuffersDetour), reinterpret_cast<void**>(&originalResizeBuffers));
		MH_EnableHook(resizePtr);
		printf("[!] Created present hook\n");
	}
	// CI
	{
		void* ci_update = Scanner::scanPattern(
			"48 89 5C 24 10 48 89 74 24 18 55 57 41 54 41 56 41 57 48 8D AC 24 00 FC FF FF 48 81 EC 00 05 00 00 48");  // is it long?
		if (!ci_update) {
			printf("[-] Couldnt find ClientInstance::update\n");
			return;
		}
		MH_CreateHook(ci_update, reinterpret_cast<void*>(&updateDetour), reinterpret_cast<void**>(&originalUpdate));
		MH_EnableHook(ci_update);
		printf("[!] Created CI hook\n");
	}

	loadDll();
}

void Loader::loadDll() {
	std::optional<std::string> path = prepareDll();
	if (!path) {
		printf("[-] Failed to load client data\n");
		return;
	}
	HANDLE result = LoadLibraryA(path->c_str());
	if (!result) {
		printf("[-] LoadLibraryA failed\n");
	} else {
		printf("[+] Loaded client\n");
	}
}

std::optional<std::string> Loader::prepareDll() {
	auto [data, size] = Files::loadResource(dllHandle, PACKET_V3_PE);
	if (!data) {
		return std::nullopt;
	}
	std::string dllPath = Files::getRoamingPath() + "\\packet-client-vector-powered2.dll";
	if (Files::exists(dllPath)) {
		return dllPath;
	}
	std::ofstream outFile(dllPath, std::ios::binary);
	outFile.write(reinterpret_cast<const char*>(data), size);
	return dllPath;
}
