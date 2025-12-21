// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorDefaults.h"

#include "NEditorStyle.h"

FAssetCategoryPath FNEditorDefaults::AssetCategory(FText::FromName(GetEditorSettingsCategoryName()));

const FPlacementCategoryInfo* FNEditorDefaults::GetPlacementCategory()
{
	const IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
	const FName PlacementModeCategoryHandle = TEXT("NEXUS");

	if (const FPlacementCategoryInfo* RegisteredInfo = 
		PlacementModeModule.GetRegisteredPlacementCategory(PlacementModeCategoryHandle))
	{
		return RegisteredInfo;
	}

	// Ensure we are initialized
	FNEditorStyle::Initialize();
	
	if (const ISlateStyle* Style = &FNEditorStyle::Get(); 
		Style != nullptr)
	{
		FPlacementCategoryInfo Info(
			NSLOCTEXT("NexusCoreEditor", "PlacementCategoryName", "NEXUS"),
			FSlateIcon(Style->GetStyleSetName(), "NEXUS.Icon"),
			PlacementModeCategoryHandle,
			TEXT("NEXUS"),
			42 // Determines where the category shows up in the list with respect to the others.
		);

		Info.ShortDisplayName = NSLOCTEXT("NexusCoreEditor", "PlacementCategoryShortName", "NEXUS");
		IPlacementModeModule::Get().RegisterPlacementCategory(Info);

		return PlacementModeModule.GetRegisteredPlacementCategory(PlacementModeCategoryHandle);
	}
	else
	{
		return nullptr;
	}
}
