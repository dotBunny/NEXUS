// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellBoundsGenerationSettings.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellBoundsGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { "NCELL_BoundsIgnore" };

	// TODO: Padding to surfaces ? character height

	bool Equals(const FNCellBoundsGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave && bIncludeNonColliding == Other.bIncludeNonColliding;
	}
};