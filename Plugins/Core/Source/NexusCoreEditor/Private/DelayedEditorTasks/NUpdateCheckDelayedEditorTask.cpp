// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"

#include "EditorUtilityLibrary.h"
#include "HttpModule.h"
#include "NCoreEditorMinimal.h"
#include "NEditorSettings.h"
#include "NEditorUserSettings.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UNUpdateCheckDelayedEditorTask::Create()
{
	const UNEditorSettings* Settings = UNEditorSettings::Get();

	//  Check if settings enabled, don't bother making the object if it's not.
	if (!Settings->bUpdatesCheck) return;
	
	// Let's check the last time we actually looked so that were not hammering
	const UNEditorUserSettings* UserSettings = UNEditorUserSettings::Get();
	const FTimespan TimeDifference = FDateTime::Now() - UserSettings->UpdatesLastChecked;
	if (TimeDifference.GetDays() <= Settings->UpdatesFrequency)
	{
		return;
	}

	// Ok, fine, let's actually do the update, but let's wait a bit
	UAsyncEditorDelay* DelayedMechanism = CreateDelayMechanism();
	UNUpdateCheckDelayedEditorTask* UpdateObject = NewObject<UNUpdateCheckDelayedEditorTask>(DelayedMechanism);
	UpdateObject->Lock(DelayedMechanism);
	
	DelayedMechanism->Complete.AddDynamic(UpdateObject, &UNUpdateCheckDelayedEditorTask::Execute);
	DelayedMechanism->Start(5.f, 25);
}

void UNUpdateCheckDelayedEditorTask::Execute()
{
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(GetQueryURI());
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetTimeout(5.0f);
	HttpRequest->AppendToHeader(TEXT("UpdateURI"), GetUpdateURI());
    
	HttpRequest->OnProcessRequestComplete().BindLambda([](const FHttpRequestPtr& Request, const FHttpResponsePtr& Response, const bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			TArray<FString> Lines;
			Response->GetContentAsString().ParseIntoArrayLines(Lines, true);
			for (int32 i = Lines.Num() - 1; i >= 0; i--)
			{
				if (Lines[i].StartsWith(TEXT("#define N_VERSION_NUMBER")))
				{
					const FString VersionNumber = Lines[i].Replace(TEXT("#define N_VERSION_NUMBER"), TEXT("")).TrimStartAndEnd();
					UE_LOG(LogNexusCoreEditor, Verbose, TEXT("Found remote plugin version(%s)."), *VersionNumber);

					if (VersionNumber.IsNumeric())
					{
						const int32 VersionNumberActual = FCString::Atoi(*VersionNumber);
						UNEditorSettings* Settings = UNEditorSettings::GetMutable();
						
						// Check for a previously saved ignored version, and bump it up to current
						if (Settings->UpdatesIgnoreVersion < N_VERSION_NUMBER)
						{
							Settings->UpdatesIgnoreVersion = N_VERSION_NUMBER;
							Settings->SaveConfig();
						}
					
						if (VersionNumberActual > Settings->UpdatesIgnoreVersion)
						{
							const FText DialogTitle = FText::FromString(TEXT("NEXUS: Update Detected"));
							const FText DialogMessage = FText::FromString(FString::Printf(
								TEXT("An update is available for the NEXUS Framework.\nCurrently you are on v%i.%i.%i (%i), there is a newer version number (%i) available.\n\nWould you like to get it?\n\nSelecting 'No' will ignore this version update."),
								N_VERSION_MAJOR, N_VERSION_MINOR, N_VERSION_PATCH, N_VERSION_NUMBER, VersionNumberActual));

							const EAppReturnType::Type DialogResponse = FMessageDialog::Open(EAppMsgCategory::Success,
								EAppMsgType::Type::YesNo,DialogMessage, DialogTitle);

							if (DialogResponse == EAppReturnType::Yes)
							{
								UE_LOG(LogNexusCoreEditor, Verbose, TEXT("Opening browser for new plugin version(%i)."), VersionNumberActual);
								FPlatformProcess::LaunchURL(*Request->GetHeader(TEXT("UpdateURI")),nullptr, nullptr);
							}
							else
							{
								UNEditorSettings* EditorSettings = UNEditorSettings::GetMutable();
								EditorSettings->UpdatesIgnoreVersion = VersionNumberActual;
								EditorSettings->SaveConfig();
								UE_LOG(LogNexusCoreEditor, Log, TEXT("Ignoring plugin version (%i)."), VersionNumberActual);
							}
						}
					}
					break;
				}
			}
		}
	});
	HttpRequest->ProcessRequest();
	
	// Update last checked
	UNEditorUserSettings* UserSettings = UNEditorUserSettings::GetMutable();
	UserSettings->UpdatesLastChecked = FDateTime::Now();
	UserSettings->SaveConfig();
	Release();
}

FString UNUpdateCheckDelayedEditorTask::GetQueryURI() const
{
	switch (const UNEditorSettings* Settings = UNEditorSettings::Get();
		Settings->UpdatesChannel)
	{
		case NUC_GithubDev:
		return DevQueryURI;
		case NUC_Custom:
		return Settings->UpdatesCustomQueryURI;
		default:
		return MainQueryURI;
	}
}

FString UNUpdateCheckDelayedEditorTask::GetUpdateURI() const
{
	switch (const UNEditorSettings* Settings = UNEditorSettings::Get();
		Settings->UpdatesChannel)
	{
		case NUC_GithubDev:
		return DevUpdateURI;
		case NUC_Custom:
		return Settings->UpdatesCustomUpdateURI;
		default:
		return MainUpdateURI;
	}
}
