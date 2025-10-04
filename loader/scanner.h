#pragma once

#define NOMINMAX
#include <Windows.h>

#include <string>

struct SectionInfo {
	void* startAddress;
	size_t size;
};

class Scanner {
public:
	static SectionInfo getSection(HANDLE handle, std::string const& targetSegmentName);

	static void* scanPattern(std::string const& pattern);
};
