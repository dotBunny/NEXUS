// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "Types/NPositionRotation.h"
#include "NOrganComponent.generated.h"

struct FNTissueTagGroups;
struct FNTissueEntry;
class UNCell;
class UNTissue;

/**
 * When an organ should run its generation pass.
 */
UENUM(BlueprintType)
enum class ENOrganGenerationTrigger : uint8
{
	OnDemand = 0	UMETA(DisplayName="On Demand", ToolTip = "Generates only when requested (e.g. via Blueprint or Subsystem)."),
	BeginPlay = 1	UMETA(DisplayName="Begin Play", ToolTip = "Generates when the component receives it's BeginPlay call.")
};

/**
 * Component attached to an actor (commonly an AVolume) to mark it as a procedural "organ".
 *
 * The organ defines a region of space plus the pool of tissues and cell-count constraints the
 * World Assembly pipeline will use to populate it. The owning actor's transform/bounds drive placement;
 * this component supplies the rules.
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Organ", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input))
class NEXUSWORLDASSEMBLY_API UNOrganComponent : public UActorComponent
{
	friend class NOrganGenerator;
	friend class FNVirtualOrganContext;
	friend class FNAssemblyOperationContext;

	GENERATED_BODY()

public:
	/** When false, the organ is skipped entirely by the World Assembly pipeline. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Inputs")
	bool bActivated = true;
	
	/** When false, the organ can produce no results and still be considered a successful assembly operation. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Inputs")
	bool bRequired = true;

	/** When true, the organ is not clipped to its owning volume — cells may extend past the volume bounds. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Inputs")
	bool bUnbound = false;

	/** Lower bound on the cell count placed in this organ; 0 means no minimum. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Requirements", meta=(ClampMin=0))
	int32 MinimumCellCount = 0;

	/** Upper bound on the cell count placed in this organ; 0 means no maximum. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Requirements", meta=(ClampMin=0))
	int32 MaximumCellCount = 0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Requirements")
	FGameplayTagContainer ContextTags;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Requirements")
	TArray<FNGameplayTagCounterConstraint> TagCounters;

	/** Tissues (and transitively, cells) the organ may draw from during generation. */
	UPROPERTY(EditAnywhere, Category = "Organ Component")
	TArray<TSoftObjectPtr<UNTissue>> Tissues;
	
	/** Controls when the organ is allowed to generate (on-demand, begin-play). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Organ Component|Operation", meta = (DisplayPriority = 200))
	ENOrganGenerationTrigger GenerationTrigger = ENOrganGenerationTrigger::OnDemand;
	
	/** Deterministic seed mixed into the organ's generation decisions. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Organ Component|Operation")
	int32 Seed = -1;

	/** @return true if the owning actor is a volume-derived actor. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|World Assembly")
	bool IsVolumeBased() const { return GetOwner()->IsA<AVolume>(); }

	/** @return The owning actor cast to AVolume, or nullptr if the owner isn't a volume. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|World Assembly")
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
		return Components;
	}

	/** Render the organ's debug outline and labels via the provided primitive draw interface. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;

	/** Stash the ticket of the most recent operation that targeted this organ. */
	void SetLastOperationTicket(const int32& Ticket) { LastOperationTicket = Ticket; }

	/** @return The ticket of the most recent operation that targeted this organ. */
	const int32& GetLastOperationTicket() const { return LastOperationTicket; }

	/** @return The stored ticket, clearing the slot so subsequent callers see zero. */
	int32 GetAndResetLastOperationTicket()
	{
		const int32 ReturnTicket = LastOperationTicket;
		LastOperationTicket = 0;
		return ReturnTicket;
	}

	/** Stable unique identifier for this organ, used to keep generation deterministic across runs. */
	UPROPERTY(VisibleAnywhere, Category = "Organ Component|Operation")
	FGuid Identifier = FGuid::NewGuid();

	//~UActorComponent
	virtual void BeginPlay() override;
	//End UActorComponent
protected:

	//~UActorComponent
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//End UActorComponent

	/** Map of candidate cells to the tissue entry they belong to, flattened from Tissues. */
	void GetTissueMap(TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutMap, FNTissueTagGroups& OutTagGroups) const;

private:
	/** Ticket of the operation most recently scheduled against this organ; zeroed by GetAndResetLastOperationTicket. */
	int32 LastOperationTicket = 0;
};