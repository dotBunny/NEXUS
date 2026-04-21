// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMeshLoop.generated.h"

/**
 * One ordered index loop within an FNRawMesh (triangle, quad, or n-gon).
 *
 * Indices are stored in winding order and reference entries in the parent mesh's Vertices array.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRawMeshLoop
{
	GENERATED_BODY()

	FNRawMeshLoop() = default;

	/** Triangle constructor. */
	FNRawMeshLoop(const int32 A, const int32 B, const int32 C)
	{
		Indices.Add(A);
		Indices.Add(B);
		Indices.Add(C);
	};

	/** Quad constructor. */
	FNRawMeshLoop(const int32 A, const int32 B, const int32 C, const int32 D)
	{
		Indices.Add(A);
		Indices.Add(B);
		Indices.Add(C);
		Indices.Add(D);
	};

	/** N-gon constructor accepting any vertex count. Moves InIndices into the loop. */
	FNRawMeshLoop(TArray<int32> InIndices)
		: Indices(MoveTemp(InIndices))
	{
	}

	/** Ordered list of vertex indices referencing the parent mesh's vertex buffer. */
	UPROPERTY(VisibleAnywhere)
	TArray<int> Indices;

	/** Exact order-sensitive equality with Other. */
	bool IsEqual(const FNRawMeshLoop& Other) const
	{
		return Indices == Other.Indices;
	}

	/** Returns true when the loop has exactly four vertices. */
	bool IsQuad() const
	{
		return Indices.Num() == 4;
	}

	/** Returns true when the loop has more than four vertices. */
	bool IsNgon() const
	{
		return Indices.Num() > 4;
	}

	/** Returns true when the loop has exactly three vertices. */
	bool IsTriangle() const
	{
		return Indices.Num() == 3;
	}
};