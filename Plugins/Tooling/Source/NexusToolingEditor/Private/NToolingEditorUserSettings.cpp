// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorUserSettings.h"

#include "NEditorInputProcessor.h"
#include "NEditorUtils.h"
#include "NToolingEditorMinimal.h"
#include "NToolingEditorModule.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GameFramework/GameUserSettings.h"

void UNToolingEditorUserSettings::OnPostEngineInit()
{
	if (const UNToolingEditorUserSettings* Settings = Get())
	{
		Settings->ApplyEditorFrameRateLimit();
		Settings->ApplyAlwaysShowFrameRateAndMemory();
		Settings->ApplySpaceToPan();
	}
}

void UNToolingEditorUserSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Check if we are updating the frame rate
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNToolingEditorUserSettings, EditorFrameRateLimit))
	{
		ApplyEditorFrameRateLimit();
	}

	ApplySpaceToPan();
}

void UNToolingEditorUserSettings::ApplyEditorFrameRateLimit() const
{
	if (FNEditorUtils::IsUserControlled())
	{
		UE_LOG(LogNexusToolingEditor, Log, TEXT("Apply framerate limit(%f)."), EditorFrameRateLimit);

		GEngine->SetMaxFPS(EditorFrameRateLimit);
		GEngine->GameUserSettings->SetFrameRateLimit(EditorFrameRateLimit);
	}
}

void UNToolingEditorUserSettings::ApplyAlwaysShowFrameRateAndMemory() const
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

void UNToolingEditorUserSettings::ApplySpaceToPan() const
{
	if (FNEditorUtils::IsUserControlled())
	{
		const FNToolingEditorModule& ToolingModule = FModuleManager::GetModuleChecked<FNToolingEditorModule>("NexusToolingEditor");
		FNEditorInputProcessor* InputProcessor = ToolingModule.GetInputProcessor();
		InputProcessor->bCachedGraphNavigationSpaceToPan = bGraphNavigationSpaceToPan;
		InputProcessor->CachedGraphNavigationPanSpeedMultiplier = GraphNavigationPanSpeedMultiplier;
	}
}
