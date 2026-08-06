// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellHullSplitTool.h"

#include "InteractiveToolManager.h"
#include "SceneManagement.h"
#include "ToolContextInterfaces.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Cell/NCellRootComponent.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

UInteractiveTool* UNCellHullSplitToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UNCellHullSplitTool>(SceneState.ToolManager);
}

void UNCellHullSplitTool::Setup()
{
	UNCellToolBase::Setup();

	USingleClickInputBehavior* ClickBehavior = NewObject<USingleClickInputBehavior>(this);
	ClickBehavior->Initialize(this);
	AddInputBehavior(ClickBehavior);

	// Hover is not decoration here. The split lands where the cursor is, and an edge is a line the user is aiming at
	// from an angle — without the marker there is no way to tell where along it a click will actually land.
	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>(this);
	HoverBehavior->Initialize(this);
	AddInputBehavior(HoverBehavior);

	GetToolManager()->DisplayMessage(
		LOCTEXT("NCellHullSplitTool_Message", "Click a hull edge to insert a vertex where you clicked."),
		EToolMessageLevel::UserNotification);
}

void UNCellHullSplitTool::Shutdown(const EToolShutdownType ShutdownType)
{
	HoveredEdge = FNEdgeHit();

	GetToolManager()->DisplayMessage(FText::GetEmpty(), EToolMessageLevel::UserNotification);

	UNCellToolBase::Shutdown(ShutdownType);
}

void UNCellHullSplitTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	UNCellToolBase::Render(RenderAPI);

	FPrimitiveDrawInterface* PDI = RenderAPI != nullptr ? RenderAPI->GetPrimitiveDrawInterface() : nullptr;
	if (PDI == nullptr) return;

	if (!HoveredEdge.IsValid()) return;

	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	const TArray<FIntVector2>& Edges = UNWorldAssemblyEdMode::GetCachedHullEdges();
	if (!Edges.IsValidIndex(HoveredEdge.EdgeIndex)) return;

	const FIntVector2& Edge = Edges[HoveredEdge.EdgeIndex];
	if (!Vertices.IsValidIndex(Edge.X) || !Vertices.IsValidIndex(Edge.Y)) return;

	// White, against the hull color the mode draws the wireframe in: which edge is armed and where the vertex will
	// land are the whole answer this tool owes before a click, and they have to read as separate from the hull
	// whatever the user has set that color to.
	PDI->DrawLine(Vertices[Edge.X], Vertices[Edge.Y], FLinearColor::White, SDPG_Foreground, HoveredEdgeThickness);
	PDI->DrawPoint(HoveredEdge.SplitPosition, FLinearColor::White, SplitPointSize, SDPG_Foreground);
}

FInputRayHit UNCellHullSplitTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	const FNEdgeHit Hit = FindEdgeUnderRay(ClickPos.WorldRay);
	if (!Hit.IsValid()) return FInputRayHit();

	return FInputRayHit(static_cast<float>(Hit.RayParameter));
}

void UNCellHullSplitTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	// Re-picked from the click ray rather than reusing HoveredEdge: a click can arrive without a hover ever having
	// been delivered, and this is the ray the user actually committed with.
	HoveredEdge = FindEdgeUnderRay(ClickPos.WorldRay);

	SplitHoveredEdge();
}

FInputRayHit UNCellHullSplitTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos)
{
	// Always captures, unlike the click test. A hover that only began over an edge could never report the cursor
	// leaving one, so the marker would stick to the last edge hovered until another was found.
	return FInputRayHit(0.0f);
}

void UNCellHullSplitTool::OnBeginHover(const FInputDeviceRay& DevicePos)
{
	OnUpdateHover(DevicePos);
}

bool UNCellHullSplitTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	HoveredEdge = FindEdgeUnderRay(DevicePos.WorldRay);
	return true;
}

void UNCellHullSplitTool::OnEndHover()
{
	HoveredEdge = FNEdgeHit();
}

