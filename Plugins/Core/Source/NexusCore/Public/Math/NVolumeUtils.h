// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNVolumeGeometryData
{
	TArray<FDynamicMeshVertex> Vertices;
	TArray<uint32> Indices;
};

class NEXUSCORE_API FNVolumeUtils
{
public:
	static void FillGeometryData(UModel* Model, FNVolumeGeometryData& OutData);
};