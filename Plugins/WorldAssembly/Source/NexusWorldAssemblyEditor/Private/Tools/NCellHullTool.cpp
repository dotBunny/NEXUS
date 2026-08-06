// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellHullTool.h"

#include "InteractiveToolManager.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "BaseGizmos/CombinedTransformGizmo.h"
#include "BaseGizmos/TransformGizmoUtil.h"
#include "BaseGizmos/TransformProxy.h"
#include "Cell/NCellRootComponent.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"


UInteractiveTool* UNCellHullToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UNCellHullTool>(SceneState.ToolManager);
}

void UNCellHullTool::Setup()
{
	UNCellToolBase::Setup();

	USingleClickInputBehavior* ClickBehavior = NewObject<USingleClickInputBehavior>(this);
	ClickBehavior->Initialize(this);
	AddInputBehavior(ClickBehavior);

	GetToolManager()->DisplayMessage(
		LOCTEXT("NCellHullTool_Message", "Click a hull vertex to drag it, or an edge to select it for splitting."),
		EToolMessageLevel::UserNotification);
}

void UNCellHullTool::Shutdown(const EToolShutdownType ShutdownType)
{
	DragTransaction.Reset();
	ClearSelection();

	GetToolManager()->DisplayMessage(FText::GetEmpty(), EToolMessageLevel::UserNotification);

	UNCellToolBase::Shutdown(ShutdownType);
}

void UNCellHullTool::OnTick(float DeltaTime)
{
	// The cached hull is rebuilt every mode tick, and a recompute (Calculate Hull, undo/redo) can shrink it beneath a
	// captured index. Drop the selection rather than letting the gizmo drive an index that no longer exists.
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	if (VertexIndex != INDEX_NONE && !Vertices.IsValidIndex(VertexIndex))
	{
		ClearSelection();
		return;
	}
	if (HasEdgeSelected() && (!Vertices.IsValidIndex(EdgeStartIndex) || !Vertices.IsValidIndex(EdgeEndIndex)))
	{
		ClearSelection();
		return;
	}

	// Follow the vertex when it moves for reasons other than this gizmo (undo, Calculate Hull, the cell actor being
	// dragged). Skipped mid-drag, where the gizmo is the thing driving the vertex.
	if (VertexIndex == INDEX_NONE || VertexGizmo == nullptr || DragTransaction.IsValid()) return;

	const FVector TargetPosition = Vertices[VertexIndex];
	if (VertexGizmo->GetGizmoTransform().GetLocation().Equals(TargetPosition)) return;

	// ReinitializeGizmoTransform, not UTransformProxy::SetTransform: the gizmo caches its transform on its own actor
	// and only re-reads it while being dragged, so moving the proxy alone leaves the handle behind where the vertex
	// used to be. This moves both, without firing the change callbacks that would write straight back into the hull.
	VertexGizmo->ReinitializeGizmoTransform(FTransform(TargetPosition));
}

FInputRayHit UNCellHullTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	double Distance = 0.0;
	if (FindVertexUnderRay(ClickPos.WorldRay, Distance) != INDEX_NONE)
	{
		return FInputRayHit(static_cast<float>(Distance));
	}
	if (FindEdgeUnderRay(ClickPos.WorldRay, Distance) != INDEX_NONE)
	{
		return FInputRayHit(static_cast<float>(Distance));
	}
	return FInputRayHit();
}

void UNCellHullTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	double VertexDistance = 0.0;
	const int32 HitVertex = FindVertexUnderRay(ClickPos.WorldRay, VertexDistance);

	double EdgeDistance = 0.0;
	const int32 HitEdge = FindEdgeUnderRay(ClickPos.WorldRay, EdgeDistance);

	ClearSelection();

	// Vertices win ties: an edge's endpoints sit on the edge, so a click near a corner is ambiguous by construction
	// and the vertex is the more specific target.
	if (HitVertex != INDEX_NONE && (HitEdge == INDEX_NONE || VertexDistance <= EdgeDistance))
	{
		VertexIndex = HitVertex;
		UpdateVertexGizmo();
		return;
	}

	if (HitEdge != INDEX_NONE)
	{
		const TArray<FIntVector2>& Edges = UNWorldAssemblyEdMode::GetCachedHullEdges();
		EdgeStartIndex = Edges[HitEdge].X;
		EdgeEndIndex = Edges[HitEdge].Y;
	}
}

void UNCellHullTool::ClearSelection()
{
	VertexIndex = INDEX_NONE;
	EdgeStartIndex = INDEX_NONE;
	EdgeEndIndex = INDEX_NONE;

	GetToolManager()->GetPairedGizmoManager()->DestroyAllGizmosByOwner(this);
	VertexGizmo = nullptr;
	VertexProxy = nullptr;
}

void UNCellHullTool::UpdateVertexGizmo()
{
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	if (!Vertices.IsValidIndex(VertexIndex)) return;

	VertexProxy = NewObject<UTransformProxy>(this);
	VertexProxy->SetTransform(FTransform(Vertices[VertexIndex]));
	VertexProxy->OnTransformChanged.AddUObject(this, &UNCellHullTool::OnVertexTransformChanged);
	VertexProxy->OnBeginTransformEdit.AddUObject(this, &UNCellHullTool::OnVertexTransformBegin);
	VertexProxy->OnEndTransformEdit.AddUObject(this, &UNCellHullTool::OnVertexTransformEnd);

	VertexGizmo = UE::TransformGizmoUtil::CreateCustomTransformGizmo(
		GetToolManager(), ETransformGizmoSubElements::TranslateAllAxes | ETransformGizmoSubElements::TranslateAllPlanes, this);

	// Null when no UCombinedTransformGizmoContextObject is registered on the tools context; see UNCellBoundsTool.
	if (VertexGizmo == nullptr) return;

	VertexGizmo->SetActiveTarget(VertexProxy, GetToolManager());
}

