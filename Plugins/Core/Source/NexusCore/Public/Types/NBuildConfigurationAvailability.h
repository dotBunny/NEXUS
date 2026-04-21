// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Macros/NFlagsMacros.h"

/**
 * Bit flags identifying the Unreal build configurations a feature should be available in.
 *
 * Combine with ENUM_CLASS_FLAGS-style bitwise operators or the N_BUILD_CONFIGURATION_AVAILABILITY_*
 * helper masks below to set a specific availability profile.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENBuildConfigurationAvailability : uint8
{
	None = 0 UMETA(Hidden),
	Debug =  1 << 0,
	Development = 1 << 1,
	Shipping = 1 << 2,
	Test = 1 << 3,
	Editor = 1 << 4
};
ENUM_CLASS_FLAGS(ENBuildConfigurationAvailability)

/** All build configurations, including Shipping. */
#define N_BUILD_CONFIGURATION_AVAILABILITY_ALL \
	static_cast<uint8>(ENBuildConfigurationAvailability::Debug) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Development) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Shipping) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Test) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Editor)

/** Every build configuration except Shipping; use for developer-only features. */
#define N_BUILD_CONFIGURATION_AVAILABILITY_ALL_NOT_SHIPPING \
	static_cast<uint8>(ENBuildConfigurationAvailability::Debug) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Development) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Test) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Editor)

/**
 * Runtime helper that compares an ENBuildConfigurationAvailability mask against the current build.
 */
class FNBuildConfigurationAvailability
{
public:
	/**
	 * Checks whether the current build configuration is included in BuildConfigurationAvailability.
	 * @param BuildConfigurationAvailability Bit mask of permitted configurations.
	 * @return true when the current build matches one of the flagged configurations.
	 */
	FORCEINLINE static bool IsAvailableInBuild(ENBuildConfigurationAvailability BuildConfigurationAvailability)
	{
#if WITH_EDITOR	
		return N_FLAGS_UINT8_HAS_UINT8(BuildConfigurationAvailability, ENBuildConfigurationAvailability::Editor);
#else		
#if UE_BUILD_DEBUG
		return N_FLAGS_UINT8_HAS_UINT8(BuildConfigurationAvailability, ENBuildConfigurationAvailability::Debug);
#elif UE_BUILD_DEVELOPMENT
		return N_FLAGS_UINT8_HAS_UINT8(BuildConfigurationAvailability, ENBuildConfigurationAvailability::Development);
#elif UE_BUILD_SHIPPING		
		return N_FLAGS_UINT8_HAS_UINT8(BuildConfigurationAvailability, ENBuildConfigurationAvailability::Shipping);
#elif UE_BUILD_TEST
		return N_FLAGS_UINT8_HAS_UINT8(BuildConfigurationAvailability, ENBuildConfigurationAvailability::Test);
#else // !(UE_BUILD_DEBUG || UE_BUILD_DEBUG_GAME || UE_BUILD_DEVELOPMENT || UE_BUILD_SHIPPING || UE_BUILD_TEST)
		return false;
#endif // UE_BUILD_DEBUG	
#endif // WITH_EDITOR
	}
};