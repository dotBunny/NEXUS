// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum class ENActorComponentLifecycleStart : uint8
{
	ACLS_BeginPlay = 0				UMETA(DisplayName = "Begin Play"), 
	ACLS_InitializeComponent = 1	UMETA(DisplayName = "Initialize Component"),
};

UENUM(BlueprintType)
enum class ENActorComponentLifecycleEnd : uint8
{
    ACLE_EndPlay = 0				UMETA(DisplayName = "End Play"),
    ACLE_UninitializeComponent = 1	UMETA(DisplayName = "Uninitialize Component"),
};