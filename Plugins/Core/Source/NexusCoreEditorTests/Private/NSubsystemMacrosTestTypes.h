// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// NOTE: Deliberately not wrapped in a preprocessor guard. UHT emits these types into the module's
// unscoped registration table, so any #if around a reflected type breaks targets where that define is 0.
// Guard test bodies with WITH_TESTS instead.

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Macros/NSubsystemMacros.h"
#include "Subsystems/WorldSubsystem.h"
#include "NSubsystemMacrosTestTypes.generated.h"

/**
 * The switch both test subsystems read in their ShouldCreate expression.
 *
 * Off by default, so neither subsystem ever exists outside a test that turns it on around the worlds it makes.
 */
struct FNSubsystemMacrosTestSwitch
{
	/** The switch, handed out by pointer so a fixture can gate on it in the shape UNGuardianSubsystem gates on its settings. */
	static bool* Get()
	{
		static bool bAllowCreation = false;
		return &bAllowCreation;
	}
};

/**
 * Test-only world subsystem built on N_WORLD_SUBSYSTEM_GAME_ONLY.
 *
 * Its ShouldCreate is a conditional expression, the form UNGuardianSubsystem passes, which the macro once expanded
 * without parentheses and so read as always-create.
 */
UCLASS()
class NEXUSCOREEDITORTESTS_API UNSubsystemMacrosTestWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	N_WORLD_SUBSYSTEM_GAME_ONLY(UNSubsystemMacrosTestWorldSubsystem,
		(FNSubsystemMacrosTestSwitch::Get() != nullptr) ? *FNSubsystemMacrosTestSwitch::Get() : false)
};

/** Test-only tickable world subsystem built on N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY; it never ticks. */
UCLASS()
class NEXUSCOREEDITORTESTS_API UNSubsystemMacrosTestTickableWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNSubsystemMacrosTestTickableWorldSubsystem, *FNSubsystemMacrosTestSwitch::Get())
	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Never)
};
