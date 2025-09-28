#include "files.h"

std::pair<void*, size_t> Files::loadResource(HMODULE handle, int resourceId) {
	const HRSRC resourceInfo = FindResourceA(handle, MAKEINTRESOURCEA(resourceId), "WAVE");
	if (!resourceInfo) {
		return {nullptr, 0};
	}
	const HGLOBAL resourceData = LoadResource(handle, resourceInfo);
	if (!resourceData) {
		return {nullptr, 0};
	}
	const LPVOID resourcePtr = LockResource(resourceData);
	const DWORD resourceSize = SizeofResource(handle, resourceInfo);
	return {static_cast<char*>(resourcePtr), resourceSize};
}

std::string Files::substring(const std::string& str, const int start, const int end) {
	return str.substr(start, end - start);
}

std::string Files::getEnv(std::string const& variableName) {
	size_t requiredSize;
	getenv_s(&requiredSize, NULL, 0, "localappdata");
	if (requiredSize == 0) {
		return "";
	}
	char* buffer = static_cast<char*>(malloc(requiredSize));
	getenv_s(&requiredSize, buffer, requiredSize, "localappdata");
	std::string envResult(buffer);
	free(buffer);
	return envResult;
}

std::string Files::getRoamingPath() {
	std::string homeDir = getEnv("home");
	std::string removalString = "\\AC";
	return substring(homeDir, 0, homeDir.length() - removalString.length()) + "\\RoamingState";
}

bool Files::exists(std::string const& path) {
	struct stat info;
	if (stat(path.c_str(), &info) != 0) {
		return false;  // couldn't access it
	}

	return info.st_mode & S_IFDIR || info.st_mode & S_IFREG;
}
