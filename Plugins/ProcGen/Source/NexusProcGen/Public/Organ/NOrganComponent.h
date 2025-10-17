// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrganComponent.generated.h"

struct FNTissueEntry;
class UNCell;
class UNTissue;

UENUM(BlueprintType)
enum class ENOrganGenerationTrigger : uint8
{
	OGT_GenerateOnLoad    UMETA(DisplayName="Generate On Load", ToolTip = "Generates only when the component is loaded into the level."),
	OGT_GenerateOnDemand  UMETA(DisplayName="Generate On Demand", ToolTip = "Generates only when requested (e.g. via Blueprint)."),
	OGT_GenerateAtRuntime UMETA(DisplayName="Generate At Runtime", ToolTip = "Generates only when scheduled by the Runtime Generation Scheduler.")
};

// TODO: Need to get size from parent volume? or whatever its on?

UCLASS(ClassGroup=(Nexus), DisplayName = "NProcGen Component", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input))
class NEXUSPROCGEN_API UNOrganComponent : public UActorComponent
{
	friend class NOrganGenerator;
	
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (DisplayPriority = 100))
	bool bActivated = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (DisplayPriority = 600))
	int Seed = 42;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Settings, meta = (DisplayPriority = 200))
	ENOrganGenerationTrigger GenerationTrigger = ENOrganGenerationTrigger::OGT_GenerateOnLoad;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UNTissue>> Tissues;

	UFUNCTION(BlueprintCallable)
	bool IsVolumeBased() const { return GetOwner()->IsA<AVolume>(); }

	FString GetDebugLabel() const;
	FVector GetDebugLocation() const;
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI, const FLinearColor Color, float DepthBias = 0.f) const;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

protected:
	TMap<TObjectPtr<UNCell>, FNTissueEntry> GetTissueMap() const;
};