#include "scanner.h"

#include "LightningScanner/LightningScanner.hpp"

SectionInfo Scanner::getSection(HANDLE handle, std::string const& targetSegmentName) {
	auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(handle);
	auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<char*>(dosHeader) + dosHeader->e_lfanew);
	auto firstSection = IMAGE_FIRST_SECTION(ntHeaders);
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
		auto section = firstSection[i];
		const char* segmentName = reinterpret_cast<const char*>(&section.Name[0]);
		if (targetSegmentName != segmentName) {
			continue;
		}
		char* startAddress = reinterpret_cast<char*>(handle) + section.VirtualAddress;
		size_t segmentSize = section.Misc.VirtualSize;
		return {
			startAddress,
			segmentSize};
	}
	return {
		nullptr,
		0};
}

void* Scanner::scanPattern(std::string const& pattern) {
	auto scanner = LightningScanner::Scanner<>(LightningScanner::Pattern(pattern));

	auto modHandle = GetModuleHandleA("Minecraft.Windows.exe");
	static SectionInfo info = getSection(modHandle, ".text");
	if (info.startAddress == nullptr) {
		return nullptr;
	}
	return scanner.Find(info.startAddress, info.size).Get<uint8_t>();
}
