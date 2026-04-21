// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Identifies which UActorComponent lifecycle hook a feature should initialize on.
 */
UENUM(BlueprintType)
enum class ENActorComponentLifecycleStart : uint8
{
	/** Defer initialization until BeginPlay; safe for most gameplay-side setup. */
	BeginPlay = 0,
	/** Initialize earlier, during InitializeComponent; required when setup must be in place before other components' BeginPlay runs. */
	InitializeComponent = 1,
};

/**
 * Identifies which UActorComponent lifecycle hook a feature should tear down on.
 */
UENUM(BlueprintType)
enum class ENActorComponentLifecycleEnd : uint8
{
	/** Tear down during EndPlay; mirrors the BeginPlay start path. */
    EndPlay = 0,
	/** Tear down during UninitializeComponent; mirrors the InitializeComponent start path. */
    UninitializeComponent = 1,
};