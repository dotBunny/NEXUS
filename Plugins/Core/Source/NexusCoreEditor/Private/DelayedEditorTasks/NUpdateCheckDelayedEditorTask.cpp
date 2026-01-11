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
	
	if (TimeDifference.GetDays() < Settings->UpdatesFrequency)
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Next update check in %i days."), (Settings->UpdatesFrequency - TimeDifference.GetDays()));
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
	HttpRequest->OnProcessRequestComplete().BindStatic(OnUpdateQueryResponse);
	
	// Start processing the request, triggering our response handler.
	HttpRequest->ProcessRequest();
	
	// Update last checked
	UNEditorUserSettings* UserSettings = UNEditorUserSettings::GetMutable();
	UserSettings->UpdatesLastChecked = FDateTime::Now();
	UserSettings->SaveConfig();
	Release();
}

void UNUpdateCheckDelayedEditorTask::OnUpdateQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bProcessedSuccessfull)
{
	if (!bProcessedSuccessfull)
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("The update check web request was unable to be processed."))
		return;
	}
	if (!Response.IsValid())
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("The update check web request response was invalid."))
		return;
	}
	
	TArray<FString> Lines;
	FString TargetVersion = TEXT("");
	Response->GetContentAsString().ParseIntoArrayLines(Lines, true);
	
	for (int32 i = Lines.Num() - 1; i >= 0; i--)
	{
		if (Lines[i].TrimStart().StartsWith(TEXT("constexpr int Number")))
		{
			TargetVersion = Lines[i];
			break;
		}
	}

	// We didn't find anything, bail
	if (TargetVersion.IsEmpty())
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("Unable to find remote plugin version for update."))
		return;
	}
	
	// Grab a changable version of the settings
	UNEditorSettings* Settings = UNEditorSettings::GetMutable();
	
	// Check for a previously saved ignored version, and bump it up to current
	if (Settings->UpdatesIgnoreVersion < NEXUS::Version::Number)
	{
		
		Settings->UpdatesIgnoreVersion = NEXUS::Version::Number;
		Settings->SaveConfig();
	}
	
	// We need to clean up the line and look just for the number
	TargetVersion = TargetVersion.Replace(TEXT("constexpr int Number ="), TEXT(""));
	TargetVersion = TargetVersion.Replace(TEXT(";"), TEXT(""));
	TargetVersion = TargetVersion.TrimStartAndEnd();
	UE_LOG(LogNexusCoreEditor, Log, TEXT("Found remote plugin version(%s)."), *TargetVersion);

	if (!TargetVersion.IsNumeric())
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to parse remote plugin version(%s) as a number."), *TargetVersion);
		return;
	}

	const int32 VersionNumberActual = FCString::Atoi(*TargetVersion);
	
	// Notify about upgrade
	if (VersionNumberActual > Settings->UpdatesIgnoreVersion)
	{
		const FText DialogTitle = FText::FromString(TEXT("NEXUS: Update Detected"));
		const FText DialogMessage = FText::FromString(FString::Printf(
			TEXT("An update is available for the NEXUS Framework.\nCurrently you are on v%i.%i.%i (%i), there is a newer version number (%i) available.\n\nWould you like to get it?\n\nSelecting 'No' will ignore this version update."),
			NEXUS::Version::Major, NEXUS::Version::Minor, NEXUS::Version::Patch, NEXUS::Version::Number, VersionNumberActual));

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

FString UNUpdateCheckDelayedEditorTask::GetQueryURI() const
{
	switch (const UNEditorSettings* Settings = UNEditorSettings::Get();
		Settings->UpdatesChannel)
	{
		case NUC_GithubMain:
		return MainQueryURI;
		case NUC_Custom:
		return Settings->UpdatesCustomQueryURI;
		default:
		return ReleaseQueryURI;
	}
}

FString UNUpdateCheckDelayedEditorTask::GetUpdateURI() const
{
	switch (const UNEditorSettings* Settings = UNEditorSettings::Get();
		Settings->UpdatesChannel)
	{
		case NUC_GithubMain:
		return MainUpdateURI;
		case NUC_Custom:
		return Settings->UpdatesCustomUpdateURI;
		default:
		return ReleaseUpdateURI;
	}
}
