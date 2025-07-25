﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Validators/BlueprintValidator.h"
#include "K2Node_Event.h"
#include "NFixersEditorSettings.h"
#include "Misc/DataValidation.h"

bool UBlueprintValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (UNFixersEditorSettings::Get()->IsAssetIgnored(InAssetData.GetSoftObjectPath()))
	{
		return false;
	}
	return InObject && InObject->IsA<UBlueprint>();
}

EDataValidationResult UBlueprintValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	static const FName EventTickName(TEXT("ReceiveTick"));
	const UNFixersEditorSettings* Settings = UNFixersEditorSettings::Get();

	// Check Type
	UBlueprint* Blueprint = Cast<UBlueprint>(InAsset);
	if (!Blueprint) return EDataValidationResult::NotValidated;

	EDataValidationResult Result = EDataValidationResult::Valid;
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			// Check for Empty Ticks
			if (Settings->bBlueprintEmptyTick)
			{
				if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node); EventNode && EventNode->EventReference.GetMemberName() == EventTickName)
				{
					if (IsEmptyTick(EventNode))
					{
						Context.AddError(FText::FromString(TEXT("Empty Tick nodes still produce overhead, please use or remove it.")));
						Result = EDataValidationResult::Invalid;
					}
				}
			}

			// Check for MultiPin Pure Nodes
			if (Settings->bBlueprintMultiPinPureNode)
			{
				if (UK2Node* PureNode = Cast<UK2Node>(Node); PureNode && PureNode->IsNodePure())
				{
					if (IsMultiPinPureNode(PureNode))
					{
						Context.AddError(FText::Join(
							FText::FromString(" "), PureNode->GetNodeTitle(ENodeTitleType::Type::MenuTitle),
							FText::FromString(TEXT("MultiPin Pure Nodes actually get called for each connected pin output."))));
						Result = EDataValidationResult::Invalid;
					}
				}
			}
		}
	}
	return Result;
}

bool UBlueprintValidator::IsEmptyTick(const UK2Node_Event* EventNode)
{
	if (EventNode->IsAutomaticallyPlacedGhostNode()) return false; // Ghost nodes aren't real nodes

    const UEdGraphPin* ExecThenPin = EventNode->FindPin(UEdGraphSchema_K2::PN_Then);
    return ExecThenPin && ExecThenPin->LinkedTo.IsEmpty();
}

bool UBlueprintValidator::IsMultiPinPureNode(UK2Node* PureNode)
{
	int PinConnectionCount = 0;
	for (UEdGraphPin* Pin : PureNode->Pins)
	{
		//if we're an output pin and we have a connection in the graph - this counts.
		if (Pin->Direction == EGPD_Output && Pin->LinkedTo.Num() > 0)
		{
			PinConnectionCount++;
		}
	}
	
	return PinConnectionCount > 1;
}
