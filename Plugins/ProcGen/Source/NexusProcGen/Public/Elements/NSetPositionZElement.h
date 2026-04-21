// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NSetPositionZElement.generated.h"

/**
 * Parameters for UNSetPositionZSettings.
 */
USTRUCT(BlueprintType)
struct FNSetPositionZParams
{
	GENERATED_BODY()

	/** Target world Z value that every input point is snapped to. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	float ZValue = 0.f;
};

/**
 * PCG settings node that flattens input points onto a single world-Z plane.
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNSetPositionZSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Set Position Z"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Squashes point data down to the designated Z-plane."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::GetElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	/** Instance of the per-evaluation parameter struct, exposed as PCG-overridable. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties, PCG_Overridable))
	FNSetPositionZParams SetParams;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNSetPositionZSettings.
 */
class FNSetPositionZElement : public IPCGElement
{
public:
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return true; }
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};