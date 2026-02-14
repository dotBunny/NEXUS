// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NToolsEditorUserSettings.generated.h"

UCLASS(config = NexusUserSettings, meta = (DisplayName = "Tools (User)"))
class NEXUSTOOLSEDITOR_API UNToolsEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolsEditorUserSettings);

	static void Register() { FNEditorUtils::RegisterSettings(GetMutable()); }
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	
	static void OnPostEngineInit();
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Tools (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Specific settings for NEXUS: Tools included with the framework."));
		return SectionDescription;
	}
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, config,  Category = "Graph Navigation", meta = (DisplayName = "Space To Pan", Tooltip = "While holding space down, left click and drag to pan the foreground graph."))
	bool bGraphNavigationSpaceToPan = true;

	UPROPERTY(EditAnywhere, config,  Category = "Graph Navigation", meta = (DisplayName = "Pan Speed Multiplier", Tooltip = "The multiplier applied when dragging with the Space To Pan feature."))
	float GraphNavigationPanSpeedMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", meta = (DisplayName = "Initial Editor Maximum FPS", Tooltip = "Limit the frame rate of editor, keeping our GPUs quiet and cool. Setting this to 0 will use the default engine value."))
	float EditorFrameRateLimit = 60.0f;

	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", meta = (DisplayName = "Always Show Frame Rate & Memory", Tooltip = "Ensures that the checkbox for Show Frame Rate & Memory remains checked, even if local settings are wiped."))
	bool bAlwaysShowFrameRateAndMemory = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Leak Check", DisplayName ="Duration", meta=(ToolTip="How long should the leak check run?"))
	float LeakCheckTime = 30.f;
	
#endif // WITH_EDITORONLY_DATA

private:
	void ApplyEditorFrameRateLimit() const;
	void ApplyAlwaysShowFrameRateAndMemory() const;
	void ApplySpaceToPan() const;
};