// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "PCG/NTargetPointComponent.h"
#include "PCGSettings.h"

#include "NGetTargetPointsElement.generated.h"

/**
 * PCG settings node that emits one point per target point component found on the actor running the
 * graph, optionally narrowed to those carrying a given component tag.
 *
 * PCG's own component parsing only understands splines, shapes, primitives and virtual textures, and
 * returns nothing for anything else — so a bare scene component like UNTargetPointComponent produces no
 * data at all through Get Actor Data. This node reads them directly, which keeps markers authorable as
 * components on the PCG actor itself rather than as separate actors scattered through the outliner.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/pcg/types/elements/get-target-points/">UNGetTargetPointsSettings</a>
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNGetTargetPointsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Get Target Points"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Emits one point per target point component on the actor running the graph."); }
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::InputOutput; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	/** Component class to gather. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PCG_Overridable, ToolTip = "Component class to gather. Narrow this to a subclass to keep separate sets of markers on one actor apart from each other."))
	TSubclassOf<UNTargetPointComponent> ComponentClass = UNTargetPointComponent::StaticClass();

	/** When true, gather only components carrying the tag below. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Filter By Tag?", Category = "Settings", meta = (PCG_Overridable, ToolTip = "Should only components carrying the tag below be gathered? Lets one actor hold several sets of markers that different nodes pick up separately."))
	bool bFilterByTag = false;

	/** Component tag a target point must carry to be gathered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (EditCondition = "bFilterByTag", EditConditionHides, PCG_Overridable, ToolTip = "Component tag a target point must carry to be gathered. Matched against the component's own tags, not its actor's. Leaving it empty gathers everything rather than nothing."))
	FName ComponentTag;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNGetTargetPointsSettings.
 */
class FNGetTargetPointsElement : public IPCGElement
{
public:
	/**
	 * Tests whether a component's tags satisfy the tag filter.
	 * Pure and PCG-free so the filter rule can be unit-tested directly.
	 * @param ComponentTags The component's own tags.
	 * @param bFilterByTag Whether filtering is enabled at all.
	 * @param Tag The tag to require.
	 * @return True when the component should be gathered.
	 * @note An enabled filter with no tag set matches everything. Matching nothing would read as the node
	 *       being broken rather than as a filter that happens to exclude every component.
	 */
	static NEXUSCORE_API bool MatchesTagFilter(TConstArrayView<FName> ComponentTags, bool bFilterByTag, FName Tag);

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	/** Reads live actor components, which is not safe away from the game thread. */
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }

	/** The components are live scene state; caching would keep serving their old positions after a move. */
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
};
