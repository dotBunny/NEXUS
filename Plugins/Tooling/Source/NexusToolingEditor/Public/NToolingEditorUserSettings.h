// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NToolingEditorUserSettings.generated.h"

UCLASS(config = NexusUserSettings, meta = (DisplayName = "Tooling (User)"))
class NEXUSTOOLINGEDITOR_API UNToolingEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolingEditorUserSettings);
	
	static void OnPostEngineInit();
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Tooling (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Specific settings for NEXUS: Tooling included with the framework."));
		return SectionDescription;
	}
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, config,  Category="Graph Navigation", DisplayName="Space To Pan", meta=(Tooltip="While holding space down, left click and drag to pan the foreground graph."))
	bool bGraphNavigationSpaceToPan = true;

	UPROPERTY(EditAnywhere, config,  Category="Graph Navigation", DisplayName = "Pan Speed Multiplier", meta=(Tooltip="The multiplier applied when dragging with the Space To Pan feature."))
	float GraphNavigationPanSpeedMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", DisplayName = "Initial Editor Maximum FPS", meta=(Tooltip="Limit the frame rate of editor, keeping our GPUs quiet and cool. Setting this to 0 will use the default engine value."))
	float EditorFrameRateLimit = 60.0f;

	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", DisplayName = "Always Show Frame Rate & Memory", meta=(Tooltip="Ensures that the checkbox for Show Frame Rate & Memory remains checked, even if local settings are wiped."))
	bool bAlwaysShowFrameRateAndMemory = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Leak Check", DisplayName ="Duration", meta=(ToolTip="How long should the leak check run?"))
	float LeakCheckTime = 30.f;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers: Distribution", DisplayName = "Outer Color")
	FColor VisualizationDistributionOuterColor = FColor(255,0,88);
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers: Distribution", DisplayName = "Inner Color")
	FColor VisualizationDistributionInnerColor = FColor(150,0,89);
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers: Distribution", DisplayName = "Line Thickness", meta = (Tooltip = "The thickness of debug lines that are drawn."))
	float VisualizationLineThickness = 1.5f;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers: Distribution", DisplayName = "Circle Sides",  meta = (Tooltip = "The number of lines used to draw a debug circle."))
	int VisualizationCircleSides = 64;
	
#endif // WITH_EDITORONLY_DATA

private:
	void ApplyEditorFrameRateLimit() const;
	void ApplyAlwaysShowFrameRateAndMemory() const;
	void ApplySpaceToPan() const;
};