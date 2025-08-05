// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "InstanceObjects/NUpdateCheckInstanceObject.h"

#include "EditorUtilityLibrary.h"
#include "HttpModule.h"
#include "NCoreEditorMinimal.h"
#include "NEditorSettings.h"
#include "NEditorUserSettings.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UNUpdateCheckInstanceObject::Create()
{
	const UNEditorSettings* Settings = UNEditorSettings::Get();

	//  Check if settings enabled, don't bother making the object if it's not.
	if (!Settings->bUpdatesCheck) return;
	
	// Let's check the last time we actually looked so that were not hammering
	const UNEditorUserSettings* UserSettings = UNEditorUserSettings::Get();
	const FTimespan TimeDifference = FDateTime::Now() - UserSettings->UpdatesLastChecked;
	// if (TimeDifference.GetDays() <= Settings->UpdatesFrequency)
	// {
	// 	return;
	// }

	NE_LOG(Warning, TEXT("TIME diff %i HIT"), TimeDifference.GetDays())

	// Ok fine, lets actually do the update, but let's wait a bit
	UAsyncEditorDelay* DelayedCheck = NewObject<UAsyncEditorDelay>();

	UNUpdateCheckInstanceObject* UpdateObject = NewObject<UNUpdateCheckInstanceObject>(DelayedCheck);
	UpdateObject->AddToRoot();
	
	DelayedCheck->Complete.AddDynamic(UpdateObject, &UNUpdateCheckInstanceObject::Execute);
	DelayedCheck->Start(5.f, 25);
}

void UNUpdateCheckInstanceObject::Execute()
{
	RemoveFromRoot();
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(GetURI());
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetTimeout(5.0f);
    
	HttpRequest->OnProcessRequestComplete().BindLambda([]([[maybe_unused]] FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			TArray<FString> Lines;
			Response->GetContentAsString().ParseIntoArrayLines(Lines, true);

			// We go reverse because it is near the end
			for (int i = Lines.Num() - 1; i >= 0; i--)
			{
				if (Lines[i].StartsWith(TEXT("#define N_VERSION_NUMBER")))
				{
					const FString VersionNumber = Lines[i].Replace(TEXT("#define N_VERSION_NUMBER"), TEXT("")).TrimStartAndEnd();
					NE_LOG(Log, TEXT("[UNUpdateCheckInstanceObject::Execute] Found remote version: %s"), *VersionNumber);

					if (VersionNumber.IsNumeric())
					{
						const int32 VersionNumberActual = FCString::Atoi(*VersionNumber);
						const UNEditorSettings* Settings = UNEditorSettings::Get();
					
						if (VersionNumberActual > Settings->UpdatesIgnoreVersion)
						{
							// TODO: NOTICE OF UPDATE
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
}

FString UNUpdateCheckInstanceObject::GetURI() const
{
	switch (const UNEditorSettings* Settings = UNEditorSettings::Get();
		Settings->UpdatesChannel)
	{
		case NUC_GithubDev:
		return DevURI;
		case NUC_Custom:
		return Settings->UpdatesCustomURI;
		default:
		return MainURI;
	}
}
