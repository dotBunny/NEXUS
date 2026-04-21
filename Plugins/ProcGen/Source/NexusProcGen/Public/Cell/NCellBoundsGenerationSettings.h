// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellBoundsGenerationSettings.generated.h"

/**
 * Settings controlling how a cell's world-space bounds are computed from its level content.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellBoundsGenerationSettings
{
	GENERATED_BODY()

	/** When true, bounds are recomputed automatically whenever the cell is saved. */
	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	/** When true, actors without collision still contribute to bounds. */
	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	/** When true, editor-only actors contribute to bounds. */
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	/** Actors carrying any of these tags are excluded from the bounds calculation. */
	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { "NCELL_BoundsIgnore" };

	// TODO: Padding to surfaces ? character height
	/** @return true if all fields match structurally; used when diffing live vs. side-car cell data. */
	bool Equals(const FNCellBoundsGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};