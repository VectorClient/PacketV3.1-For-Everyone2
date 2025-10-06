#pragma once

#include <Windows.h>
#include <minwindef.h>
#include <winnt.h>

#include <string>

namespace Injector {
void grant(std::wstring path);

void inject(HANDLE processHandle, std::wstring path);
}  // namespace Injector
