// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "IPlacementModeModule.h"

/**
 * A collection of default values to use within the Nexus Editor modules.
 */
class NEXUSCOREEDITOR_API FNEditorDefaults
{
public:
	/**
	 * A default category to use in the asset creation context menu for NEXUS related assets.
	 */
	static FAssetCategoryPath AssetCategory;

	/**
	 * A container name to use for the editor settings.
	 */
	static FName GetEditorSettingsContainerName()
	{
		const FName ContainerName = TEXT("Editor");
		return ContainerName;
	}

	/**
	 * A category name to use for the editor settings.
	 */
	static FName GetEditorSettingsCategoryName()
	{
		const FName CategoryName = TEXT("NEXUS");
		return CategoryName;
	}


	/**
	 * 
	 * Gets the placement category for NEXUS related AActors.
	 * @return The qualified placement category for NEXUS related AActors.
	 */
	static const FPlacementCategoryInfo* GetPlacementCategory();
};