// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorValidatorBase.h"
#include "NBlueprintValidator.generated.h"

class UK2Node;
class UK2Node_Event;

/**
 * Validator that scans UBlueprint assets for common bad-practice patterns — empty Event Tick
 * overrides and pure K2 nodes with multiple connected output pins. Each pattern's severity is
 * controlled independently via UNToolingEditorSettings.
 */
UCLASS()
class UNBlueprintValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

	//~UEditorValidatorBase
	virtual bool CanValidateAsset_Implementation(
		const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;

	virtual EDataValidationResult ValidateLoadedAsset_Implementation(
		const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	//End UEditorValidatorBase

	/** @return true if EventNode is an Event Tick override whose output exec pin has no connections. */
	static bool IsEmptyTick(const UK2Node_Event* EventNode);

	/** @return true if PureNode is a pure node with its output used by more than one consumer. */
	static bool IsMultiPinPureNode(UK2Node* PureNode);
};