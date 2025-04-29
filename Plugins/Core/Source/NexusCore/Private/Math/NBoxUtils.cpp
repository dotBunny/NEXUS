// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NBoxUtils.h"

TArray<FVector> FNBoxUtils::GetVertices(const FBox& Box)
{
	TArray<FVector> Vertices;
	const FVector Min = Box.Min;
	const FVector Max = Box.Max;

	Vertices.Add(FVector(Min.X, Min.Y, Min.Z));
	Vertices.Add(FVector(Min.X, Min.Y, Max.Z));
	Vertices.Add(FVector(Max.X, Min.Y, Max.Z));
	Vertices.Add(FVector(Max.X, Min.Y, Min.Z));
	
	Vertices.Add(FVector(Min.X, Max.Y, Min.Z));
	Vertices.Add(FVector(Min.X, Max.Y, Max.Z));
	Vertices.Add(FVector(Max.X, Max.Y, Max.Z));
	Vertices.Add(FVector(Max.X, Max.Y, Min.Z));

	return MoveTemp(Vertices);
}
