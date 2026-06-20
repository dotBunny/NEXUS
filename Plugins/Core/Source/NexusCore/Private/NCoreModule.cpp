// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreModule.h"
#include "NCoreMinimal.h"
#include "Developer/NPrimitiveFont.h"
#include "Interfaces/IPluginManager.h"
#if WITH_EDITOR
#include "NMultiplayerUtils.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "Kismet/KismetSystemLibrary.h"
#endif // WITH_EDITOR
#include "Modules/ModuleManager.h"

void FNCoreModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusCore")

	UE_LOG(LogNexusCore, Log, TEXT("NEXUS Version %i (%i.%i.%i)"), NEXUS::Version::Number, NEXUS::Version::Major, NEXUS::Version::Minor, NEXUS::Version::Patch);

	// Initialize our font glyphs
	FNPrimitiveFont::Initialize();

#if WITH_EDITOR
	N_MODULE_POST_ENGINE_INIT(FNCoreModule, OnPostEngineInit);
#endif // WITH_EDITOR
}

void FNCoreModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT()
	IModuleInterface::ShutdownModule();
}

IMPLEMENT_MODULE(FNCoreModule, NexusCore)

#if WITH_EDITOR
void FNCoreModule::OnPostEngineInit()
{
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

		UE_LOG(LogNexusCore, Log, TEXT("Multiplayer Test - %s"), *WindowTitle);
		MultiplayerTestWindowTitle = FText::FromString(WindowTitle);
		UKismetSystemLibrary::SetWindowTitle(MultiplayerTestWindowTitle);
	}
}
#endif // WITH_EDITOR