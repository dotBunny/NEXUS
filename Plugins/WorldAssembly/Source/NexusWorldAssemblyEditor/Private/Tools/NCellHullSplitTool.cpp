// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellHullSplitTool.h"

#include "InteractiveToolManager.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "SceneManagement.h"
#include "ToolContextInterfaces.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Cell/NCellRootComponent.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/**
 * Collect the edges of every hull face turned toward a viewer.
 *
 * @param Hull The cell's hull, read for its loops — the world positions come from WorldVertices instead.
 * @param WorldVertices The mode's cached hull vertices, parallel to the hull's own vertex indices.
 * @param ViewOrigin Where the viewer is; for a pick this is the ray's origin.
 * @return Undirected (min,max) keys of the edges on at least one front-facing face, matching the form
 *         FNRawMesh::GetEdgeIndices produces. Empty when the hull has no usable faces.
 * @note Normals are computed here from WorldVertices rather than read from the hull's cached face planes: those
 *       are per-triangle in the hull's own space, and the cell root's offset would have to be undone to compare
 *       them against a world-space viewer. Newell's method, so an n-gon loop works as well as a triangle.
 */
static TSet<FIntVector2> GatherFrontFacingEdgeKeys(const FNRawMesh& Hull, const TArray<FVector>& WorldVertices, const FVector& ViewOrigin)
{
	TSet<FIntVector2> Keys;

	// The hull's own Center is in local space; this is the same quantity in the space the vertices arrive in, and
	// it is what orients the normals below outward without trusting the loops to share a winding.
	FVector Centroid = FVector::ZeroVector;
	for (const FVector& Vertex : WorldVertices)
	{
		Centroid += Vertex;
	}
	if (!WorldVertices.IsEmpty())
	{
		Centroid /= WorldVertices.Num();
	}

	for (const FNRawMeshLoop& Loop : Hull.Loops)
	{
		const int32 Count = Loop.Indices.Num();
		if (Count < 3) continue;

		FVector Normal = FVector::ZeroVector;
		FVector LoopCenter = FVector::ZeroVector;
		bool bIndicesValid = true;

		for (int32 i = 0; i < Count; i++)
		{
			if (!WorldVertices.IsValidIndex(Loop.Indices[i]) || !WorldVertices.IsValidIndex(Loop.Indices[(i + 1) % Count]))
			{
				bIndicesValid = false;
				break;
			}

			const FVector& A = WorldVertices[Loop.Indices[i]];
			const FVector& B = WorldVertices[Loop.Indices[(i + 1) % Count]];

			Normal.X += (A.Y - B.Y) * (A.Z + B.Z);
			Normal.Y += (A.Z - B.Z) * (A.X + B.X);
			Normal.Z += (A.X - B.X) * (A.Y + B.Y);

			LoopCenter += A;
		}

		if (!bIndicesValid) continue;

		Normal = Normal.GetSafeNormal();
		if (Normal.IsNearlyZero()) continue;

		LoopCenter /= Count;

		// Point the normal away from the hull rather than relying on winding order, which SplitEdge's
		// fan-triangulation is under no obligation to keep consistent with the rest of the mesh.
		if (Normal.Dot(LoopCenter - Centroid) < 0.0)
		{
			Normal = -Normal;
		}

		if (Normal.Dot(ViewOrigin - LoopCenter) <= 0.0) continue;

		for (int32 i = 0; i < Count; i++)
		{
			const int32 V0 = Loop.Indices[i];
			const int32 V1 = Loop.Indices[(i + 1) % Count];
			Keys.Add(FIntVector2(FMath::Min(V0, V1), FMath::Max(V0, V1)));
		}
	}

	return Keys;
}

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
	PDI->DrawLine(Vertices[Edge.X], Vertices[Edge.Y], FLinearColor::White, SDPG_Foreground,
		NEXUS::WorldAssembly::EdModeMetrics::HighlightThickness);
	PDI->DrawPoint(HoveredEdge.SplitPosition, FLinearColor::White,
		NEXUS::WorldAssembly::EdModeMetrics::ActiveHandleSize, SDPG_Foreground);
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

	// Ahead of the flag write below, not left to the one inside SplitHullEdge. Modify snapshots the component as it
	// is at the moment it is called, and records that snapshot only once per transaction — so writing the flag first
	// meant every split was captured with it already false, and no amount of undo ever put it back.
	RootComponent->Modify();

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

UNCellHullSplitTool::FNEdgeHit UNCellHullSplitTool::FindEdgeUnderRay(const FRay& Ray) const
{
	const TArray<FVector>& Vertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
	const TArray<FIntVector2>& Edges = UNWorldAssemblyEdMode::GetCachedHullEdges();

	const UNCellRootComponent* RootComponent = GetCellRoot();

	// Empty covers both "no cell" and a hull with no usable faces, and is read below as "cull nothing" — a tool that
	// silently stops picking anything is worse than one that occasionally offers an edge facing away.
	const TSet<FIntVector2> FrontFacingEdges = RootComponent != nullptr
		? GatherFrontFacingEdgeKeys(RootComponent->Details.Hull, Vertices, Ray.Origin)
		: TSet<FIntVector2>();

	FNEdgeHit Best;

	for (int32 i = 0; i < Edges.Num(); i++)
	{
		if (!Vertices.IsValidIndex(Edges[i].X) || !Vertices.IsValidIndex(Edges[i].Y)) continue;

		// GetEdgeIndices already normalizes each entry to (min,max), the same key the gather above builds.
		if (!FrontFacingEdges.IsEmpty() && !FrontFacingEdges.Contains(Edges[i])) continue;

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
