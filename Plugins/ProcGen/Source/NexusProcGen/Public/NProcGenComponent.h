// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenComponent.generated.h"

struct FNCellSetEntry;
class UNCell;
class UNCellSet;

UENUM(Blueprintable)
enum class ENProcGenTrigger : uint8
{
	GenerateOnLoad    UMETA(ToolTip = "Generates only when the component is loaded into the level."),
	GenerateOnDemand  UMETA(ToolTip = "Generates only when requested (e.g. via Blueprint)."),
	GenerateAtRuntime UMETA(ToolTip = "Generates only when scheduled by the Runtime Generation Scheduler.")
};


// TODO: Need to get size from parent volume? or whatever its on?


UCLASS(ClassGroup=(Nexus), DisplayName = "NProcGen Component")
class NEXUSPROCGEN_API UNProcGenComponent : public UActorComponent
{
	friend class NOrganGenerator;
	
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (DisplayPriority = 100))
	bool bActivated = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (DisplayPriority = 600))
	int Seed = 42;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Settings, meta = (DisplayPriority = 200))
	ENProcGenTrigger GenerationTrigger = ENProcGenTrigger::GenerateOnLoad;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNCellSet> CellSet;

	UFUNCTION(BlueprintCallable)
	bool IsVolumeBased() const { return GetOwner()->IsA<AVolume>(); }

	FString GetDebugLabel();

protected:
	TMap<TObjectPtr<UNCell>, FNCellSetEntry> GetCellMap() const;
};