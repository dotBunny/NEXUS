// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NGetAllJunctionDataElement.generated.h"

USTRUCT(BlueprintType)
struct FNGetAllJunctionDataParams
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Setting)
	float SocketDepth = 100.f;
};


UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNGetAllJunctionDataSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	const FName JunctionsAttribute = FName("Junctions");
	
	
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Get All Junction Data"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Gets all registered Junction data from FNProcGenRegistry."); }
	virtual FLinearColor GetNodeTitleColor() const override { return FNColor::GetElement; };
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	
	virtual FPCGElementPtr CreateElement() const override;
	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties, PCG_Overridable))
	FNGetAllJunctionDataParams GetParams;
};

class FNGetAllJunctionDataElement : public IPCGElement
{
public:
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual bool IsPassthrough(const UPCGSettings* InSettings) const override { return true; }
	virtual bool SupportsBasePointDataInputs(FPCGContext* InContext) const override { return true; }

};