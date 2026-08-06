// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellHullVertexTool.h"

#include "InteractiveToolManager.h"
#include "NWorldAssemblyEditorColors.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "SceneManagement.h"
#include "ToolContextInterfaces.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "BaseGizmos/CombinedTransformGizmo.h"
#include "BaseGizmos/TransformGizmoUtil.h"
#include "BaseGizmos/TransformProxy.h"
#include "Cell/NCellRootComponent.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"


UInteractiveTool* UNCellHullVertexToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UNCellHullVertexTool>(SceneState.ToolManager);
}

void UNCellHullVertexTool::Setup()
{
	UNCellToolBase::Setup();

	USingleClickInputBehavior* ClickBehavior = NewObject<USingleClickInputBehavior>(this);
	ClickBehavior->Initialize(this);

	// Both, treated the same way. Shift-extends and ctrl-toggles are separate gestures elsewhere in the editor, but
	// a hull vertex is either in the selection or out of it — there is no range for shift to extend over — so the
	// distinction would only be a rule to remember.
	ClickBehavior->Modifiers.RegisterModifier(ShiftModifierID, FInputDeviceState::IsShiftKeyDown);
	ClickBehavior->Modifiers.RegisterModifier(CtrlModifierID, FInputDeviceState::IsCtrlKeyDown);

	AddInputBehavior(ClickBehavior);

	GetToolManager()->DisplayMessage(
		LOCTEXT("NCellHullVertexTool_Message", "Click a hull vertex to drag it. Shift- or ctrl-click to add and remove vertices from the selection."),
		EToolMessageLevel::UserNotification);
}

void UNCellHullVertexTool::Shutdown(const EToolShutdownType ShutdownType)
{
	DragTransaction.Reset();
	ClearSelection();

	GetToolManager()->DisplayMessage(FText::GetEmpty(), EToolMessageLevel::UserNotification);

	UNCellToolBase::Shutdown(ShutdownType);
}

void UNCellHullVertexTool::OnTick(float DeltaTime)
{
	if (VertexIndices.IsEmpty()) return;

	// The cached hull is rebuilt every mode tick, and a recompute (Calculate Hull, undo/redo) can shrink it beneath a
	// captured index. Drop the whole selection rather than salvaging the indices that still resolve: after a rebuild
	// they no longer name the vertices they were picked as, so keeping them would drag arbitrary geometry.
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	for (const int32 Index : VertexIndices)
	{
		if (!Vertices.IsValidIndex(Index))
		{
			ClearSelection();
			return;
		}
	}

	// Follow the vertices when they move for reasons other than this gizmo (undo, Calculate Hull, the cell actor being
	// dragged). Skipped mid-drag, where the gizmo is the thing driving them.
	if (VertexGizmo == nullptr || DragTransaction.IsValid()) return;

	const FVector TargetPosition = GetSelectionCenter();
	if (VertexGizmo->GetGizmoTransform().GetLocation().Equals(TargetPosition)) return;

	// ReinitializeGizmoTransform, not UTransformProxy::SetTransform: the gizmo caches its transform on its own actor
	// and only re-reads it while being dragged, so moving the proxy alone leaves the handle behind where the vertex
	// used to be. This moves both, without firing the change callbacks that would write straight back into the hull.
	VertexGizmo->ReinitializeGizmoTransform(FTransform(TargetPosition));
}

void UNCellHullVertexTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	UNCellToolBase::Render(RenderAPI);

	FPrimitiveDrawInterface* PDI = RenderAPI != nullptr ? RenderAPI->GetPrimitiveDrawInterface() : nullptr;
	if (PDI == nullptr) return;

	// The hull's own color, so a handle reads as part of the wireframe it belongs to rather than as separate furniture.
	const FLinearColor HullColor = FNWorldAssemblyEditorColors::GetCellHull();
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();

	// SDPG_Foreground, unlike the edges: FindVertexUnderRay is a pure ray-to-point distance test with no occlusion
	// check, so a vertex behind geometry is still clickable. Drawing it in the world layer would hide exactly the
	// handles that still respond, leaving the tool picking things the user cannot see.
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		// White and larger for the selection, matching what the split tool marks its subject with. Contains rather
		// than a parallel flag array: a hull that is large enough for the linear scan to matter is one the pick above
		// is already walking in full every click.
		const bool bIsSelected = VertexIndices.Contains(i);

		PDI->DrawPoint(Vertices[i],
			bIsSelected ? FLinearColor::White : HullColor,
			bIsSelected ? NEXUS::WorldAssembly::EdModeMetrics::ActiveHandleSize : NEXUS::WorldAssembly::EdModeMetrics::HandleSize,
			SDPG_Foreground);
	}
}

