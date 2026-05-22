// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum class ENSeverity : uint8
{
	Info		= 0,
	Message		= 1,
	Warning		= 2,
	Error		= 3,
	Fatal		= 4
};