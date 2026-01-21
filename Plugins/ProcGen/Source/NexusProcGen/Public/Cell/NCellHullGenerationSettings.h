// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Chaos/Convex.h"
#include "NCellHullGenerationSettings.generated.h"

// Keep in sync with Chaos::FConvexBuilder::EBuildMethod
UENUM(BlueprintType) 
enum class ENullBuildMethod : uint8
{
	Default = 0,
	Original = 1,
	ConvexHull3 = 2,
	ConvexHull3Simplified = 3,
};

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellHullGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;
	
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	UPROPERTY(VisibleAnywhere)
	ENullBuildMethod BuildMethod = ENullBuildMethod::Original;

	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { "NCELL_HullIgnore" };
	
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
	bool Equals(const FNCellHullGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly && BuildMethod == Other.BuildMethod
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};