FInputRayHit UNCellHullVertexTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	double Distance = 0.0;
	if (FindVertexUnderRay(ClickPos.WorldRay, Distance) != INDEX_NONE)
	{
		return FInputRayHit(static_cast<float>(Distance));
	}
	return FInputRayHit();
}

void UNCellHullVertexTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	double VertexDistance = 0.0;
	const int32 HitVertex = FindVertexUnderRay(ClickPos.WorldRay, VertexDistance);

	const bool bExtendSelection = bShiftDown || bCtrlDown;

	if (!bExtendSelection)
	{
		ClearSelection();

		if (HitVertex != INDEX_NONE)
		{
			VertexIndices.Add(HitVertex);
		}

		UpdateVertexGizmo();
		return;
	}

	// A miss while extending leaves the selection alone. Clearing on it would mean one stray click part-way through
	// picking out a dozen vertices costs the lot, and the modifier is exactly the signal that the user is mid-build.
	if (HitVertex == INDEX_NONE) return;

	if (VertexIndices.Remove(HitVertex) == 0)
	{
		VertexIndices.Add(HitVertex);
	}

	UpdateVertexGizmo();
}

void UNCellHullVertexTool::OnUpdateModifierState(const int ModifierID, const bool bIsOn)
{
	if (ModifierID == ShiftModifierID)
	{
		bShiftDown = bIsOn;
	}
	else if (ModifierID == CtrlModifierID)
	{
		bCtrlDown = bIsOn;
	}
}

void UNCellHullVertexTool::ClearSelection()
{
	VertexIndices.Reset();
	DragStartPositions.Reset();

	GetToolManager()->GetPairedGizmoManager()->DestroyAllGizmosByOwner(this);
	VertexGizmo = nullptr;
	VertexProxy = nullptr;
}

void UNCellHullVertexTool::UpdateVertexGizmo()
{
	// Rebuilt rather than moved. The gizmo is only ever repositioned here in response to the selection changing, and
	// a UTransformProxy's target is fixed once the gizmo has it — so there is nothing to preserve across the change.
	GetToolManager()->GetPairedGizmoManager()->DestroyAllGizmosByOwner(this);
	VertexGizmo = nullptr;
	VertexProxy = nullptr;

	if (VertexIndices.IsEmpty()) return;

	VertexProxy = NewObject<UTransformProxy>(this);
	VertexProxy->SetTransform(FTransform(GetSelectionCenter()));
	VertexProxy->OnTransformChanged.AddUObject(this, &UNCellHullVertexTool::OnVertexTransformChanged);
	VertexProxy->OnBeginTransformEdit.AddUObject(this, &UNCellHullVertexTool::OnVertexTransformBegin);
	VertexProxy->OnEndTransformEdit.AddUObject(this, &UNCellHullVertexTool::OnVertexTransformEnd);

	// Translate only, even with several vertices selected: rotate and scale would need a pivot and a frame the hull
	// does not define, and moving a group is what a multi-vertex selection is for.
	VertexGizmo = UE::TransformGizmoUtil::CreateCustomTransformGizmo(
		GetToolManager(), ETransformGizmoSubElements::TranslateAllAxes | ETransformGizmoSubElements::TranslateAllPlanes, this);

	// Null when no UCombinedTransformGizmoContextObject is registered on the tools context; see UNCellBoundsTool.
	if (VertexGizmo == nullptr) return;

	VertexGizmo->SetActiveTarget(VertexProxy, GetToolManager());
}

FVector UNCellHullVertexTool::GetSelectionCenter() const
{
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();

	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;

	for (const int32 Index : VertexIndices)
	{
		if (!Vertices.IsValidIndex(Index)) continue;

		Sum += Vertices[Index];
		Count++;
	}

	return Count > 0 ? Sum / Count : FVector::ZeroVector;
}

