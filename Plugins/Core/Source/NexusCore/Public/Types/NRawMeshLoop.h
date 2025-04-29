// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMeshLoop.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRawMeshLoop
{
	GENERATED_BODY()

	FNRawMeshLoop() = default;

	FNRawMeshLoop(const int A, const int B, const int C)
	{
		Indices.Add(A);
		Indices.Add(B);
		Indices.Add(C);
	};

	FNRawMeshLoop(const int A, const int B, const int C, const int32 D)
	{
		Indices.Add(A);
		Indices.Add(B);
		Indices.Add(C);
		Indices.Add(D);
	};
	
	FNRawMeshLoop(TArray<int32> InIndices)
		: Indices(MoveTemp(InIndices))
	{
	}
	
	UPROPERTY(VisibleAnywhere)
	TArray<int> Indices;

	bool IsEqual(const FNRawMeshLoop& Other) const
	{
		return Indices == Other.Indices;
	}

	bool IsQuad() const
	{
		return Indices.Num() == 4;
	}

	bool IsTriangle() const
	{
		return Indices.Num() == 3;
	}
};