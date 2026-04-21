// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"

/**
 * Intersection and containment helpers for FNRawMesh geometry.
 */
class NEXUSCORE_API FNRawMeshUtils
{
public:
	/**
	 * Tests whether two raw meshes intersect when placed at the given origins and rotations.
	 * @param LeftMesh First mesh.
	 * @param LeftOrigin World-space translation applied to LeftMesh.
	 * @param LeftRotation World-space rotation applied to LeftMesh.
	 * @param RightMesh Second mesh.
	 * @param RightOrigin World-space translation applied to RightMesh.
	 * @param RightRotation World-space rotation applied to RightMesh.
	 * @return true when the transformed meshes overlap.
	 */
	static bool DoesIntersect(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);

	/**
	 * Tests whether RelativePoint lies inside Mesh using the mesh's local space.
	 * @param Mesh The mesh to test against.
	 * @param RelativePoint Point expressed in Mesh's local space.
	 * @return true when RelativePoint is inside the mesh volume.
	 */
	static bool IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint);

	/**
	 * Tests whether any of RelativePoints are inside Mesh.
	 * @param Mesh The mesh to test against.
	 * @param RelativePoints Points expressed in Mesh's local space.
	 * @return true when at least one point lies inside the mesh.
	 */
	static bool AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints);

private:

	static bool DoesIntersectTriangles(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
};