// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NFilterJunctionPoints2DElement.generated.h"


UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNFilterJunctionPoints2DSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

	const FName SourceAttribute = FName("Sources");
	const FName JunctionsAttribute = FName("Junctions");
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Filter Junction Points 2D"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Filters points based on 2D distance to junctions."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::FilterElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	
	virtual FPCGElementPtr CreateElement() const override;
};

class FNFilterJunctionPoints2DElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
	
};