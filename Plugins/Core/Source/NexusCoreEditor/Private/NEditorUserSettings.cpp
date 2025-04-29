// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUserSettings.h"
#include "NCoreEditorMinimal.h"
#include "NCoreEditorModule.h"
#include "NEditorInputProcessor.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GameFramework/GameUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NEditorUserSettings)

void UNEditorUserSettings::OnPostEngineInit()
{
	UNEditorUserSettings* Settings = GetMutable();
	Settings->ApplyEditorFrameRateLimit();
	Settings->ApplyAlwaysShowFrameRateAndMemory();
	Settings->ApplySpaceToPan();
}

void UNEditorUserSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Check if we are updating the frame rate
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNEditorUserSettings, EditorFrameRateLimit))
	{
		ApplyEditorFrameRateLimit();
	}

	ApplySpaceToPan();
}

void UNEditorUserSettings::ApplyEditorFrameRateLimit()
{
	if (FNEditorUtils::IsUserControlled())
	{
		NE_LOG(Log, TEXT("[UNEditorUserSettings::ApplyEditorFrameRateLimit] Attempting to apply framerate limit of: %f"), EditorFrameRateLimit);

		GEngine->SetMaxFPS(EditorFrameRateLimit);
		GEngine->GameUserSettings->SetFrameRateLimit(EditorFrameRateLimit);

		// Check results as it doesn't always stick.
		EditorFrameRateLimit = GEngine->GetMaxFPS();
		NE_LOG(Log, TEXT("[UNEditorUserSettings::ApplyEditorFrameRateLimit] *NEW* GEngine::MaxFPS: %f AND GameUserSettings::FrameRateLimit: %f"),
			EditorFrameRateLimit, GEngine->GameUserSettings->GetFrameRateLimit());
	}
}

void UNEditorUserSettings::ApplyAlwaysShowFrameRateAndMemory() const
{
	if (FNEditorUtils::IsUserControlled() && bAlwaysShowFrameRateAndMemory)
	{
		if (const UEditorPerformanceSettings* EditorPerformanceSettings = GetDefault<UEditorPerformanceSettings>();
			EditorPerformanceSettings != nullptr &&
			!EditorPerformanceSettings->bShowFrameRateAndMemory)
		{
			UEditorPerformanceSettings* Settings = GetMutableDefault<UEditorPerformanceSettings>();
			Settings->bShowFrameRateAndMemory = 1;
		}
	}
}

void UNEditorUserSettings::ApplySpaceToPan() const
{
	const FNCoreEditorModule& CoreEditorModule = FModuleManager::GetModuleChecked<FNCoreEditorModule>("NexusCoreEditor");
	CoreEditorModule.GetInputProcessor()->bCachedPanSetting = bSpaceToPan;
}
