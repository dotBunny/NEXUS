// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NEditorUserSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "Editor"))
class NEXUSCOREEDITOR_API UNEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNEditorUserSettings);

	static void Register() { FNEditorUtils::RegisterSettings(GetMutable()); }
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	static void OnPostEngineInit();
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Core (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("General user-specific framework settings for the Unreal Editor."));
		return SectionDescription;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(EditAnywhere, config,  Category = "Graphs", meta = (DisplayName = "Space To Pan", Tooltip = "While holding space down, left click and drag to pan the foreground graph."))
	bool bSpaceToPan = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", meta = (DisplayName = "Initial Editor Maximum FPS", Tooltip = "Limit the frame rate of editor, keeping our GPUs quiet and cool. Setting this to 0 will use the default engine value."))
	float EditorFrameRateLimit = 60.0f;

	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", meta = (DisplayName = "Always Show Frame Rate & Memory", Tooltip = "Ensures that the checkbox for Show Frame Rate & Memory remains checked, even if local settings are wiped."))
	bool bAlwaysShowFrameRateAndMemory = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers", meta = (DisplayName = "Line Thickness", Tooltip = "The thickness of debug lines that are drawn."))
	float DebugLineThickness = 2.0f;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers", meta = (DisplayName = "Circle Sides", Tooltip = "The number of lines used to draw a debug circle."))
	int DebugCircleSides = 64;

	UPROPERTY(VisibleAnywhere, Config, Category = "Updates", meta = (DisplayName = "Last Checked"))
	FDateTime UpdatesLastChecked = FDateTime(2025, 7, 12);
	
#endif // WITH_EDITORONLY_DATA

private:
	void ApplyEditorFrameRateLimit();
	void ApplyAlwaysShowFrameRateAndMemory() const;
	void ApplySpaceToPan() const;
};