void UNCellHullSplitTool::SplitHoveredEdge()
{
	if (!HoveredEdge.IsValid()) return;

	ANCellActor* Actor = CellActor.Get();
	UNCellRootComponent* RootComponent = GetCellRoot();
	if (Actor == nullptr || RootComponent == nullptr) return;

	const TArray<FIntVector2>& Edges = UNWorldAssemblyEdMode::GetCachedHullEdges();
	if (!Edges.IsValidIndex(HoveredEdge.EdgeIndex)) return;

	const FIntVector2& Edge = Edges[HoveredEdge.EdgeIndex];

	const FScopedTransaction Transaction(LOCTEXT("NCellHullSplitTool_SplitEdge", "Split Hull Edge"));

	// The cached vertices are world-space; the hull is stored in the cell root's local space.
	const FTransform OffsetTransform(RootComponent->GetOffsetRotator(), RootComponent->GetOffsetLocation());
	const FVector LocalPosition = OffsetTransform.InverseTransformPosition(HoveredEdge.SplitPosition);

	// Matches what a vertex drag in UNCellHullVertexTool does: once the hull has been shaped by hand, the save-time
	// pass recomputing it from level content would throw the edit away. Calculate Hull is how the user opts back in.
	RootComponent->Details.HullSettings.bCalculateOnSave = false;

	Actor->SplitHullEdge(Edge.X, Edge.Y, LocalPosition);

	// The indices this hit was resolved against no longer describe the hull it split.
	HoveredEdge = FNEdgeHit();
}

UNCellHullSplitTool::FNEdgeHit UNCellHullSplitTool::FindEdgeUnderRay(const FRay& Ray)
{
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	const TArray<FIntVector2>& Edges = UNWorldAssemblyEdMode::GetCachedHullEdges();

	FNEdgeHit Best;

	for (int32 i = 0; i < Edges.Num(); i++)
	{
		if (!Vertices.IsValidIndex(Edges[i].X) || !Vertices.IsValidIndex(Edges[i].Y)) continue;

		// Sampled along the segment rather than solved analytically, matching the pick UNCellHullVertexTool used: an
		// exact segment-ray closest approach is more arithmetic than deciding *which* edge needs, and hull edges are
		// short enough that a handful of samples covers them. Where on the edge the split lands is solved below.
		constexpr int32 SampleCount = 8;
		const FVector& Start = Vertices[Edges[i].X];
		const FVector& End = Vertices[Edges[i].Y];

		for (int32 Sample = 0; Sample <= SampleCount; Sample++)
		{
			const FVector Point = FMath::Lerp(Start, End, static_cast<double>(Sample) / SampleCount);

			double RayParameter = 0.0;
			if (!IsPickedByRay(Ray, Point, RayParameter)) continue;
			if (Best.IsValid() && RayParameter >= Best.RayParameter) continue;

			Best.EdgeIndex = i;
			Best.RayParameter = RayParameter;
		}
	}

	if (Best.IsValid())
	{
		Best.SplitPosition = FindClosestPointOnSegment(Ray, Vertices[Edges[Best.EdgeIndex].X], Vertices[Edges[Best.EdgeIndex].Y]);
	}

	return Best;
}

FVector UNCellHullSplitTool::FindClosestPointOnSegment(const FRay& Ray, const FVector& Start, const FVector& End)
{
	const FVector Segment = End - Start;
	const FVector ToStart = Start - Ray.Origin;

	// Closest approach between the segment and the ray, both treated as infinite lines and the result clamped back
	// onto the segment. Ray.Direction is unit length, which is what lets the D·D term drop out of the denominator.
	const double SegmentLengthSquared = Segment.Dot(Segment);
	const double SegmentAlongRay = Segment.Dot(Ray.Direction);
	const double Denominator = SegmentLengthSquared - SegmentAlongRay * SegmentAlongRay;

	// Degenerate or edge-on-to-the-ray: there is no single closest point to prefer, so the midpoint is as good an
	// answer as any and is the one the old Split Hull Edge always gave.
	double Parameter = 0.5;
	if (!FMath::IsNearlyZero(Denominator))
	{
		Parameter = (SegmentAlongRay * Ray.Direction.Dot(ToStart) - Segment.Dot(ToStart)) / Denominator;
	}

	Parameter = FMath::Clamp(Parameter, EndpointClearance, 1.0 - EndpointClearance);

	return Start + Segment * Parameter;
}

#undef LOCTEXT_NAMESPACE
