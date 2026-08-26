// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NTargetPointComponentVisualizer.h"

#include "CanvasTypes.h"
#include "NColor.h"
#include "PrimitiveDrawingUtils.h"
#include "SceneManagement.h"
#include "SceneView.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Engine.h"
#include "PCG/NTargetPointComponent.h"

/**
 * Tuning for how a target point marker is drawn in a level viewport.
 */
namespace NEXUS::CoreEditor::TargetPoint
{
	/** Screen-space size of the handle marking the point itself, matching the engine's own visualizer handles. */
	inline constexpr float PointHandleSize = 15.f;

	/** How far from the camera tag labels stop being drawn, in world units. */
	inline constexpr double MaximumTagLabelDistance = 2500.0;

	/** Pixel offset from the projected marker to its tag label, clearing the handle drawn at the point. */
	inline constexpr double TagLabelOffsetX = 14.0;
	inline constexpr double TagLabelOffsetY = 6.0;
}

void FNTargetPointComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	const UNTargetPointComponent* TargetPointComponent = Cast<const UNTargetPointComponent>(Component);
	if (TargetPointComponent == nullptr)
	{
		return;
	}

	const FTransform& ComponentTransform = TargetPointComponent->GetComponentTransform();
	const FVector Location = ComponentTransform.GetLocation();


	// Everything the marker draws sits behind one hit proxy, so clicking any part of it selects the component
	// rather than the actor holding it — which is what puts the transform gizmo on the marker being moved. The
	// component's sprite is a click target too, but only an indirect one: it is a child component, so selecting it
	// resolves up the attachment chain rather than landing on the marker outright. Foreground priority to match
	// the depth priority below, so a click lands on a marker drawn over the geometry it sits inside rather than on
	// the geometry itself, and on the marker rather than on its own sprite.
	PDI->SetHitProxy(new HComponentVisProxy(Component, HPP_Foreground));

	// The point itself is drawn as a screen-space handle rather than as world-space geometry, so it stays both
	// visible and clickable at any zoom — a wire shape sized in world units becomes a speck exactly when a set
	// of markers is being surveyed from a distance, which is when the click target is most needed.
	PDI->DrawPoint(Location, FNColor::GetElement, NEXUS::CoreEditor::TargetPoint::PointHandleSize, SDPG_Foreground);

	PDI->SetHitProxy(nullptr);
}

void FNTargetPointComponentVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport,
	const FSceneView* View, FCanvas* Canvas)
{
	const UNTargetPointComponent* TargetPointComponent = Cast<const UNTargetPointComponent>(Component);
	if (TargetPointComponent == nullptr || TargetPointComponent->ComponentTags.IsEmpty())
	{
		return;
	}

	const FVector Location = TargetPointComponent->GetComponentLocation();

	// One actor can carry a great many markers, and every one of them labeled at once is a wall of text rather
	// than information. Distance is what separates the markers being worked on from the rest of the set. The
	// cull is skipped in an orthographic view, where the view origin sits far outside the scene and would put
	// every label past the limit.
	if (View->ViewMatrices.IsPerspectiveProjection()
		&& FVector::DistSquared(Location, View->ViewMatrices.GetViewOrigin())
			> FMath::Square(NEXUS::CoreEditor::TargetPoint::MaximumTagLabelDistance))
	{
		return;
	}

	FVector2D PixelLocation;
	if (!View->ScreenToPixel(View->WorldToScreen(Location), PixelLocation))
	{
		// Behind the camera.
		return;
	}

	// The projection is in render pixels; the canvas draws in unscaled ones.
	PixelLocation /= Canvas->GetDPIScale();

	// Joined onto one line rather than stacked: a marker carries one or two tags in practice, and a column per
	// marker costs more vertical room than the labels are worth once several markers are on screen.
	const FString TagLabel = FString::JoinBy(TargetPointComponent->ComponentTags, TEXT(", "),
		[](const FName& Tag) { return Tag.ToString(); });

	Canvas->DrawShadowedString(
		PixelLocation.X + NEXUS::CoreEditor::TargetPoint::TagLabelOffsetX,
		PixelLocation.Y + NEXUS::CoreEditor::TargetPoint::TagLabelOffsetY,
		TagLabel, GEngine->GetSmallFont(), FNColor::GetElement);
}

bool FNTargetPointComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
	HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	// Accepting the click is what selects the component: FComponentVisualizerManager then selects the proxy's
	// element directly. That directness is the point, because letting the click fall through to ProcessClick's
	// own element handling does not work here —
	// FComponentElementLevelEditorSelectionCustomization::GetSelectionElementStatic gates component selection on
	// FKismetEditorUtilities::CanCreateBlueprintOfClass(OwningActor), which is false for an actor that is itself
	// a Blueprint: its class carries a ClassGeneratedBy, and the AllowDerivedBlueprints ini opt-out defaults off.
	// Markers live on Blueprint actors, so that path resolves every click up to the actor, which is already
	// selected, and nothing appears to happen.
	//
	// The cost of accepting is that this becomes the *active* component visualizer, which would block alt-drag
	// duplication. TrackingStarted below hands that status straight back.
	return VisProxy != nullptr && VisProxy->Component.IsValid() && VisProxy->Component->IsA<UNTargetPointComponent>();
}

bool FNTargetPointComponentVisualizer::ShouldAutoSelectElementOnHandleClick() const
{
	// The base defers to Editor.ComponentVisualizer.AutoSelectComponent, which exists for visualizers whose click
	// picks a handle within a component rather than the component itself. Selecting the component is the entire
	// reason this one accepts a click, so it does not follow that switch.
	return true;
}

void FNTargetPointComponentVisualizer::TrackingStarted(FEditorViewportClient* InViewportClient)
{
	// Give up "active visualizer" status as a drag begins, which is what lets the engine duplicate on alt-drag.
	// Both duplicate paths refuse to run while any visualizer is active — UEditorTRSGizmo::AllowsDragDuplicate
	// and FLevelEditorViewportClient::InputKey both test ComponentVisManager::IsActive() — on the assumption
	// that an active visualizer wants alt-drag for itself, the way the spline one takes it to add points. This
	// one has no drag behavior of its own: a marker is moved by the ordinary component gizmo, and none of the
	// input the manager routes to an active visualizer is implemented here. The status is only ever a side
	// effect of accepting the click, so releasing it here costs nothing.
	//
	// The timing is exact rather than lucky. UEditorTRSGizmo::BeginTransformEditSequence calls
	// StateTarget->BeginUpdate(), which reaches FLevelEditorViewportClient::BeginTransform and this callback,
	// and only then tests AllowsDragDuplicate — a few lines further down the same function.
	GUnrealEd->ComponentVisManager.ClearActiveComponentVis();
}
