// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NColor.h"
#include "PCGSettings.h"
#include "NGetAllJunctionDataElement.generated.h"

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
	
	virtual void GetStaticTrackedKeys(FPCGSelectionKeyToSettingsMap& OutKeysToSettings, TArray<TObjectPtr<const UPCGGraph>>& OutVisitedGraphs) const override;
	virtual bool CanDynamicallyTrackKeys() const override { return true; }
#endif
	
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

class FNGetAllJunctionDataElement : public IPCGElement
{
public:
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }
	virtual void GetDependenciesCrc(const FPCGGetDependenciesCrcParams& InParams, FPCGCrc& OutCrc) const override;

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};