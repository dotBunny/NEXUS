// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "InstanceObjects/NUpdateCheckInstanceObject.h"

#include "EditorUtilityLibrary.h"
#include "NCoreEditorMinimal.h"
#include "NEditorSettings.h"
#include "NEditorUserSettings.h"

void UNUpdateCheckInstanceObject::Create()
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
	
	NE_LOG(Warning, TEXT("EXECUTE UPDATE"))

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
