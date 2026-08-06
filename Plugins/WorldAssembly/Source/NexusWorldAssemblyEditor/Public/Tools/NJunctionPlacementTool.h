// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Tools/NCellToolBase.h"

#include "NJunctionPlacementTool.generated.h"

class UNCellJunctionComponent;

/** Builds UNJunctionPlacementTool for the edit mode's focused cell. */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNJunctionPlacementToolBuilder : public UNCellToolBuilderBase
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Places junctions on the focused cell by clicking the surface they should open through.
 *
 * The command this supersedes attached a junction at the identity transform to whatever actors happened to be
 * selected, leaving the user to position it by hand. Here the click both picks the owner — always the focused cell —
 * and supplies the placement, orienting the junction to face out of the surface it was dropped on.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNJunctionPlacementTool : public UNCellToolBase, public IClickBehaviorTarget
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	//End UInteractiveTool

	//~IClickBehaviorTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;
	//End IClickBehaviorTarget

private:
	/** Everything a click settles about a new junction: where it goes, which way it faces, and what it belongs to. */
	struct FNJunctionPlacement
	{
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;

		/** Actor that was clicked, and which the junction becomes a component of. Null for a click into empty space. */
		TWeakObjectPtr<AActor> Owner;
	};

	/**
	 * Resolves what a click settles about a new junction.
	 *
	 * @param Ray Click ray in world space.
	 * @param OutPlacement Receives the resolved placement.
	 * @return true when the ray hit world geometry; false leaves OutPlacement at a fallback placement in open space,
	 *         with no owning actor.
	 */
	bool TracePlacement(const FRay& Ray, FNJunctionPlacement& OutPlacement) const;

	/**
	 * Builds the placement orientation, keeping the junction upright whatever was clicked.
	 *
	 * @param Ray Click ray in world space, used for the facing when the surface cannot supply one.
	 * @param SurfaceNormal Normal of the surface that was hit, or a zero vector when nothing was.
	 * @return A rotation whose up axis is world up and whose forward is horizontal.
	 */
	static FRotator MakeUprightRotation(const FRay& Ray, const FVector& SurfaceNormal);

	/** Attach a junction to the resolved owner at the resolved transform, and select it. */
	void PlaceJunction(const FNJunctionPlacement& Placement) const;
};
