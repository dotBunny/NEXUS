// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Native utility methods for working with FBoxSphereBounds and AVolumes.
 *
 * All bounds helpers treat the box portion of FBoxSphereBounds as authoritative; the sphere is
 * used opportunistically as an early-out to avoid the more expensive box check.
 */
class NEXUSCORE_API FNBoundsUtils
{
public:
	/**
	 * Tests whether the inner bounds are fully enclosed by the outer bounds.
	 * @param InnerBounds The bounds being tested for containment.
	 * @param OuterBounds The candidate enclosing bounds.
	 * @return true when every corner of InnerBounds lies on or within OuterBounds.
	 */
	static FORCEINLINE bool IsBoundsContainedInBounds(const FBoxSphereBounds& InnerBounds, const FBoxSphereBounds& OuterBounds)
	{
		// Check if the inner box is completely contained within the outer box
		const FVector InnerMin = InnerBounds.Origin - InnerBounds.BoxExtent;
		const FVector InnerMax = InnerBounds.Origin + InnerBounds.BoxExtent;
		const FVector OuterMin = OuterBounds.Origin - OuterBounds.BoxExtent;
		const FVector OuterMax = OuterBounds.Origin + OuterBounds.BoxExtent;

		return (InnerMin.X >= OuterMin.X && InnerMin.Y >= OuterMin.Y && InnerMin.Z >= OuterMin.Z &&
				InnerMax.X <= OuterMax.X && InnerMax.Y <= OuterMax.Y && InnerMax.Z <= OuterMax.Z);
	}

	/**
	 * Tests whether a point lies inside the supplied bounds, using the sphere radius as an early-out.
	 * @param Point World-space point to test.
	 * @param Bounds Bounds to test against.
	 * @return true when Point is inside or on the box face of Bounds.
	 */
	static FORCEINLINE bool IsPointInBounds(const FVector& Point, const FBoxSphereBounds& Bounds)
	{
		if (FVector::DistSquared(Point, Bounds.Origin) > FMath::Square(Bounds.SphereRadius))
		{
			return false;
		}
		return Bounds.GetBox().IsInsideOrOn(Point);
	}

	/**
	 * Returns Point if it is inside Bounds, otherwise the closest surface point on the box.
	 * @param Point World-space point to clamp.
	 * @param Bounds Bounds to clamp into.
	 * @return Point itself if contained, otherwise its nearest projection onto Bounds.
	 */
	static FORCEINLINE FVector GetPointInBounds(const FVector& Point, const FBoxSphereBounds& Bounds)
	{
		if (IsPointInBounds(Point, Bounds)) return Point;
		const FBox Box = Bounds.GetBox();
		return Box.GetClosestPointTo(Point);
	}

	/**
	 * Variant of GetPointInBounds that shrinks the bounds by Margin before clamping.
	 * @param Point World-space point to clamp.
	 * @param Bounds Bounds to clamp into.
	 * @param Margin Per-axis inset applied to Bounds before the containment test.
	 * @return Point itself if it is inside the shrunk bounds, otherwise its nearest projection onto them.
	 */
	static FORCEINLINE FVector GetPointInBoundsWithMargin(const FVector& Point, const FBoxSphereBounds& Bounds, const FVector& Margin)
	{
		const FVector NewExtent = Bounds.BoxExtent - Margin;
		const float NewRadius = Bounds.SphereRadius - Margin.GetMax();
		const FBoxSphereBounds ReducedBounds(Bounds.Origin, NewExtent, NewRadius);

		if (IsPointInBounds(Point, ReducedBounds)) return Point;
		const FBox Box = ReducedBounds.GetBox();
		return Box.GetClosestPointTo(Point);
	}

	/**
	 * Tests whether an inner volume is fully contained within an outer volume using precise geometry.
	 * @param InnerVolume The candidate contained volume.
	 * @param OuterVolume The candidate enclosing volume.
	 * @return true when every point of InnerVolume lies inside OuterVolume.
	 */
	static bool IsVolumeContainedInVolume(const AVolume* InnerVolume, const AVolume* OuterVolume);

	/**
	 * Fast (bounds-only) variant of IsVolumeContainedInVolume.
	 * @param InnerVolume The candidate contained volume.
	 * @param OuterVolume The candidate enclosing volume.
	 * @return true when InnerVolume's axis-aligned bounds lie within OuterVolume's.
	 * @note Can return true for geometry that is not actually contained; use IsVolumeContainedInVolume when precision matters.
	 */
	static bool IsVolumeContainedInVolumeFast(const AVolume* InnerVolume, const AVolume* OuterVolume);
};