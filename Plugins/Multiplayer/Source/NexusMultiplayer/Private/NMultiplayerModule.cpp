// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerModule.h"

#include "NMultiplayerUtils.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif // PLATFORM_WINDOWS


void FNMultiplayerModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusMultiplayer")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNMultiplayerModule, OnPostEngineInit);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNMultiplayerModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusMultiplayerSamples")
	
	if (FNMultiplayerUtils::IsMultiplayerTest())
	{
#if PLATFORM_WINDOWS
		const Windows::HWND WindowHandle = FWindowsPlatformMisc::GetTopLevelWindowHandle(FWindowsPlatformProcess::GetCurrentProcessId());
		FString FileName = FPaths::GetCleanFilename(FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
		FileName.ReplaceInline(TEXT(".log"), TEXT(""), ESearchCase::IgnoreCase);
		FString NewWindowTitle;
		
		FString ServerAddress;
		const TCHAR* CmdLine = FCommandLine::Get();
		
		// Check if connecting to something
		if (CmdLine && FCString::Strlen(CmdLine) > 0)
		{
			FString Token;
			FParse::Token(CmdLine, Token, false);
			if (Token.Contains(TEXT(".")) || Token.Contains(TEXT(":")))
			{
				ServerAddress = Token;
			}
		}
		if (!ServerAddress.IsEmpty())
		{
			NewWindowTitle = FString::Printf(TEXT("Client (%s) %s"), *ServerAddress, *FileName);
		}
		else
		{
			if (IsRunningDedicatedServer())
			{
				NewWindowTitle = FString::Printf(TEXT("DedicatedServer %s"), *FileName);
			}
			else if (IsRunningClientOnly())
			{
				NewWindowTitle = FString::Printf(TEXT("Client %s"), *FileName);
			}
			else
			{
				NewWindowTitle = FString::Printf(TEXT("ClientServer %s"), *FileName);
			}
		}
		
		SetWindowText(WindowHandle, NewWindowTitle.GetCharArray().GetData());
#else // !PLATFORM_WINDOWS
	
#endif // PLATFORM_WINDOWS
	}
}

IMPLEMENT_MODULE(FNMultiplayerModule, NexusMultiplayer)