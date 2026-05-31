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
	Occupied	= 1
};
ENUM_CLASS_FLAGS(ENCellVoxel)

/**
 * Flat 3D grid of ENCellVoxel flags describing occupancy inside a cell.
 *
 * Stored packed as a flat uint8 array keyed by (X, Y, Z); N_IMPLEMENT_FLAT_3D_ARRAY provides the
 * indexing helpers. Sized from the cell's unit-bounds / unit-size.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellVoxelData
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

		const int32 Count = Size.X * Size.Y * Size.Z;
		for (int32 i = 0; i < Count; i++)
		{
			if (Data[i] != Other.Data[i]) return false;
		}

		return true;
	}
	/** @return true when every dimension is non-zero (i.e. the grid has been initialized). */
	bool IsValid() const
	{
		return Size.X > 0 && Size.Y > 0 && Size.Z > 0;
	}

	/** @return Grid dimensions in voxel units. */
	FORCEINLINE FNVoxelCoordinate GetSize() const
	{
		return Size;
	}

	/** @return World-space origin the grid is anchored to; voxel (0,0,0) sits at Origin. */
	FORCEINLINE const FVector& GetOrigin() const
	{
		return Origin;
	}

	/**
	 * Rotate the voxel grid in place about a world-space pivot, re-anchoring it onto the shared world
	 * voxel lattice so independently-placed cells can be compared cell-for-cell.
	 *
	 * The grid is re-voxelized: the rotated occupancy is refit to a new axis-aligned footprint whose
	 * Origin is snapped to the global VoxelSize lattice (anchored at the world origin) and whose Size
	 * grows to cover the rotated bounds. Cardinal (90-degree) rotations on uniform voxels take an exact
	 * integer forward-map touching only occupied voxels; any other rotation inverse-samples each
	 * destination voxel back into the source grid so no occupancy is dropped.
	 *
	 * @param WorldPoint Pivot in world space the grid is rotated around.
	 * @param Rotation Rotation applied to the grid.
	 * @param VoxelSize World-space size of a single voxel (cached from UNWorldAssemblySettings::VoxelSize).
	 * @note Resampling onto the shared lattice is approximate to within roughly one voxel; it is intended
	 *       for broad occupancy-overlap tests, not exact volume reconstruction.
	 */
	void RotateAroundPivot(const FVector& WorldPoint, const FRotator& Rotation, const FVector& VoxelSize)
	{
		if (!IsValid() || Data.Num() == 0) return;

		// Snapshot the source grid; Reset below reallocates Data against the new dimensions.
		const FVector OldOrigin = Origin;
		const FNVoxelCoordinate OldSize = Size;
		const TArray<uint8> OldData = Data;
		const FVector HalfVoxel = VoxelSize * 0.5;

		// Rotate the 8 corners of the source grid's world AABB about the pivot to find the new footprint.
		const FVector SourceMin = OldOrigin;
		const FVector SourceMax = OldOrigin + FVector(OldSize.X, OldSize.Y, OldSize.Z) * VoxelSize;
		FBox RotatedBounds(ForceInit);
		for (int32 Corner = 0; Corner < 8; ++Corner)
		{
			const FVector Point(
				(Corner & 1) ? SourceMax.X : SourceMin.X,
				(Corner & 2) ? SourceMax.Y : SourceMin.Y,
				(Corner & 4) ? SourceMax.Z : SourceMin.Z);
			RotatedBounds += Rotation.RotateVector(Point - WorldPoint) + WorldPoint;
		}

		// Snap the new origin down onto the global lattice and size up to fully cover the rotated bounds.
		const FVector MinUnits(
			LatticeFloor(RotatedBounds.Min.X / VoxelSize.X),
			LatticeFloor(RotatedBounds.Min.Y / VoxelSize.Y),
			LatticeFloor(RotatedBounds.Min.Z / VoxelSize.Z));
		const FVector MaxUnits(
			LatticeCeil(RotatedBounds.Max.X / VoxelSize.X),
			LatticeCeil(RotatedBounds.Max.Y / VoxelSize.Y),
			LatticeCeil(RotatedBounds.Max.Z / VoxelSize.Z));

		const FVector NewOrigin = MinUnits * VoxelSize;
		const uint32 NewSizeX = static_cast<uint32>(FMath::Max(1.0, MaxUnits.X - MinUnits.X));
		const uint32 NewSizeY = static_cast<uint32>(FMath::Max(1.0, MaxUnits.Y - MinUnits.Y));
		const uint32 NewSizeZ = static_cast<uint32>(FMath::Max(1.0, MaxUnits.Z - MinUnits.Z));

		Reset(NewSizeX, NewSizeY, NewSizeZ);
		Origin = NewOrigin;

		if (IsCardinalRotation(Rotation) && IsUniformVoxelSize(VoxelSize))
		{
			// Exact integer forward-map: a cardinal rotation on cubic voxels is a lattice bijection, so we
			// only need to touch occupied source voxels rather than sweeping the whole destination volume.
			for (uint32 z = 0; z < OldSize.Z; ++z)
			{
				for (uint32 y = 0; y < OldSize.Y; ++y)
				{
					for (uint32 x = 0; x < OldSize.X; ++x)
					{
						const uint32 SourceIndex = x + (y * OldSize.X) + (z * OldSize.X * OldSize.Y);
						const uint8 Value = OldData[SourceIndex];
						if (Value == ValueFromFlag(ENCellVoxel::Empty)) continue;

						const FVector SourceCenter = OldOrigin + FVector(x, y, z) * VoxelSize + HalfVoxel;
						const FVector DestWorld = Rotation.RotateVector(SourceCenter - WorldPoint) + WorldPoint;
						const FVector DestLocal = (DestWorld - Origin) / VoxelSize;
						const int32 DestX = static_cast<int32>(FMath::FloorToDouble(DestLocal.X));
						const int32 DestY = static_cast<int32>(FMath::FloorToDouble(DestLocal.Y));
						const int32 DestZ = static_cast<int32>(FMath::FloorToDouble(DestLocal.Z));
						if (DestX < 0 || DestY < 0 || DestZ < 0 ||
							DestX >= static_cast<int32>(Size.X) || DestY >= static_cast<int32>(Size.Y) || DestZ >= static_cast<int32>(Size.Z)) continue;

						Data[GetIndex(DestX, DestY, DestZ)] = Value;
					}
				}
			}
			return;
		}

		// Inverse-sample every destination voxel back into the source grid; this guarantees no holes when
		// the rotated grid grows past the source footprint (as it does for non-cardinal rotations).
		for (uint32 z = 0; z < Size.Z; ++z)
		{
			for (uint32 y = 0; y < Size.Y; ++y)
			{
				for (uint32 x = 0; x < Size.X; ++x)
				{
					const FVector DestCenter = Origin + FVector(x, y, z) * VoxelSize + HalfVoxel;
					const FVector SourceWorld = Rotation.UnrotateVector(DestCenter - WorldPoint) + WorldPoint;
					const FVector SourceLocal = (SourceWorld - OldOrigin) / VoxelSize;
					const int32 SourceX = static_cast<int32>(FMath::FloorToDouble(SourceLocal.X));
					const int32 SourceY = static_cast<int32>(FMath::FloorToDouble(SourceLocal.Y));
					const int32 SourceZ = static_cast<int32>(FMath::FloorToDouble(SourceLocal.Z));
					if (SourceX < 0 || SourceY < 0 || SourceZ < 0 ||
						SourceX >= static_cast<int32>(OldSize.X) || SourceY >= static_cast<int32>(OldSize.Y) || SourceZ >= static_cast<int32>(OldSize.Z)) continue;

					const uint32 SourceIndex = SourceX + (SourceY * OldSize.X) + (SourceZ * OldSize.X * OldSize.Y);
					Data[GetIndex(x, y, z)] = OldData[SourceIndex];
				}
			}
		}
	}

	/** World-space origin the voxel grid is anchored to; voxel (0,0,0) sits at Origin. */
	UPROPERTY(VisibleAnywhere)
	FVector Origin = FVector::ZeroVector;

