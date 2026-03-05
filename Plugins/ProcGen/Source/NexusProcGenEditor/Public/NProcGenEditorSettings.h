// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "Cell/NCellBoundsGenerationSettings.h"
#include "Cell/NCellHullGenerationSettings.h"
#include "Cell/NCellVoxelGenerationSettings.h"
#include "Cell/NCellRootComponent.h"
#include "Types/NRotationConstraint.h"
#include "NProcGenEditorSettings.generated.h"

UCLASS(config = NexusEditor, defaultconfig)
class UNProcGenEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNProcGenEditorSettings);
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Procedural Generation"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Editor side settings used in NProcGen."));
		return SectionDescription;
	}
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Rotation Constraints",
		meta=(ToolTip=""))
	FNRotationConstraint CellRotationConstraints;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Bounds Generation",
		meta=(ToolTip=""))
	FNCellBoundsGenerationSettings CellBoundsGenerationSettings;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Voxel Generation",
		meta=(ToolTip=""))
	FNCellVoxelGenerationSettings CellVoxelGenerationSettings;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Hull Generation",
		meta=(ToolTip=""))
	FNCellHullGenerationSettings CellHullGenerationSettings;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Proxy Color",
		meta=(ToolTip=""))
	FLinearColor CellProxyColor;
	
	void ApplyDefaultSettings(UNCellRootComponent* CellRoot) const
	{
		CellRoot->Details.BoundsSettings = CellBoundsGenerationSettings;
		CellRoot->Details.HullSettings = CellHullGenerationSettings;
		CellRoot->Details.VoxelSettings = CellVoxelGenerationSettings;
		
		CellRoot->Details.RotationConstraints = CellRotationConstraints;
		CellRoot->Details.ProxyColor = CellProxyColor;
	}
};