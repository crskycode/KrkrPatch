// encoding.cpp

#include <windows.h>
#include "encoding.h"

#undef max

namespace Encoding
{
	std::wstring AnsiToUnicode(const std::string& source, int codePage)
	{
		if (source.length() == 0)
		{
			return std::wstring();
		}

		if (source.length() > (size_t)std::numeric_limits<int>::max())
		{
			return std::wstring();
		}

		int length = MultiByteToWideChar(codePage, 0, source.c_str(), (int)source.length(), NULL, 0);

		if (length <= 0)
		{
			return std::wstring();
		}

		std::wstring output(length, L'\0');

		if (MultiByteToWideChar(codePage, 0, source.c_str(), (int)source.length(), (LPWSTR)output.data(), (int)output.length() + 1) == 0)
		{
			return std::wstring();
		}

		return output;
	}

	std::string UnicodeToAnsi(const std::wstring& source, int codePage)
	{
		if (source.length() == 0)
		{
			return std::string();
		}

		if (source.length() > (size_t)std::numeric_limits<int>::max())
		{
			return std::string();
		}

		int length = WideCharToMultiByte(codePage, 0, source.c_str(), (int)source.length(), NULL, 0, NULL, NULL);

		if (length <= 0)
		{
			return std::string();
		}

		std::string output(length, '\0');

		if (WideCharToMultiByte(codePage, 0, source.c_str(), (int)source.length(), (LPSTR)output.data(), (int)output.length() + 1, NULL, NULL) == 0)
		{
			return std::string();
		}

		return output;
	}

	std::wstring Utf8ToUtf16(const std::string& source)
	{
		return AnsiToUnicode(source, UTF_8);
	}

	std::string Utf16ToUtf8(const std::wstring& source)
	{
		return UnicodeToAnsi(source, UTF_8);
	}
}

