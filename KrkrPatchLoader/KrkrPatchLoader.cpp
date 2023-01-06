// KrkrPatchLoader.cpp


#include "../Common/encoding.h"
#include "../Common/util.h"
#include "../Common/path.h"
#include <nlohmann/json.hpp>
#include <detours/detours.h>
#include <fstream>


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	auto dirPath = Util::GetAppDirectoryW();
	auto exePath = Util::GetAppPathW();

	// # Load configuration

	std::wstring gameExecutablePath;
	std::wstring gameCommandLine;

	try
	{
		auto cfgPath = dirPath + L"\\KrkrPatch.json";

		std::ifstream cfgFile(cfgPath);

		if (!cfgFile)
		{
			Util::ThrowError(L"Couldn't to open file: %s", cfgPath.c_str());
			return 1;
		}

		nlohmann::json cfg = nlohmann::json::parse(cfgFile);

		auto gameExecutableFile_ = Encoding::Utf8ToUtf16(cfg["gameExecutableFile"].get<std::string>());
		auto gameCommandLine_ = Encoding::Utf8ToUtf16(cfg["gameCommandLine"].get<std::string>());

		if (gameExecutableFile_.empty())
		{
			Util::ThrowError(L"Game executable file must be set.");
			return 1;
		}

		gameExecutablePath = dirPath + L"\\" + gameExecutableFile_;
		gameCommandLine = gameCommandLine_;
	}
	catch (const std::exception& e)
	{
		Util::ThrowError("Couldn't to load config file: %s", e.what());
		return 1;
	}

	// # Launch game

	STARTUPINFOW startupInfo{};
	PROCESS_INFORMATION processInfo{};

	startupInfo.cb = sizeof(startupInfo);

	LPCSTR dll = "KrkrPatch.dll";

	if (DetourCreateProcessWithDllsW(gameExecutablePath.c_str(), (LPWSTR)gameCommandLine.c_str(),
		NULL, NULL, FALSE, 0, NULL, dirPath.c_str(), &startupInfo, &processInfo, 1, &dll, NULL) == FALSE)
	{
		auto error = Util::GetLastErrorMessageW();
		Util::ThrowError(L"Couldn't to launch game: %s", error.c_str());
		return 1;
	}

	return 0;
}
