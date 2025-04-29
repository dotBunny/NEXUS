// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum ENActorComponentLifecycleStart : uint8
{
	BeginPlay = 0,
	InitializeComponent = 1,
};

UENUM(BlueprintType)
enum ENActorComponentLifecycleEnd : uint8
{
    EndPlay = 0,
    UninitializeComponent = 1,
};