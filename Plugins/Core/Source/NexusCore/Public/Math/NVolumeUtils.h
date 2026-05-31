// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Geometry payload extracted from a UModel so it can be used for rendering or collision checks
 * without re-walking the BSP tree.
 */
struct FNVolumeGeometryData
{
	/** Triangle-list vertices produced from the source UModel. */
	TArray<FDynamicMeshVertex> Vertices;

	/** Triangle indices referencing Vertices. */
	TArray<uint32> Indices;
};

/**
 * Helpers for working with AVolume and its underlying UModel geometry.
 */
class NEXUSCORE_API FNVolumeUtils
{
public:
	/**
	 * Populates OutData with a triangle-list representation of Model's BSP geometry.
	 * @param Model The UModel (typically sourced from an AVolume's Brush) to tessellate.
	 * @param OutData Output buffer that receives the generated vertices and indices.
	 */
	static void FillGeometryData(UModel* Model, FNVolumeGeometryData& OutData);
};