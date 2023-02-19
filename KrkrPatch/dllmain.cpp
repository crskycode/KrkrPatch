// dllmain.cpp


#include "../Common/encoding.h"
#include "../Common/path.h"
#include "../Common/pe.h"
#include "../Common/util.h"
#include "../KrkrPlugin/tp_stub.h"
#include <nlohmann/json.hpp>
#include <detours/detours.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fstream>


static int g_logLevel;


static HMODULE g_hEXE;
static HMODULE g_hDLL;


static std::list<std::wstring> g_patchProtocols;
static std::list<std::wstring> g_patchArchives;
static bool g_patchNoProtocol;


template<class T>
void InlineHook(T& OriginalFunction, T DetourFunction)
{
	DetourUpdateThread(GetCurrentThread());
	DetourTransactionBegin();
	DetourAttach(&(PVOID&)OriginalFunction, (PVOID&)DetourFunction);
	DetourTransactionCommit();
}


template<class T>
void UnInlineHook(T& OriginalFunction, T DetourFunction)
{
	DetourUpdateThread(GetCurrentThread());
	DetourTransactionBegin();
	DetourDetach(&(PVOID&)OriginalFunction, (PVOID&)DetourFunction);
	DetourTransactionCommit();
}


typedef tTJSBinaryStream* (_fastcall* tTVPCreateStreamProc)(const ttstr&, tjs_uint);


// Original
static tTVPCreateStreamProc pfnTVPCreateStream;
// Hook
tTJSBinaryStream* _fastcall HookTVPCreateStream(const ttstr& name, tjs_uint flags)
{
	if (flags == TJS_BS_READ)
	{
		auto inarcname = name.c_str();

		bool accepted = false;

		if (wcsstr(inarcname, L"://") != NULL)
		{
			for (auto& protocol : g_patchProtocols)
			{
				if (_wcsnicmp(inarcname, protocol.c_str(), protocol.length()) == 0)
				{
					inarcname += protocol.length();
					accepted = true;
					break;
				}
			}
		}
		else if (g_patchNoProtocol)
		{
			accepted = true;
		}

		if (accepted)
		{
			if (wcsncmp(inarcname, L"./", 2) == 0)
			{
				inarcname += 2;
			}

			for (auto& arc : g_patchArchives)
			{
				auto patchname = TVPGetAppPath() + arc.c_str() + L">" + inarcname;

				// spdlog::debug("Find {}", Encoding::Utf16ToUtf8(patchname.c_str()));

				if (TVPIsExistentStorageNoSearchNoNormalize(patchname))
				{
					// spdlog::debug("Open {}", Encoding::Utf16ToUtf8(patchname.c_str()));

					return pfnTVPCreateStream(patchname, flags);
				}
			}
		}
	}

	return pfnTVPCreateStream(name, flags);
}


static const char CreateStreamSig[] = "\x55\x8B\xEC\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x2A\x2A\x2A\x2A\x50\x83\xEC\x5C\x53\x56\x57\xA1\x2A\x2A\x2A\x2A\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x2A\x2A\x2A\x2A\x89\x65\xF0\x89\x4D\xEC\xC7\x45\x2A\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x8B\x4D\xF4\x64\x89\x0D\x2A\x2A\x2A\x2A\x59\x5F\x5E\x5B\x8B\xE5\x5D\xC3";


void InstallStreamHook()
{
	auto base = PE::GetModuleBase(g_hEXE);
	auto size = PE::GetModuleSize(g_hEXE);

	pfnTVPCreateStream = (tTVPCreateStreamProc)PE::SearchPattern(base, size, CreateStreamSig, sizeof(CreateStreamSig) - 1);

	if (pfnTVPCreateStream)
	{
		InlineHook(pfnTVPCreateStream, HookTVPCreateStream);

		spdlog::debug("Hook installed.");
	}
	else
	{
		spdlog::error("Couldn't to find signature on target executable.");
	}
}


extern "C"
{
	typedef HRESULT(_stdcall* tTVPV2LinkProc)(iTVPFunctionExporter*);
	typedef HRESULT(_stdcall* tTVPV2UnlinkProc)();
}


// Original
static tTVPV2LinkProc pfnTVPV2Link;
// Hook
HRESULT _stdcall HookV2Link(iTVPFunctionExporter* exporter)
{
	pfnTVPV2Link(exporter);
	UnInlineHook(pfnTVPV2Link, HookV2Link);
	TVPInitImportStub(exporter);
	InstallStreamHook();
	return 1;
}


// Original
auto pfnGetProcAddress = GetProcAddress;
// Hook
FARPROC WINAPI HookGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	auto result = pfnGetProcAddress(hModule, lpProcName);

	if (HIWORD(lpProcName))
	{
		if (strcmp(lpProcName, "V2Link") == 0)
		{
			pfnTVPV2Link = (tTVPV2LinkProc)result;

			UnInlineHook(pfnGetProcAddress, HookGetProcAddress);
			InlineHook(pfnTVPV2Link, HookV2Link);
		}
	}

	return result;
}


void InstallHook()
{
	InlineHook(pfnGetProcAddress, HookGetProcAddress);
}


void LoadConfig()
{
	try
	{
		auto dirPath = Util::GetAppDirectoryW();
		auto cfgPath = dirPath + L"\\KrkrPatch.json";

		std::ifstream cfgFile(cfgPath);

		if (!cfgFile)
		{
			Util::ThrowError(L"Couldn't to open file: %s", cfgPath.c_str());
			return;
		}

		nlohmann::json cfg = nlohmann::json::parse(cfgFile);

		// # logger

		g_logLevel = cfg["logLevel"].get<int>();

		switch (g_logLevel)
		{
			case 0:
				spdlog::set_level(spdlog::level::off);
				break;
			case 1:
				spdlog::set_level(spdlog::level::err);
				break;
			case 2:
				spdlog::set_level(spdlog::level::warn);
				break;
			case 3:
				spdlog::set_level(spdlog::level::info);
				break;
			case 4:
				spdlog::set_level(spdlog::level::debug);
				break;
		}

		// # protocols

		for (auto& proto : cfg["patchProtocols"])
		{
			g_patchProtocols.emplace_back(Encoding::Utf8ToUtf16(proto.get<std::string>()));
			spdlog::info("Patch protocol: {}", proto.get<std::string>());
		}

		// # patches

		for (auto& arc : cfg["patchArchives"])
		{
			g_patchArchives.emplace_back(Encoding::Utf8ToUtf16(arc.get<std::string>()));
			spdlog::info("Patch archive: {}", arc.get<std::string>());
		}

		// # patch no protocol file

		g_patchNoProtocol = cfg["patchNoProtocol"].get<bool>();
	}
	catch (const std::exception& e)
	{
		Util::ThrowError("Couldn't to load config file: %s", e.what());
		return;
	}
}


void OnStartup()
{
	LoadConfig();

	if (g_logLevel > 0)
	{
		spdlog::set_default_logger(spdlog::basic_logger_mt("KrkrPatch", "KrkrPatch.log"));
		spdlog::info("Startup");
	}

	InstallHook();
}


void OnShutdown()
{
	if (g_logLevel > 0)
	{
		spdlog::info("Shutdown");
		spdlog::default_logger()->flush();
	}
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hEXE = GetModuleHandle(NULL);
			g_hDLL = hModule;
			OnStartup();
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			OnShutdown();
			break;
		}
	}

	return TRUE;
}


extern "C" __declspec(dllexport) BOOL CreatePlugin()
{
	return TRUE;
}