void UNCellHullTool::OnVertexTransformBegin(UTransformProxy* Proxy)
{
	if (DragTransaction.IsValid()) return;

	DragTransaction = MakeUnique<FScopedTransaction>(LOCTEXT("NCellHullTool_AdjustHullVertex", "Adjust Hull Vertex"));

	if (UNCellRootComponent* RootComponent = GetCellRoot())
	{
		RootComponent->Modify();
	}
}

void UNCellHullTool::OnVertexTransformEnd(UTransformProxy* Proxy)
{
	DragTransaction.Reset();
}

void UNCellHullTool::OnVertexTransformChanged(UTransformProxy* Proxy, FTransform Transform)
{
	UNCellRootComponent* RootComponent = GetCellRoot();
	if (RootComponent == nullptr) return;
	if (!RootComponent->Details.Hull.Vertices.IsValidIndex(VertexIndex)) return;

	RootComponent->Details.HullSettings.bCalculateOnSave = false;
	RootComponent->Details.Hull.bIsChaosGenerated = false;

	// The gizmo works in world space; the hull is stored in the cell root's local space.
	const FTransform OffsetTransform(RootComponent->GetOffsetRotator(), RootComponent->GetOffsetLocation());
	const FVector NewPosition = OffsetTransform.InverseTransformPosition(Transform.GetLocation());
	const FVector PreviousPosition = RootComponent->Details.Hull.Vertices[VertexIndex];

	// Moved through SetVertex rather than by assigning into Vertices: a direct write leaves the convexity flags and
	// the face-plane cache describing the pre-drag hull, so the next penetration query measures against surfaces that
	// have already moved.
	//
	// Neither branch calls Validate() explicitly. SetVertex marks the derived state dirty, so the convexity gate below
	// re-evaluates on read and any later reader of the restored hull does the same. An eager Validate here would only
	// duplicate that: CheckConvex is O(vertices * faces) and runs on every mouse-move frame of a drag.
	RootComponent->Details.Hull.SetVertex(VertexIndex, NewPosition);

	// Ahead of the gate: CheckConvex scales its planarity tolerances by the mesh extent, so it wants Bounds describing
	// the geometry it is about to judge.
	RootComponent->Details.Hull.CalculateCenterAndBounds();

	// If we're not allowing non-convex, move it back. Restoring through SetVertex is what makes the discarded verdict
	// safe: a reverted hull that kept the non-convex result computed for the thrown-away position would latch a convex
	// hull as non-convex until the next CalculateHull or reload, pushing every later query onto the slower path.
	if (!RootComponent->Details.HullSettings.bAllowNonConvex && !RootComponent->Details.Hull.IsConvex())
	{
		RootComponent->Details.Hull.SetVertex(VertexIndex, PreviousPosition);
		RootComponent->Details.Hull.CalculateCenterAndBounds();
	}

	MarkCellDirty();
}

void UNCellHullTool::SplitSelectedEdge()
{
	if (!HasEdgeSelected()) return;

	ANCellActor* Actor = CellActor.Get();
	if (Actor == nullptr) return;

	Actor->SplitHullEdge(EdgeStartIndex, EdgeEndIndex);
	ClearSelection();
}

int32 UNCellHullTool::FindVertexUnderRay(const FRay& Ray, double& OutDistance) const
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

int32 UNCellHullTool::FindEdgeUnderRay(const FRay& Ray, double& OutDistance) const
{
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	const TArray<FIntVector2>& Edges = UNWorldAssemblyEdMode::GetCachedHullEdges();

	int32 BestIndex = INDEX_NONE;
	double BestRayParameter = 0.0;

	for (int32 i = 0; i < Edges.Num(); i++)
	{
		if (!Vertices.IsValidIndex(Edges[i].X) || !Vertices.IsValidIndex(Edges[i].Y)) continue;

		// Sampled along the segment rather than solved analytically: an exact segment-ray closest approach is more
		// arithmetic than a pick needs, and hull edges are short enough that a handful of samples covers them.
		constexpr int32 SampleCount = 8;
		const FVector& Start = Vertices[Edges[i].X];
		const FVector& End = Vertices[Edges[i].Y];

		for (int32 Sample = 0; Sample <= SampleCount; Sample++)
		{
			const FVector Point = FMath::Lerp(Start, End, static_cast<double>(Sample) / SampleCount);

			double RayParameter = 0.0;
			if (!IsPickedByRay(Ray, Point, RayParameter)) continue;
			if (BestIndex != INDEX_NONE && RayParameter >= BestRayParameter) continue;

			BestIndex = i;
			BestRayParameter = RayParameter;
		}
	}

	if (BestIndex != INDEX_NONE)
	{
		OutDistance = BestRayParameter;
	}
	return BestIndex;
}

#undef LOCTEXT_NAMESPACE
