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
	/** Grid dimensions in voxel units; Size.X/Y/Z is the extent consumed by the flat Data array. */
	UPROPERTY(VisibleAnywhere)
	FNVoxelCoordinate Size;

	/** Flat per-voxel storage for ENCellVoxel flag bitmasks, indexed via the generated N_IMPLEMENT_FLAT_3D_ARRAY accessors. */
	UPROPERTY(VisibleAnywhere)
	TArray<uint8> Data;

	N_IMPLEMENT_FLAT_3D_ARRAY(uint32, uint8, Data, Size.X, Size.Y, Size.Z)

	/** @return The raw stored byte reinterpreted as an ENCellVoxel bitmask. */
	FORCEINLINE static ENCellVoxel FlagFromValue(const uint8 Value)
	{
		return static_cast<ENCellVoxel>(Value);
	}

	/** @return The ENCellVoxel value cast back to the packed uint8 representation. */
	FORCEINLINE static uint8 ValueFromFlag(const ENCellVoxel Value)
	{
		return static_cast<uint8>(Value);
	}

	/** OR the supplied flag into the voxel at the flat Index. */
	void AddFlag(const uint32 Index, const ENCellVoxel Flag)
	{
		N_FLAGS_ADD(Data[Index], ValueFromFlag(Flag));
	}

	/** Clear the supplied flag on the voxel at the flat Index. */
	void RemoveFlag(const uint32 Index, const ENCellVoxel Flag)
	{
		N_FLAGS_REMOVE(Data[Index], ValueFromFlag(Flag));
	}

	/** OR the supplied flag into the voxel at (X, Y, Z). */
	void AddFlag(const uint32 X, const uint32 Y, const uint32 Z, const ENCellVoxel Flag)
	{
		N_FLAGS_ADD(Data[GetIndex(X, Y, Z)], ValueFromFlag(Flag));
	}

	/** Clear the supplied flag on the voxel at (X, Y, Z). */
	void RemoveFlag(const uint32 X, const uint32 Y, const uint32 Z, const ENCellVoxel Flag)
	{
		N_FLAGS_REMOVE(Data[GetIndex(X, Y, Z)], ValueFromFlag(Flag));
	}


	/** @return true if Other has identical dimensions and voxel contents; false otherwise. */
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
	/** @return true when every dimension is non-zero (i.e. the grid has been initialised). */
	bool IsValid() const
	{
		return Size.X > 0 && Size.Y > 0 && Size.Z > 0;
	}

	/**
	 * Rotate the voxel grid in place about a world-space pivot.
	 * @param WorldPoint Pivot in world space the grid is rotated around.
	 * @param Rotation Rotation applied to the grid.
	 * @note Not yet implemented.
	 */
	void RotatedAroundPivot(const FVector& WorldPoint, const FRotator& Rotation)
	{
		// TODO: Implement
	}

	/** World-space origin the voxel grid is anchored to; voxel (0,0,0) sits at Origin. */
	UPROPERTY(VisibleAnywhere)
	FVector Origin = FVector::ZeroVector;
};