// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

namespace NEXUS::Picker
{
	constexpr float Tolerance = 0.01f;
	namespace VLog
	{
		const FColor InnerColor = FColor::Black;
		const FColor OuterColor = FColor::White;
		const FColor PointColor = FColor::Green;
		constexpr float PointSize = 2.f;
	}
}

NEXUSPICKER_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusPicker, Log, All);