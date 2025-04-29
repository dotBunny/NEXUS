// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDetailLayoutBuilderUtils.h"
#include "DetailLayoutBuilder.h"

void FNDetailLayoutBuilderUtils::HideDefaultCategories(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Tags"));
	DetailBuilder.HideCategory(TEXT("Activation"));
	DetailBuilder.HideCategory(TEXT("Cooking"));
	DetailBuilder.HideCategory(TEXT("AssetUserData"));
	DetailBuilder.HideCategory(TEXT("Navigation"));
}

void FNDetailLayoutBuilderUtils::HideVisualCategories(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Rendering"));
	DetailBuilder.HideCategory(TEXT("LOD"));
	DetailBuilder.HideCategory(TEXT("HLOD"));
}

void FNDetailLayoutBuilderUtils::HidePhysicsCategories(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Physics"));
	DetailBuilder.HideCategory(TEXT("Collision"));
}

void FNDetailLayoutBuilderUtils::HideNetworkingCategories(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Networking"));
	DetailBuilder.HideCategory(TEXT("Replication"));
}
void FNDetailLayoutBuilderUtils::HideActorCategories(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Actor"));
	DetailBuilder.HideCategory(TEXT("Input"));
}

void FNDetailLayoutBuilderUtils::HideLevelCategories(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("LevelInstance"));
	DetailBuilder.HideCategory(TEXT("WorldPartition"));
	DetailBuilder.HideCategory(TEXT("DataLayers"));
}
