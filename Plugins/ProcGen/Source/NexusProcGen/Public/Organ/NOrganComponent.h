// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Types/NPositionRotation.h"
#include "NOrganComponent.generated.h"

struct FNTissueEntry;
class UNCell;
class UNTissue;

/**
 * When an organ should run its generation pass.
 */
UENUM(BlueprintType)
enum class ENOrganGenerationTrigger : uint8
{
	GenerateOnLoad = 0		UMETA(DisplayName="Generate On Load", ToolTip = "Generates only when the component is loaded into the level."),
	GenerateOnDemand = 1	UMETA(DisplayName="Generate On Demand", ToolTip = "Generates only when requested (e.g. via Blueprint)."),
	GenerateAtRuntime = 2	UMETA(DisplayName="Generate At Runtime", ToolTip = "Generates only when scheduled by the Runtime Generation Scheduler.")
};

/**
 * Component attached to an actor (commonly an AVolume) to mark it as a procedural "organ".
 *
 * The organ defines a region of space plus the pool of tissues and cell-count constraints the
 * ProcGen pipeline will use to populate it. The owning actor's transform/bounds drive placement;
 * this component supplies the rules.
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Organ", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input))
class NEXUSPROCGEN_API UNOrganComponent : public UActorComponent
{
	friend class NOrganGenerator;
	friend class FNVirtualOrganContext;
	friend class FNProcGenOperationContext;

	GENERATED_BODY()

public:
	/** When false, the organ is skipped entirely by the ProcGen pipeline. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component", meta = (DisplayPriority = 100))
	bool bActivated = true;

	/** When true, the organ is not clipped to its owning volume — cells may extend past the volume bounds. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component")
	bool bUnbounded = false;

	/** Lower bound on the cell count placed in this organ; -1 means no minimum. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component", meta=(ClampMin=-1))
	int32 MinimumCellCount = -1;

	/** Upper bound on the cell count placed in this organ; -1 means no maximum. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component", meta=(ClampMin=-1))
	int32 MaximumCellCount = -1;

	/** Deterministic seed mixed into the organ's generation decisions. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component", meta = (DisplayPriority = 600))
	int32 Seed = -1;

	/** Controls when the organ is allowed to generate (load-time, on-demand, or runtime-scheduled). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Organ Component", meta = (DisplayPriority = 200))
	ENOrganGenerationTrigger GenerationTrigger = ENOrganGenerationTrigger::GenerateOnLoad;

	/** Tissues (and transitively, cells) the organ may draw from during generation. */
	UPROPERTY(EditAnywhere, Category = "Organ Component")
	TArray<TSoftObjectPtr<UNTissue>> Tissues;

	/** @return true if the owning actor is a volume-derived actor. */
	UFUNCTION(BlueprintCallable)
	bool IsVolumeBased() const { return GetOwner()->IsA<AVolume>(); }

	/** @return The owning actor cast to AVolume, or nullptr if the owner isn't a volume. */
	UFUNCTION(BlueprintCallable)
	AVolume* GetVolume() const { return Cast<AVolume>(GetOwner()); }

	/** @return Human-readable debug label drawn in the viewport overlay. */
	FString GetDebugLabel() const;

	/** @return Position/rotation at which the debug label should be rendered. */
	FNPositionRotation GetDebugLabelPositionRotation() const;

	/**
	 * Filter helper that extracts all UNOrganComponent instances from a heterogeneous weak-object set.
	 * @param Objects Weak references to any UObject set (typically a selection or context).
	 * @return Strong pointers to the organ components contained in Objects.
	 */
	FORCEINLINE static TArray<UNOrganComponent*> GetOrganComponents(TArray<TWeakObjectPtr<UObject>> Objects)
	{
		TArray<UNOrganComponent*> Components;
		for (TWeakObjectPtr WeakObject : Objects)
		{
			UObject* Object = WeakObject.Get();
			if (Object != nullptr && Object->IsA<UNOrganComponent>())
			{
				Components.Add(Cast<UNOrganComponent>(Object));
			}
		}
		return MoveTemp(Components);
	}

	/** Render the organ's debug outline and labels via the provided primitive draw interface. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;

	/** Stash the ticket of the most recent operation that targeted this organ. */
	void SetLastOperationTicket(const uint32& Ticket) { LastOperationTicket = Ticket; }

	/** @return The ticket of the most recent operation that targeted this organ. */
	const uint32& GetLastOperationTicket() const { return LastOperationTicket; }

	/** @return The stored ticket, clearing the slot so subsequent callers see zero. */
	uint32 GetAndResetLastOperationTicket()
	{
		const uint32 ReturnTicket = LastOperationTicket;
		LastOperationTicket = 0;
		return ReturnTicket;
	}

	UPROPERTY(VisibleAnywhere, Category = "Organ Component")
	FGuid Identifier = FGuid::NewGuid();

protected:

	//~UActorComponent
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//End UActorComponent

	/** @return Map of candidate cells to the tissue entry they belong to, flattened from Tissues. */
	TMap<TObjectPtr<UNCell>, FNTissueEntry> GetTissueMap() const;

private:
	/** Ticket of the operation most recently scheduled against this organ; zeroed by GetAndResetLastOperationTicket. */
	uint32 LastOperationTicket;
};