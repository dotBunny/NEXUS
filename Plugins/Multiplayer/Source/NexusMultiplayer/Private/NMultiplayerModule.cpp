// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerModule.h"

#include "NMultiplayerUtils.h"
#include "HAL/PlatformOutputDevices.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modules/ModuleManager.h"

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
		MultiplayerTestWindowTitle = FText::FromString(WindowTitle);
		UKismetSystemLibrary::SetWindowTitle(MultiplayerTestWindowTitle);
	}
}

IMPLEMENT_MODULE(FNMultiplayerModule, NexusMultiplayer)
