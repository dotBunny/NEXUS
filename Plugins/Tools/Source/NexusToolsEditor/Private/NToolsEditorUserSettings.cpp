// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolsEditorUserSettings.h"

#include "NCoreEditorModule.h"
#include "NEditorInputProcessor.h"
#include "NToolsEditorMinimal.h"
#include "NToolsEditorModule.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GameFramework/GameUserSettings.h"

void UNToolsEditorUserSettings::OnPostEngineInit()
{
	if (const UNToolsEditorUserSettings* Settings = Get())
	{
		Settings->ApplyEditorFrameRateLimit();
		Settings->ApplyAlwaysShowFrameRateAndMemory();
		Settings->ApplySpaceToPan();
	}
}

void UNToolsEditorUserSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Check if we are updating the frame rate
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNToolsEditorUserSettings, EditorFrameRateLimit))
	{
		ApplyEditorFrameRateLimit();
	}

	ApplySpaceToPan();
}

void UNToolsEditorUserSettings::ApplyEditorFrameRateLimit() const
{
	if (FNEditorUtils::IsUserControlled())
	{
		UE_LOG(LogNexusToolsEditor, Log, TEXT("Apply framerate limit(%f)."), EditorFrameRateLimit);

		GEngine->SetMaxFPS(EditorFrameRateLimit);
		GEngine->GameUserSettings->SetFrameRateLimit(EditorFrameRateLimit);
	}
}

void UNToolsEditorUserSettings::ApplyAlwaysShowFrameRateAndMemory() const
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

void UNToolsEditorUserSettings::ApplySpaceToPan() const
{
	if (FNEditorUtils::IsUserControlled())
	{
		const FNToolsEditorModule& ToolsEditorModule = FModuleManager::GetModuleChecked<FNToolsEditorModule>("NexusToolsEditor");
		FNEditorInputProcessor* InputProcessor = ToolsEditorModule.GetInputProcessor();
		InputProcessor->bCachedGraphNavigationSpaceToPan = bGraphNavigationSpaceToPan;
		InputProcessor->CachedGraphNavigationPanSpeedMultiplier = GraphNavigationPanSpeedMultiplier;
	}
}
