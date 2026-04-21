// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Macros/NArrayMacros.h"
#include "Macros/NFlagsMacros.h"
#include "Math/NVoxelUtils.h"

#include "NCellVoxelData.generated.h"

/**
 * Per-voxel flags for the cell occupancy grid.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENCellVoxel : uint8
{
	Empty		= 0,
	Occupied	= 1 << 0
};
ENUM_CLASS_FLAGS(ENCellVoxel)

/**
 * Flat 3D grid of ENCellVoxel flags describing occupancy inside a cell.
 *
 * Stored packed as a flat uint8 array keyed by (X, Y, Z); N_IMPLEMENT_FLAT_3D_ARRAY provides the
 * indexing helpers. Sized from the cell's unit-bounds / unit-size.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellVoxelData
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	FNVoxelCoordinate Size;
	
	UPROPERTY(VisibleAnywhere)
	TArray<uint8> Data;
	
	N_IMPLEMENT_FLAT_3D_ARRAY(uint32, uint8, Data, Size.X, Size.Y, Size.Z)
	
	FORCEINLINE static ENCellVoxel FlagFromValue(const uint8 Value)
	{
		return static_cast<ENCellVoxel>(Value);
	}
	
	FORCEINLINE static uint8 ValueFromFlag(const ENCellVoxel Value)
	{
		return static_cast<uint8>(Value);
	}
	
	void AddFlag(const uint32 Index, const ENCellVoxel Flag)
	{
		N_FLAGS_ADD(Data[Index], ValueFromFlag(Flag));
	}
	
	void RemoveFlag(const uint32 Index, const ENCellVoxel Flag)
	{
		N_FLAGS_REMOVE(Data[Index], ValueFromFlag(Flag));
	}
	
	void AddFlag(const uint32 X, const uint32 Y, const uint32 Z, const ENCellVoxel Flag)
	{
		N_FLAGS_ADD(Data[GetIndex(X, Y, Z)], ValueFromFlag(Flag));
	}
	
	void RemoveFlag(const uint32 X, const uint32 Y, const uint32 Z, const ENCellVoxel Flag)
	{
		N_FLAGS_REMOVE(Data[GetIndex(X, Y, Z)], ValueFromFlag(Flag));
	}
	
	
	bool IsEqual(const FNCellVoxelData& Other) const
	{
		if (Size.X != Other.Size.X || Size.Y != Other.Size.Y || Size.Z != Other.Size.Z) return false;
		
		const int Count = Size.X * Size.Y * Size.Z;
		for (int i = 0; i < Count; i++)
		{
			if (Data[i] != Other.Data[i]) return false;
		}
		
		return true;
	}
	bool IsValid() const
	{
		return Size.X > 0 && Size.Y > 0 && Size.Z > 0;
	}
	
	void RotatedAroundPivot(const FVector& WorldPoint, const FRotator& Rotation)
	{
		// TODO: Implement
	}
	
	UPROPERTY(VisibleAnywhere)
	FVector Origin = FVector::ZeroVector;
};