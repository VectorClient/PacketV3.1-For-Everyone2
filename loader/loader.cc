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
#include <unordered_map>

#include "MinHook.h"
#include "files.h"
#include "kiero/Kiero.h"
#include "resource/resource.h"
#include "scanner.h"

class ClientInstance;
ClientInstance* clientInstance;

constexpr uint32_t present_table_index = 8;
constexpr uint32_t resize_buffers_table_index = 13;

static bool patchedOriginal = false;
constexpr uint32_t present_hook_offset = 0x23CCC0;
constexpr uint32_t resize_buffers_hook_offset = 0x241410;

constexpr uint32_t minhook_initialize_func_offset = 0x24C010;
constexpr uint32_t present_original_callback_offset = 0x3CBB78;
constexpr uint32_t resize_buffers_original_callback_offset = 0x3CBB70;
constexpr uint32_t user_info_offset = 0x303250;
constexpr uint32_t client_instance_offset = 0x3C9778;
constexpr uint32_t func_map_offset = 0x2FA340;
constexpr uint32_t offset_map_offset = 0x2FA380;

const static std::unordered_map<uint32_t, uint64_t> func_map = {{0, 0x2909eb}, {1, 0x7eff25}, {2, 0x1fd0239}, {3, 0x1fddecd}, {4, 0x211d036}, {5, 0x1073930}, {6, 0x3e3f570}, {7, 0x1b2a040}, {8, 0x1b36540}, {9, 0x1c83870}, {10, 0x1c9f040}, {11, 0x1c9e3e0}, {12, 0x373040}, {13, 0x5e5d50}, {14, 0x535632}, {15, 0xa31dd0}, {16, 0x58b3210}, {17, 0x67fce0}, {18, 0x1b61030}, {19, 0x66f3410}, {20, 0x196f080}, {21, 0x50d8c0}, {22, 0x1d75950}, {23, 0xda9a40}, {24, 0xda8360}, {25, 0x1b53c00}, {26, 0xb5cf80}, {27, 0x50d4c0}, {28, 0x1b85180}, {29, 0x1c83230}, {30, 0x1d81e40}, {31, 0x3bf3480}, {32, 0x455e230}, {33, 0xb5d270}, {34, 0x1b8fbf0}, {35, 0x3d09f20}, {36, 0xdb1310}, {37, 0xb927e0}, {38, 0xb92180}, {39, 0xb923e0}, {40, 0x1b84930}, {41, 0x135d70}, {42, 0x5906c00}, {43, 0x3ebe030}, {44, 0x3ebf2c0}, {45, 0x3eb7c00}, {46, 0x404190}, {47, 0x403080}, {48, 0x41ba00}, {49, 0x423600}, {50, 0x416b30}, {51, 0x4235c0}, {52, 0x425a70}, {53, 0x3e88640}, {54, 0x3df5660}, {55, 0x3f16320}, {56, 0x364a010}, {57, 0x3a9c830}, {58, 0xf9ea50}, {59, 0x3a9c780}, {60, 0x3aa3aa0}, {61, 0xf97490}, {62, 0x3afa860}, {63, 0x3d128d0}, {64, 0x3a9aa20}, {65, 0x1f1ce70}, {66, 0x3a9aa80}, {67, 0x3d0b660}, {68, 0xf9ea90}, {69, 0xf9f1a0}, {70, 0x3e14de0}, {71, 0x3df23d0}, {72, 0x3d12810}, {73, 0xf97250}, {74, 0x3ee7e40}, {75, 0x2606b70}, {76, 0x3b121a0}, {77, 0x3905a0}, {78, 0xfd48c0}, {79, 0x45c8410}, {80, 0x45ce3c0}, {81, 0x4503f40}, {82, 0x4505050}, {83, 0x4504e30}, {84, 0x4505570}, {85, 0x45d3000}, {86, 0x45c67e0}, {87, 0x1b86090}, {88, 0x44fc950}, {89, 0x3ced250}, {90, 0x45673c0}, {91, 0x4716c20}, {92, 0x139a1c0}, {93, 0x48ad330}, {94, 0xd07c30}, {95, 0x40ef640}, {96, 0x4257ec0}, {97, 0x139c360}, {98, 0x139b900}, {99, 0x139cc30}, {100, 0x4259160}, {101, 0x139b5f0}, {102, 0x4018c90}, {103, 0x139e120}, {104, 0x4019400}, {105, 0x448ee50}, {106, 0x3de5cb0}, {107, 0x139f3c0}, {108, 0x24c2b30}, {109, 0x247ca30}, {110, 0x441d290}, {111, 0x54a10e0}, {112, 0x2481ba0}, {113, 0x25fbc30}, {114, 0x5494a20}};
const static std::unordered_map<uint32_t, int32_t> offset_map = {{0, 0x3f0}, {1, 0x388}, {2, 0x50}, {3, 0xe8}, {4, 0xd0}, {5, 0xf8}, {6, 0x740}, {7, 0x754}, {8, 0x670}, {9, 0xd8}, {10, 0x5b8}, {11, 0x8}, {12, 0x3a4}, {13, 0x1e8}, {14, 0x448}, {15, 0x220}, {16, 0x218}, {17, 0x404}, {18, 0x269}, {19, 0x1c8}, {20, 0xa88}, {21, 0x5c8}, {22, 0x228}, {23, 0x1d8}, {24, 0xa98}, {25, 0xc08}, {26, 0x8}, {27, 0xf0}, {28, 0x1b8}, {29, 0xe8}, {30, 0x0}, {31, 0xa4}, {32, 0xa8}, {33, 0xc0}, {34, 0x98}, {35, 0x1a0}, {36, 0x160}, {37, 0x1d6}, {38, 0x78}, {39, 0x4c}, {40, 0x44}, {41, 0x38}, {42, 0x38}, {43, 0x80}, {44, 0x18}, {45, 0xf0}, {46, 0x1c8}, {47, 0x8}, {48, 0x10}, {49, 0xc8}, {50, 0x30}, {51, 0x4b0}, {52, 0x3e0}, {53, 0xd8}, {54, 0x178}, {55, 0x150}, {56, 0x10}, {57, 0x22}, {58, 0x8}, {59, 0xb8}, {60, 0x10}, {61, 0x84}, {62, 0x8c}, {63, 0x8}, {64, 0x90}, {65, 0x7c}, {66, 0x78}, {67, 0x6c}, {68, 0x18}, {69, 0x10}, {70, 0x164}, {71, 0x18}};

