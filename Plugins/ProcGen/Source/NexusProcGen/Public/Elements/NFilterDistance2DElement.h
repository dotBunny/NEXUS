// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NFilterDistance2DElement.generated.h"

USTRUCT(BlueprintType)
struct FNFilterDistance2DParams
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	float Spacing = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	float Distance = 100.f;
};

UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNFilterDistance2DSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

	const FName SourceAttribute = FName("Source");
	const FName TargetAttribute = FName("Target");
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Filter Distance 2D"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Filters points based on a two-dimensional distance comparison."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::FilterElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	
	virtual FPCGElementPtr CreateElement() const override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties, PCG_Overridable))
	FNFilterDistance2DParams FilterParams;
};

class FNFilterDistance2DElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
	
};