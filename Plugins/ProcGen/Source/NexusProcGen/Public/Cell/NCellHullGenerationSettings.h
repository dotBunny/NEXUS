// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Chaos/Convex.h"
#include "NCellHullGenerationSettings.generated.h"

/**
 * Convex-hull construction algorithm — kept in sync with Chaos::FConvexBuilder::EBuildMethod.
 */
UENUM(BlueprintType)
enum class ENullBuildMethod : uint8
{
	Default = 0,
	Original = 1,
	ConvexHull3 = 2,
	ConvexHull3Simplified = 3,
};

/**
 * Settings controlling how a cell's convex hull is generated from its level content.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellHullGenerationSettings
{
	GENERATED_BODY()

	/** When true, the hull is recomputed automatically whenever the cell is saved. */
	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	/** When true, actors without collision still contribute to the hull. */
	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	/** When true, editor-only actors contribute to the hull. */
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	/** Algorithm used to build the convex hull. */
	UPROPERTY(VisibleAnywhere)
	ENullBuildMethod BuildMethod = ENullBuildMethod::Original;

	/** Actors carrying any of these tags are excluded from the hull calculation. */
	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { "NCELL_HullIgnore" };

	/** @return The Chaos enum matching BuildMethod. */
	Chaos::FConvexBuilder::EBuildMethod GetChaosBuildMethod() const
	{
		switch (BuildMethod)
		{
			using enum ENullBuildMethod;
		case Default:	
			return Chaos::FConvexBuilder::EBuildMethod::Default;
		case Original:
			return Chaos::FConvexBuilder::EBuildMethod::Original;
		case ConvexHull3:
			return Chaos::FConvexBuilder::EBuildMethod::ConvexHull3;
		case ConvexHull3Simplified:
			return Chaos::FConvexBuilder::EBuildMethod::ConvexHull3Simplified;
		default:
			return Chaos::FConvexBuilder::EBuildMethod::Default;
		}
	}

	// TODO: Padding to surfaces ? character height
	/** @return true if all fields match structurally; used when diffing live vs. side-car cell data. */
	bool Equals(const FNCellHullGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly && BuildMethod == Other.BuildMethod
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};