void UNCellHullVertexTool::OnVertexTransformBegin(UTransformProxy* Proxy)
{
	if (DragTransaction.IsValid()) return;

	UNCellRootComponent* RootComponent = GetCellRoot();
	if (RootComponent == nullptr) return;

	// Snapshot before anything moves. Every frame of the drag is applied to these, not to the previous frame's
	// result, so the accumulated translation stays exactly the gizmo's — and the revert below has somewhere to
	// restore from that predates the whole drag rather than just its last frame.
	DragStartPositions.Reset(VertexIndices.Num());
	for (const int32 Index : VertexIndices)
	{
		DragStartPositions.Add(RootComponent->Details.Hull.Vertices.IsValidIndex(Index)
			? RootComponent->Details.Hull.Vertices[Index]
			: FVector::ZeroVector);
	}

	DragStartCenter = VertexGizmo != nullptr ? VertexGizmo->GetGizmoTransform().GetLocation() : GetSelectionCenter();

	DragTransaction = MakeUnique<FScopedTransaction>(VertexIndices.Num() > 1
		? LOCTEXT("NCellHullVertexTool_AdjustHullVertices", "Adjust Hull Vertices")
		: LOCTEXT("NCellHullVertexTool_AdjustHullVertex", "Adjust Hull Vertex"));

	RootComponent->Modify();
}

void UNCellHullVertexTool::OnVertexTransformEnd(UTransformProxy* Proxy)
{
	DragTransaction.Reset();
	DragStartPositions.Reset();
}

void UNCellHullVertexTool::OnVertexTransformChanged(UTransformProxy* Proxy, FTransform Transform)
{
	UNCellRootComponent* RootComponent = GetCellRoot();
	if (RootComponent == nullptr) return;

	// No snapshot means no drag is in flight — the transform was set programmatically rather than dragged, and
	// applying it would move the selection by whatever the gizmo happened to be repositioned to.
	if (DragStartPositions.Num() != VertexIndices.Num() || DragStartPositions.IsEmpty()) return;

	RootComponent->Details.HullSettings.bCalculateOnSave = false;
	RootComponent->Details.Hull.bIsChaosGenerated = false;

	// The gizmo works in world space; the hull is stored in the cell root's local space. Only the translation is
	// needed, so the offset is inverse-transformed as a vector rather than as two positions.
	const FTransform OffsetTransform(RootComponent->GetOffsetRotator(), RootComponent->GetOffsetLocation());
	const FVector LocalDelta = OffsetTransform.InverseTransformVector(Transform.GetLocation() - DragStartCenter);

	// Moved through SetVertex rather than by assigning into Vertices: a direct write leaves the convexity flags and
	// the face-plane cache describing the pre-drag hull, so the next penetration query measures against surfaces that
	// have already moved.
	//
	// Neither branch calls Validate() explicitly. SetVertex marks the derived state dirty, so the convexity gate below
	// re-evaluates on read and any later reader of the restored hull does the same. An eager Validate here would only
	// duplicate that: CheckConvex is O(vertices * faces) and runs on every mouse-move frame of a drag.
	for (int32 i = 0; i < VertexIndices.Num(); i++)
	{
		RootComponent->Details.Hull.SetVertex(VertexIndices[i], DragStartPositions[i] + LocalDelta);
	}

	// Ahead of the gate: CheckConvex scales its planarity tolerances by the mesh extent, so it wants Bounds describing
	// the geometry it is about to judge.
	RootComponent->Details.Hull.CalculateCenterAndBounds();

	// If we're not allowing non-convex, move it back — all of it, since the move the gate judged was all of it.
	// Restoring through SetVertex is what makes the discarded verdict safe: a reverted hull that kept the non-convex
	// result computed for the thrown-away positions would latch a convex hull as non-convex until the next
	// CalculateHull or reload, pushing every later query onto the slower path.
	if (!RootComponent->Details.HullSettings.bAllowNonConvex && !RootComponent->Details.Hull.IsConvex())
	{
		for (int32 i = 0; i < VertexIndices.Num(); i++)
		{
			RootComponent->Details.Hull.SetVertex(VertexIndices[i], DragStartPositions[i]);
		}
		RootComponent->Details.Hull.CalculateCenterAndBounds();
	}

	MarkCellDirty();
}

int32 UNCellHullVertexTool::FindVertexUnderRay(const FRay& Ray, double& OutDistance) const
{
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();

	int32 BestIndex = INDEX_NONE;
	double BestRayParameter = 0.0;

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		double RayParameter = 0.0;
		if (!IsPickedByRay(Ray, Vertices[i], RayParameter)) continue;
		if (BestIndex != INDEX_NONE && RayParameter >= BestRayParameter) continue;

		BestIndex = i;
		BestRayParameter = RayParameter;
	}

	if (BestIndex != INDEX_NONE)
	{
		OutDistance = BestRayParameter;
	}
	return BestIndex;
}

#undef LOCTEXT_NAMESPACE
