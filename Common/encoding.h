// encoding.h

#pragma once

#include <string>

namespace Encoding
{
	enum CodePage
	{
		ACP = 0,
		UTF_8 = 65001,
		SHIFT_JIS = 932,
		GBK = 936,
	};

	std::wstring AnsiToUnicode(const std::string& source, int codePage);
	std::string UnicodeToAnsi(const std::wstring& source, int codePage);
	std::wstring Utf8ToUtf16(const std::string& source);
	std::string Utf16ToUtf8(const std::wstring& source);
}
