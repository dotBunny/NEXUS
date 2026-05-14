// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NFilterEdgePoints2DXYElement.generated.h"

/**
 * Parameters for UNFilterEdgePoints2DXYSettings.
 */
USTRUCT(BlueprintType)
struct FNFilterEdgePoints2DParams
{
	GENERATED_BODY()

	/** Grid spacing (world units) used when testing for neighbours to identify border points. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	float Spacing = 100.f;
};

/**
 * PCG settings node that keeps only the edge/border points of a filled 2D grid projected onto XY.
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNFilterEdgePoints2DXYSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Filter Edge Points 2D (XY)"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Finds and identifies edge (border) points of a filled set of 2D points along the XY axis."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::FilterElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	/** Instance of the per-evaluation parameter struct, exposed as PCG-overridable. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties, PCG_Overridable))
	FNFilterEdgePoints2DParams FilterParams;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNFilterEdgePoints2DXYSettings.
 */
class FNFilterEdgePoints2DXYElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }

};