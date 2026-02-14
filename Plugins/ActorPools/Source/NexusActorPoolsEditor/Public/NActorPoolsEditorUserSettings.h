// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NActorPoolsEditorUserSettings.generated.h"

UCLASS(config = NexusUserSettings, meta = (DisplayName = "Actor Pools (User)"))
class NEXUSACTORPOOLSEDITOR_API UNActorPoolsEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNActorPoolsEditorUserSettings);

	static void Register() { FNEditorUtils::RegisterSettings(GetMutable()); }
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	static void OnPostEngineInit();
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Actor Pools (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Specific settings for NEXUS: Actor Pools included with the framework."));
		return SectionDescription;
	}

#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers", meta = (DisplayName = "Line Thickness", Tooltip = "The thickness of debug lines that are drawn."))
	float DebugLineThickness = 1.25f;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers", meta = (DisplayName = "Circle Sides", Tooltip = "The number of lines used to draw a debug circle."))
	int DebugCircleSides = 64;

	UPROPERTY(EditAnywhere, config, Category = "Visualizers", DisplayName = "Distribution: Outer Color")
	FColor DistributionOuterColor = FColor(255,0,88);
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers", DisplayName = "Distribution: Inner Color")
	FColor DistributionInnerColor = FColor(150,0,89);

#endif // WITH_EDITORONLY_DATA
};