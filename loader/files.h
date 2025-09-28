#pragma once

#include <Windows.h>

#include <string>

class Files {
public:
	static std::pair<void*, size_t> loadResource(HMODULE handle, int resourceId);

	static std::string substring(const std::string& str, const int start, const int end);

	static std::string getEnv(std::string const& variableName);

	static std::string getRoamingPath();

	static bool exists(std::string const& path);
};
