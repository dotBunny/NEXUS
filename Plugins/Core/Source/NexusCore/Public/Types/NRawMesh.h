// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/BaseDynamicMeshComponent.h"
#include "Types/NRawMeshLoop.h"
#include "NRawMesh.generated.h"


USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRawMesh
{
	friend class FNProcGenUtils;
	
	GENERATED_BODY()

	/**
	 * The vertices of the mesh.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Vertices;

	/**
	 * Ordered shape edge definition.
	 * @remark The indices are ordered, so 1/2/3 for a triangle. This is NOT looped, therefore you must close the loop if line drawing.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FNRawMeshLoop> Loops;

	TArray<int32> GetFlatIndices();

	void ConvertToTriangles();
	
	bool IsConvex() const { return bIsConvex; }

	bool IsEqual(const FNRawMesh& Other) const
	{
		if (Loops.Num() != Other.Loops.Num()) return false;
		for (int i = 0; i < Loops.Num(); i++)
		{
			if (!Loops[i].IsEqual(Other.Loops[i])) return false;
		}
		return Vertices == Other.Vertices &&
			bIsConvex == Other.bIsConvex;
	}
	
	void Validate()
	{
		bIsConvex = CheckConvex();
		bHasQuads = CheckQuads();
	}

	FDynamicMesh3 CreateDynamicMesh(bool bProcessMesh = false);
	
private:
	bool CheckConvex();
	bool CheckQuads();
	
	UPROPERTY(VisibleAnywhere)
	bool bIsConvex = false;

	UPROPERTY(VisibleAnywhere)
	bool bHasQuads = false;
};