HRESULT Loader::present_detour(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
	if (!clientHandle || !clientInstance) {
		return originalPresent(swapChain, syncInterval, flags);
	}
	PresentFunc func = reinterpret_cast<PresentFunc>(clientHandle + present_hook_offset);
	if (!patchedOriginal) {
		printf("[+] Caught the handle: handle=%llu, target=%p\n", clientHandle, func);	// hooks
		typedef void(__fastcall * MinhookInitializeFunc)();
		reinterpret_cast<MinhookInitializeFunc>(clientHandle + minhook_initialize_func_offset)();
		printf("[+] Initialized hook\n");
		// variables
		storeVariables(clientHandle);
		printf("[+] Stored globals\n");
		// remap
		remap(clientHandle);
		printf("[+] Remapped\n");
		ShowWindow(GetConsoleWindow(), SW_HIDE);

		patchedOriginal = true;
	}

	return func(swapChain, syncInterval, flags);
}

HRESULT Loader::resize_buffers_detour(IDXGISwapChain* swapChain, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
	if (!clientHandle) {
		return originalResizeBuffers(swapChain, width, height, newFormat, swapChainFlags);
	}
	ResizeBuffersFunc func = reinterpret_cast<ResizeBuffersFunc>(clientHandle + resize_buffers_hook_offset);
	return func(swapChain, width, height, newFormat, swapChainFlags);
}

void Loader::update_detour(ClientInstance* clientInstance, bool flag) {
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
}

void Loader::remap(uint64_t handle) {
	uint64_t mcHandle = reinterpret_cast<uint64_t>(GetModuleHandleA("Minecraft.Windows.exe"));

	auto* funcMapPtr = reinterpret_cast<std::unordered_map<uint32_t, uint64_t>*>(handle + func_map_offset);
	for (auto& pair : func_map) {
		funcMapPtr->insert({pair.first, pair.second + mcHandle});
	}

	auto* offsetMapPtr = reinterpret_cast<std::unordered_map<uint32_t, int32_t>*>(handle + offset_map_offset);
	for (auto& pair : offset_map) {
		offsetMapPtr->insert({pair.first, pair.second});
	}
}

void Loader::init(HMODULE dllHandle) {
	std::filesystem::create_directory(Files::getRoamingPath() + "\\Packet 3.1");
	Loader::dllHandle = dllHandle;
	loadDll();

	MH_Initialize();
	// DX11
	{
		auto kieroStatus = Kiero::initialize(Kiero::RenderType::D3D11);
		if (kieroStatus != Kiero::Status::Success) {
			printf("[-] Couldn't initialize kiero: code=%d\n", kieroStatus);
			return;
		}
		void* presentPtr = reinterpret_cast<void*>(Kiero::getMethodsTable()[present_table_index]);
		MH_CreateHook(presentPtr, reinterpret_cast<void*>(&present_detour), reinterpret_cast<void**>(&originalPresent));
		MH_EnableHook(presentPtr);

		void* resizePtr = reinterpret_cast<void*>(Kiero::getMethodsTable()[resize_buffers_table_index]);
		MH_CreateHook(resizePtr, reinterpret_cast<void*>(&resize_buffers_detour), reinterpret_cast<void**>(&originalResizeBuffers));
		MH_EnableHook(resizePtr);

		printf("[!] Created present hook\n");
	}
	// CI
	{
		void* ci_update = Scanner::scanPattern(
			"48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 41 56 41 57 48 8D AC 24 F0 FA FF FF 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 ?? ?? ?? ?? 44");	// long!
		if (!ci_update) {
			printf("[-] Couldnt find ClientInstance::update\n");
			return;
		}
		MH_CreateHook(ci_update, reinterpret_cast<void*>(&update_detour), reinterpret_cast<void**>(&originalUpdate));
		MH_EnableHook(ci_update);
		printf("[!] Created CI hook\n");
	}
}

void Loader::loadDll() {
	std::optional<std::string> path = prepareDll();
	if (!path) {
		printf("[-] Failed to load client data\n");
		return;
	}
	clientHandle = reinterpret_cast<uint64_t>(LoadLibraryA(path->c_str()));
	if (!clientHandle) {
		printf("[-] LoadedLibraryA failed\n");
	} else {
		printf("[+] Loaded client\n");
	}
}

std::optional<std::string> Loader::prepareDll() {
	auto [data, size] = Files::loadResource(dllHandle, PACKET_V3_PE);
	if (!data) {
		return std::nullopt;
	}
	std::string dllPath = Files::getRoamingPath() + "\\packet-client-vector-powered.dll";
	if (Files::exists(dllPath)) {
		return dllPath;
	}
	std::ofstream outFile(dllPath, std::ios::binary);
	outFile.write(reinterpret_cast<const char*>(data), size);
	return dllPath;
}
