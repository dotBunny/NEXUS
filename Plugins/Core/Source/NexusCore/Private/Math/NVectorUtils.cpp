// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NVectorUtils.h"

TArray<FVector> FNVectorUtils::RotateAndOffsetVectors(const TArray<FVector>& Vectors, const FRotator& Rotation, const FVector& Offset)
{
	TArray<FVector> ModifiedVertices;
	ModifiedVertices.Reserve(Vectors.Num());
		
	for (const FVector& Vertex : Vectors)
	{
		ModifiedVertices.Add(Rotation.RotateVector(Vertex) + Offset);
	}
		
	return MoveTemp(ModifiedVertices);
}

TArray<FVector> FNVectorUtils::RotateVectors(const TArray<FVector>& Vectors, const FRotator& Rotation)
{
	TArray<FVector> ModifiedVertices;
	ModifiedVertices.Reserve(Vectors.Num());
		
	for (const FVector& Vertex : Vectors)
	{
		ModifiedVertices.Add(Rotation.RotateVector(Vertex));
	}
		
	return MoveTemp(ModifiedVertices);
}

TArray<FVector> FNVectorUtils::OffsetVectors(const TArray<FVector>& Vectors, const FVector& Offset)
{
	TArray<FVector> ModifiedVertices;
	ModifiedVertices.Reserve(Vectors.Num());

	for (const FVector& Vertex : Vectors)
	{
		ModifiedVertices.Add(Vertex+Offset);
	}
	
	return MoveTemp(ModifiedVertices);
}