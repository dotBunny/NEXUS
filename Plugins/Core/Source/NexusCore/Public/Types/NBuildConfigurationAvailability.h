// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Macros/NFlagsMacros.h"

/*
 * Build configuration flags to specify which builds a feature is available in.
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

#define N_BUILD_CONFIGURATION_AVAILABILITY_ALL \
	static_cast<uint8>(ENBuildConfigurationAvailability::Debug) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Development) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Shipping) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Test) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Editor)

#define N_BUILD_CONFIGURATION_AVAILABILITY_ALL_NOT_SHIPPING \
	static_cast<uint8>(ENBuildConfigurationAvailability::Debug) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Development) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Test) | \
	static_cast<uint8>(ENBuildConfigurationAvailability::Editor)

class FNBuildConfigurationAvailability
{
public:
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
		return N_FLAGS_HAS_UINT8(BuildConfigurationAvailability, ENBuildConfigurationAvailability::Test);
#else // !(UE_BUILD_DEBUG || UE_BUILD_DEBUG_GAME || UE_BUILD_DEVELOPMENT || UE_BUILD_SHIPPING || UE_BUILD_TEST)
		return false;
#endif // UE_BUILD_DEBUG	
#endif // WITH_EDITOR
	}
};