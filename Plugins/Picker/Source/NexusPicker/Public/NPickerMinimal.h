// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Constants shared across the NexusPicker runtime module.
 */
namespace NEXUS::Picker
{
	/** Distance (cm) within which a point is considered "on" a spline when performing inclusion tests. */
	constexpr float SplinePointTolerance = 0.1f;

	/**
	 * Visual logger / debug-draw colors and sizes used by the various picker implementations.
	 */
	namespace VLog
	{
		/** Color used for the inner boundary of a picker volume (e.g. minimum radius). */
		const FColor InnerColor = FColor::Black;
		/** Color used for the outer boundary of a picker volume (e.g. maximum radius). */
		const FColor OuterColor = FColor::White;
		/** Color used for generated sample points. */
		const FColor PointColor = FColor::Green;
		/** Radius (cm) used when drawing generated sample points. */
		constexpr float PointSize = 2.f;
	}
}

/** Log category used by all NexusPicker logging. */
NEXUSPICKER_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusPicker, Log, All);