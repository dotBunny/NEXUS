// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWidgetTabIdentifiers.generated.h"

/**
 * Parallel-arrays map of widget identifier to host tab identifier. Stored in the NexusUserSettings
 * config so editor utility widgets can be restored into the same tab across sessions; the two
 * arrays are kept index-aligned by UNEditorUtilityWidgetSubsystem.
 */
USTRUCT(BlueprintType)
struct FNWidgetTabIdentifiers
{
	GENERATED_BODY()

	/** Widget identifiers (UNEditorUtilityWidget::UniqueIdentifier); parallel to TabIdentifier. */
	UPROPERTY()
	TArray<FName> WidgetIdentifiers;

	/** Tab identifiers last used to host each widget; parallel to WidgetIdentifiers. */
	UPROPERTY()
	TArray<FName> TabIdentifier;
};