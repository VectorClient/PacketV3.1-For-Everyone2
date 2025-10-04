#include "remapper.hpp"

#include <Windows.h>

#include <unordered_map>

const static std::unordered_map<uint32_t, uint64_t> embeddedFuncMap = {{0, 0x93c2ab}, {1, 0xb41b49}, {2, 0x2649259}, {3, 0x26572ed}, {4, 0x279a381}, {5, 0x1a4cb30}, {6, 0x5180490}, {7, 0x232fef0}, {8, 0x1e32dc0}, {9, 0x2325000}, {10, 0x23246c0}, {11, 0xc5e060}, {12, 0xca7ab0}, {13, 0xd1b6c1}, {14, 0xe4acb0}, {15, 0x68856c0}, {16, 0xa55110}, {17, 0x1e71170}, {18, 0x18d990}, {19, 0xd92860}, {20, 0x24491c0}, {21, 0x13fcd00}, {22, 0x13fb6b0}, {23, 0x1e63c30}, {24, 0x1947590}, {25, 0xd92460}, {26, 0x1e94780}, {27, 0x1e32790}, {28, 0x2454940}, {29, 0x4eafb00}, {30, 0x58270d0}, {31, 0x1947870}, {32, 0x1e9f240}, {33, 0x1229ca0}, {34, 0x1132570}, {35, 0x1131ed0}, {36, 0x1132210}, {37, 0x1e93f40}, {38, 0x238ce30}, {39, 0x68bd5b0}, {40, 0x5361270}, {41, 0x5362b30}, {42, 0x535b0b0}, {43, 0xad9930}, {44, 0xad8e50}, {45, 0xaee750}, {46, 0xaf47c0}, {47, 0xaea930}, {48, 0xaf4780}, {49, 0xaf6330}, {50, 0x507d230}, {51, 0x501e590}, {52, 0x506b500}, {53, 0x4709f50}, {54, 0x4ce29a0}, {55, 0x144bbe0}, {56, 0x4ce28e0}, {57, 0x4ce9950}, {58, 0x1445340}, {59, 0x4e0c680}, {60, 0x4ce0ec0}, {61, 0x262b820}, {62, 0x4ce0f20}, {63, 0x4e05a70}, {64, 0x144c220}, {65, 0x503dce0}, {66, 0x501b510}, {67, 0x4e0c5e0}, {68, 0x14450f0}, {69, 0x57ca970}, {70, 0x575d0d0}, {71, 0x575e050}, {72, 0x575de80}, {73, 0x575e420}, {74, 0x1e955a0}, {75, 0x5756230}, {76, 0x4e440a0}, {77, 0x582f610}, {78, 0x59511d0}, {79, 0x15a3d50}, {80, 0x59279b0}, {81, 0x1834b80}, {82, 0x53a8200}, {83, 0x553f240}, {84, 0x15a5db0}, {85, 0x15a53b0}, {86, 0x15a66a0}, {87, 0x5540650}, {88, 0x15a50a0}, {89, 0x52198b0}, {90, 0x15a7cc0}, {91, 0x5219fa0}, {92, 0x544a7f0}, {93, 0x2aad946}, {94, 0x15c4a30}, {95, 0x2bf87e0}, {96, 0x2c57dd0}, {97, 0x522e990}, {98, 0x68e3c10}, {99, 0x2c5c290}, {100, 0x2f4b7c0}, {101, 0x68d7380}};
const static std::unordered_map<uint32_t, uint32_t> embeddedOffsetMap = {{0, 0x3e8}, {1, 0x348}, {2, 0x50}, {3, 0xe8}, {4, 0xd0}, {5, 0xf8}, {6, 0x740}, {7, 0x754}, {8, 0x6a0}, {9, 0xd8}, {10, 0x578}, {11, 0x8}, {12, 0x3a4}, {13, 0x1e8}, {14, 0x450}, {15, 0x220}, {16, 0x218}, {17, 0x40c}, {18, 0x269}, {19, 0x1c8}, {20, 0xa78}, {21, 0x5b8}, {22, 0x228}, {23, 0x1d8}, {24, 0xa88}, {25, 0xc18}, {26, 0x8}, {27, 0xf8}, {28, 0x1d8}, {29, 0xf0}, {30, 0x0}, {31, 0xa4}, {32, 0xa8}, {33, 0xc0}, {34, 0x98}, {35, 0x198}, {36, 0x158}, {37, 0x1c2}, {38, 0x78}, {39, 0x4c}, {40, 0x44}, {41, 0x44}, {42, 0x38}, {43, 0x80}, {44, 0x18}, {45, 0xf0}, {46, 0x1c8}, {47, 0x8}, {48, 0x10}, {49, 0xb8}, {50, 0x30}, {51, 0x4b0}, {52, 0x3e0}, {53, 0xd8}, {54, 0x178}, {55, 0x150}, {56, 0x10}, {57, 0x22}, {58, 0x8}, {59, 0xb8}, {60, 0x10}, {61, 0x84}, {62, 0x8c}, {63, 0x8}, {64, 0x90}, {65, 0x7c}, {66, 0x78}, {67, 0x6c}, {68, 0x18}, {69, 0x10}, {70, 0x164}, {71, 0x18}};

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

void Remapper::remapFunction(UnorderedMap map) {
	uint64_t minecraftHandle = reinterpret_cast<uint64_t>(GetModuleHandleA("Minecraft.Windows.exe"));
	auto* funcMap = reinterpret_cast<std::unordered_map<uint32_t, FunctionEntry*>*>(map);
	for (auto& item : embeddedFuncMap) {
		FunctionEntry* entry = static_cast<FunctionEntry*>(malloc(sizeof(FunctionEntry)));
		entry->value = item.second + minecraftHandle;
		funcMap->insert({item.first, entry});
	}
}

void Remapper::remapOffset(UnorderedMap map) {
	auto* offsetMap = reinterpret_cast<std::unordered_map<uint32_t, OffsetEntry*>*>(map);
	for (auto& item : embeddedOffsetMap) {
		OffsetEntry* entry = static_cast<OffsetEntry*>(malloc(sizeof(OffsetEntry)));
		entry->value = item.second;
		offsetMap->insert({item.first, entry});
	}
}
