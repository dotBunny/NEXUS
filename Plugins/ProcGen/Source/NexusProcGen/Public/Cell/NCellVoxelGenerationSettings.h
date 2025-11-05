// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellVoxelGenerationSettings.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellVoxelGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	// TODO: Implement
	UPROPERTY(EditAnywhere) 
	bool bIncludeEditorOnly = false;

	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { "NCELL_VoxelIgnore" };

	bool Equals(const FNCellVoxelGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};