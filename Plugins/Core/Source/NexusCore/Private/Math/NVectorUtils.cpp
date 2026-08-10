// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NVectorUtils.h"

TArray<FVector> FNVectorUtils::RotateAndOffsetPoints(const TArray<FVector>& Points, const FRotator& Rotation, const FVector& Offset)
{
	TArray<FVector> ModifiedVertices;
	ModifiedVertices.Reserve(Points.Num());

	for (const FVector& Vertex : Points)
	{
		ModifiedVertices.Add(Rotation.RotateVector(Vertex) + Offset);
	}

	return ModifiedVertices;
}

TArray<FVector> FNVectorUtils::RotatePoints(const TArray<FVector>& Vectors, const FRotator& Rotation)
{
	TArray<FVector> ModifiedVertices;
	ModifiedVertices.Reserve(Vectors.Num());

	for (const FVector& Vertex : Vectors)
	{
		ModifiedVertices.Add(Rotation.RotateVector(Vertex));
	}

	return ModifiedVertices;
}

TArray<FVector> FNVectorUtils::OffsetPoints(const TArray<FVector>& Vectors, const FVector& Offset)
{
	TArray<FVector> ModifiedVertices;
	ModifiedVertices.Reserve(Vectors.Num());

	for (const FVector& Vertex : Vectors)
	{
		ModifiedVertices.Add(Vertex+Offset);
	}

	return ModifiedVertices;
}

void FNVectorUtils::GridReducePoints(const TArray<FVector>& Points, const FVector& Center, const double GridSize,
	TSet<FIntVector>& SeenCells, TArray<FVector>& OutPoints)
{
	// Source topology plays no part here: a cave's inner surface and the far side of a spherical terrain are interior
	// points to a convex hull either way, so they are discarded by the builder whether or not they are thinned first.
	if (GridSize <= 0.0)
	{
		OutPoints.Append(Points);
		return;
	}

	for (const FVector& Point : Points)
	{
		const FIntVector Cell(
			FMath::FloorToInt32(Point.X / GridSize),
			FMath::FloorToInt32(Point.Y / GridSize),
			FMath::FloorToInt32(Point.Z / GridSize));

		bool bAlreadySeen = false;
		SeenCells.Add(Cell, &bAlreadySeen);
		if (bAlreadySeen) continue;

		// Away from the center on each axis independently.
		OutPoints.Add(FVector(
			Point.X >= Center.X ? FMath::CeilToDouble(Point.X / GridSize) * GridSize : FMath::FloorToDouble(Point.X / GridSize) * GridSize,
			Point.Y >= Center.Y ? FMath::CeilToDouble(Point.Y / GridSize) * GridSize : FMath::FloorToDouble(Point.Y / GridSize) * GridSize,
			Point.Z >= Center.Z ? FMath::CeilToDouble(Point.Z / GridSize) * GridSize : FMath::FloorToDouble(Point.Z / GridSize) * GridSize));
	}
}