// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/** Log category used by all NexusCore runtime logging. */
NEXUSCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusCore, Log, All);

/**
 * Compile-time version information for the NEXUS framework.
 * @note Number is a monotonically increasing build number; Major/Minor/Patch follow semantic versioning.
 */
namespace NEXUS::Version
{
	constexpr int Number = 12;
	constexpr int Major = 0;
	constexpr int Minor = 3;
	constexpr int Patch = 0;
}