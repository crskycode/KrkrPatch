// directory.cpp

#include <windows.h>
#include <string>

namespace Directory
{
	bool Exists(const std::string& path)
	{
		DWORD attrs = ::GetFileAttributesA(path.c_str());

		if (attrs == INVALID_FILE_ATTRIBUTES)
		{
			return false;
		}

		return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	bool Exists(const std::wstring& path)
	{
		DWORD attrs = ::GetFileAttributesW(path.c_str());

		if (attrs == INVALID_FILE_ATTRIBUTES)
		{
			return false;
		}

		return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
}
