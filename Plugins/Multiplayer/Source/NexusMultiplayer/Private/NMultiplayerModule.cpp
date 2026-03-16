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
	
	// After everything has been initialized, check for MP test and update.
	FCoreDelegates::OnFEngineLoopInitComplete.AddStatic(&FNMultiplayerModule::UpdateForMultiplayerTest);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNMultiplayerModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusMultiplayerSamples")
}

void FNMultiplayerModule::UpdateForMultiplayerTest()
{
	if (FNMultiplayerUtils::IsMultiplayerTest())
	{
#if PLATFORM_WINDOWS
		const Windows::HWND WindowHandle = FWindowsPlatformMisc::GetTopLevelWindowHandle(FWindowsPlatformProcess::GetCurrentProcessId());
		const FString FileName = FPaths::GetCleanFilename(FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
		
		FString ConnectionAddress;
		const TCHAR* CommandLine = FCommandLine::Get();
		if (CommandLine && FCString::Strlen(CommandLine) > 0)
		{
			FString Token;
			FParse::Token(CommandLine, Token, false);
			
			// Is the first token an address (the proper format) to connect to?
			if (Token.Contains(TEXT(".")) || Token.Contains(TEXT(":")))
			{
				ConnectionAddress = Token;
			}
		}
		
		FString WindowTitle;
		if (!ConnectionAddress.IsEmpty())
		{
			WindowTitle = FString::Printf(TEXT("%s Client (%s) %s"), FApp::GetProjectName(), *ConnectionAddress, *FileName);
		}
		else
		{
			if (IsRunningDedicatedServer())
			{
				WindowTitle = FString::Printf(TEXT("%s DedicatedServer %s"), FApp::GetProjectName(), *FileName);
			}
			else if (IsRunningClientOnly())
			{
				WindowTitle = FString::Printf(TEXT("%s Client %s"), FApp::GetProjectName(), *FileName);
			}
			else
			{
				WindowTitle = FString::Printf(TEXT("%s ClientServer %s"), FApp::GetProjectName(), *FileName);
			}
		}
		UE_LOG(LogNexusMultiplayer, Log, TEXT("Multiplayer Test - %s"), *WindowTitle);
		SetWindowText(WindowHandle, WindowTitle.GetCharArray().GetData());
#else // !PLATFORM_WINDOWS
	
#endif // PLATFORM_WINDOWS
	}
}

IMPLEMENT_MODULE(FNMultiplayerModule, NexusMultiplayer)
