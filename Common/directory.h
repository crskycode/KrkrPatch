// directory.h

#pragma once

#include <string>

namespace Directory
{
	bool Exists(const std::string& path);
	bool Exists(const std::wstring& path);
}
