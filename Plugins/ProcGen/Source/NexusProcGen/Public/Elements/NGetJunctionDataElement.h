// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NGetJunctionDataElement.generated.h"

USTRUCT(BlueprintType)
struct FNGetJunctionDataParams
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	float SocketDepth = 100.f;
};

UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNGetJunctionDataSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	const FName JunctionsAttribute = FName("Junctions");

	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Get Junction Data"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Parses referenced UNJunctionComponent (ComponentReference) input into expected Junction data."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::GetElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties, PCG_Overridable))
	FNGetJunctionDataParams GetParams;
	
protected:
	virtual FPCGElementPtr CreateElement() const override;
};

class FNGetJunctionDataElement : public IPCGElement
{
public:
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};