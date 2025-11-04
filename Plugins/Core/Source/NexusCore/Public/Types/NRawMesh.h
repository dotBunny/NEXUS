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
	friend class FNCellRootComponentVisualizer;
	
	GENERATED_BODY()

	/**
	 * The vertices of the mesh.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Vertices;

	/**
	 * The relative center of the mesh.
	 */
	UPROPERTY(VisibleAnywhere)
	FVector Center = FVector::ZeroVector;

	/**
	 * Ordered shape-edge definition.
	 * @note The indices are ordered, so 1/2/3 for a triangle. This is NOT looped; therefore, you must close the loop if line drawing.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FNRawMeshLoop> Loops;

	TArray<int32> GetFlatIndices();

	void ConvertToTriangles();
	
	bool IsConvex() const { return bIsConvex; }
	bool HasNonTris() const { return bHasNonTris; }

	bool IsEqual(const FNRawMesh& Other) const
	{
		if (Loops.Num() != Other.Loops.Num()) return false;
		for (int32 i = 0; i < Loops.Num(); i++)
		{
			if (!Loops[i].IsEqual(Other.Loops[i])) return false;
		}
		return Vertices == Other.Vertices 
			&& bIsConvex == Other.bIsConvex 
			&& bHasNonTris == Other.bHasNonTris 
			&& bIsChaosGenerated == Other.bIsChaosGenerated;
	}
	
	void Validate()
	{
		if (bIsChaosGenerated)
		{
			return;
		}
		bIsConvex = CheckConvex();
		bHasNonTris = CheckNonTris();
	}

	FDynamicMesh3 CreateDynamicMesh(bool bProcessMesh = false);
	
private:
	bool CheckConvex();
	bool CheckNonTris();
	
	UPROPERTY(VisibleAnywhere)
	bool bIsConvex = false;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsChaosGenerated = false;

	UPROPERTY(VisibleAnywhere)
	bool bHasNonTris = false;
};