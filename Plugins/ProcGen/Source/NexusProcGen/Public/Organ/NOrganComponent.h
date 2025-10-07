// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrganComponent.generated.h"

struct FNCellSetEntry;
class UNCell;
class UNCellSet;

UENUM(Blueprintable)
enum class ENOrganGenerationTrigger : uint8
{
	OGT_GenerateOnLoad    UMETA(ToolTip = "Generates only when the component is loaded into the level."),
	OGT_GenerateOnDemand  UMETA(ToolTip = "Generates only when requested (e.g. via Blueprint)."),
	OGT_GenerateAtRuntime UMETA(ToolTip = "Generates only when scheduled by the Runtime Generation Scheduler.")
};


// TODO: Need to get size from parent volume? or whatever its on?


UCLASS(ClassGroup=(Nexus), DisplayName = "NProcGen Component")
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
	TObjectPtr<UNCellSet> CellSet;

	UFUNCTION(BlueprintCallable)
	bool IsVolumeBased() const { return GetOwner()->IsA<AVolume>(); }

	FString GetDebugLabel() const;

protected:
	TMap<TObjectPtr<UNCell>, FNCellSetEntry> GetCellMap() const;
};