private:
	/**
	 * Floor a unit-space value to the lattice, first snapping values within tolerance of a grid line onto it.
	 * Guards against float error in the rotated corners spilling the footprint into a spurious voxel layer.
	 */
	FORCEINLINE static double LatticeFloor(const double Value)
	{
		const double Rounded = FMath::RoundToDouble(Value);
		return FMath::IsNearlyEqual(Value, Rounded, 1.e-4) ? Rounded : FMath::FloorToDouble(Value);
	}

	/** Ceil a unit-space value to the lattice, snapping values within tolerance of a grid line onto it first. */
	FORCEINLINE static double LatticeCeil(const double Value)
	{
		const double Rounded = FMath::RoundToDouble(Value);
		return FMath::IsNearlyEqual(Value, Rounded, 1.e-4) ? Rounded : FMath::CeilToDouble(Value);
	}

	/** @return true when all three components of VoxelSize are equal (cubic voxels). */
	FORCEINLINE static bool IsUniformVoxelSize(const FVector& VoxelSize)
	{
		return FMath::IsNearlyEqual(VoxelSize.X, VoxelSize.Y) && FMath::IsNearlyEqual(VoxelSize.Y, VoxelSize.Z);
	}

	/** @return true when Value is within tolerance of 0 or +/-1, i.e. a clean axis component. */
	FORCEINLINE static bool IsNearlyAxisComponent(const double Value)
	{
		const double Magnitude = FMath::Abs(Value);
		return Magnitude < 1.e-3 || FMath::IsNearlyEqual(Magnitude, 1.0, 1.e-3);
	}

	/** @return true when Rotation maps every basis axis onto an axis, i.e. it is a 90-degree multiple on all axes. */
	static bool IsCardinalRotation(const FRotator& Rotation)
	{
		const FVector Axes[3] = {
			Rotation.RotateVector(FVector::XAxisVector),
			Rotation.RotateVector(FVector::YAxisVector),
			Rotation.RotateVector(FVector::ZAxisVector)};
		for (const FVector& Axis : Axes)
		{
			if (!(IsNearlyAxisComponent(Axis.X) && IsNearlyAxisComponent(Axis.Y) && IsNearlyAxisComponent(Axis.Z)))
			{
				return false;
			}
		}
		return true;
	}
};