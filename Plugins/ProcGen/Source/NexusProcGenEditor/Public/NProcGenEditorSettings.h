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
#include "Types/NRotationConstraints.h"
#include "NProcGenEditorSettings.generated.h"

/**
 * Editor-side project settings for ProcGen.
 *
 * Stores the default cell generation settings applied to newly created cell actors so every new
 * cell starts from the studio's preferred defaults without the user re-filling each field.
 */
UCLASS(config = NexusEditor, defaultconfig)
class UNProcGenEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	UNProcGenEditorSettings()
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultCollisionVisualizerAsset(TEXT("/NexusProcGen/M_NCellProxy.M_NCellProxy"));
		if (DefaultCollisionVisualizerAsset.Succeeded())
		{
			CollisionVisualizerMaterial = DefaultCollisionVisualizerAsset.Object;
		}
	}
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

	/** Default rotation constraints applied to newly-created cell roots. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Rotation Constraints",
		meta=(ToolTip=""))
	FNRotationConstraints CellRotationConstraints;

	/** Default bounds-generation settings applied to newly-created cell roots. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Bounds Generation",
		meta=(ToolTip=""))
	FNCellBoundsGenerationSettings CellBoundsGenerationSettings;

	/** Default voxel-generation settings applied to newly-created cell roots. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Voxel Generation",
		meta=(ToolTip=""))
	FNCellVoxelGenerationSettings CellVoxelGenerationSettings;

	/** Default hull-generation settings applied to newly-created cell roots. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Hull Generation",
		meta=(ToolTip=""))
	FNCellHullGenerationSettings CellHullGenerationSettings;

	/** Default proxy color applied to newly-created cell roots. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cell (Defaults)", DisplayName="Proxy Color",
		meta=(ToolTip=""))
	FLinearColor CellProxyColor;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "World Context", DisplayName="Collision Visualizer Material")
	TSoftObjectPtr<UMaterialInterface> CollisionVisualizerMaterial;

	/** Copy every Cell default onto CellRoot's Details struct — called from the cell-actor factory. */
	void ApplyDefaultSettings(UNCellRootComponent* CellRoot) const
	{
		CellRoot->Details.BoundsSettings = CellBoundsGenerationSettings;
		CellRoot->Details.HullSettings = CellHullGenerationSettings;
		CellRoot->Details.VoxelSettings = CellVoxelGenerationSettings;

		CellRoot->Details.RotationConstraints = CellRotationConstraints;
		CellRoot->Details.ProxyColor = CellProxyColor;
	}
};