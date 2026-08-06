// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellBoundsTool.h"

#include "InteractiveToolManager.h"
#include "BaseGizmos/CombinedTransformGizmo.h"
#include "BaseGizmos/TransformGizmoUtil.h"
#include "BaseGizmos/TransformProxy.h"
#include "Cell/NCellRootComponent.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

UInteractiveTool* UNCellBoundsToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UNCellBoundsTool>(SceneState.ToolManager);
}

void UNCellBoundsTool::Setup()
{
	UNCellToolBase::Setup();

	if (GetCellRoot() == nullptr) return;

	CreateCornerGizmo(ENCorner::Min);
	CreateCornerGizmo(ENCorner::Max);
}

void UNCellBoundsTool::Shutdown(const EToolShutdownType ShutdownType)
{
	// Close any transaction still open because the tool was shut down mid-drag.
	DragTransaction.Reset();

	GetToolManager()->GetPairedGizmoManager()->DestroyAllGizmosByOwner(this);
	MinGizmo = nullptr;
	MaxGizmo = nullptr;
	MinProxy = nullptr;
	MaxProxy = nullptr;

	UNCellToolBase::Shutdown(ShutdownType);
}

void UNCellBoundsTool::OnTick(float DeltaTime)
{
	// The bounds can move without the gizmos touching them — undo/redo, Calculate Bounds, or the cell actor being
	// dragged around the level. Following them here keeps the handles on the corners they claim to drive.
	SyncGizmosToBounds();
}

void UNCellBoundsTool::CreateCornerGizmo(const ENCorner Corner)
{
	UTransformProxy* Proxy = NewObject<UTransformProxy>(this);
	Proxy->SetTransform(FTransform(GetCornerWorldPosition(Corner)));

	Proxy->OnTransformChanged.AddWeakLambda(this, [this, Corner](UTransformProxy* ChangedProxy, FTransform Transform)
	{
		OnCornerTransformChanged(ChangedProxy, Transform, Corner);
	});
	Proxy->OnBeginTransformEdit.AddUObject(this, &UNCellBoundsTool::OnCornerTransformBegin);
	Proxy->OnEndTransformEdit.AddUObject(this, &UNCellBoundsTool::OnCornerTransformEnd);

	// Translate only: the bounds are axis-aligned in the cell's local space, so rotating or scaling a corner has
	// nothing to write back to.
	UCombinedTransformGizmo* Gizmo = UE::TransformGizmoUtil::CreateCustomTransformGizmo(
		GetToolManager(), ETransformGizmoSubElements::TranslateAllAxes | ETransformGizmoSubElements::TranslateAllPlanes, this);

	// Null when no UCombinedTransformGizmoContextObject is registered on the tools context. The mode registers one in
	// Enter, so this should not happen — but the failure is a silent nullptr rather than an assert in shipping, and
	// dereferencing it takes the editor down.
	if (Gizmo == nullptr) return;

	Gizmo->SetActiveTarget(Proxy, GetToolManager());

	if (Corner == ENCorner::Min)
	{
		MinProxy = Proxy;
		MinGizmo = Gizmo;
	}
	else
	{
		MaxProxy = Proxy;
		MaxGizmo = Gizmo;
	}
}

void UNCellBoundsTool::OnCornerTransformBegin(UTransformProxy* Proxy)
{
	if (DragTransaction.IsValid()) return;

	DragTransaction = MakeUnique<FScopedTransaction>(LOCTEXT("NCellBoundsTool_AdjustBounds", "Adjust Cell Bounds"));

	if (UNCellRootComponent* RootComponent = GetCellRoot())
	{
		RootComponent->Modify();
	}
}

void UNCellBoundsTool::OnCornerTransformEnd(UTransformProxy* Proxy)
{
	DragTransaction.Reset();
}

void UNCellBoundsTool::OnCornerTransformChanged(UTransformProxy* Proxy, FTransform Transform, const ENCorner Corner)
{
	UNCellRootComponent* RootComponent = GetCellRoot();
	if (RootComponent == nullptr) return;

	// Back out of the cell root's offset so the world-space gizmo position lands in the local space the bounds are
	// stored in. The visualizer this replaces applied the raw world delta straight to the local box, which only held
	// while the offset rotation was identity.
	const FTransform OffsetTransform(RootComponent->GetOffsetRotator(), RootComponent->GetOffsetLocation());
	const FVector LocalPosition = OffsetTransform.InverseTransformPosition(Transform.GetLocation());

	// A hand-authored box is no longer something the save-time pass should overwrite.
	RootComponent->Details.BoundsSettings.bCalculateOnSave = false;

	if (Corner == ENCorner::Min)
	{
		RootComponent->Details.Bounds.Min = LocalPosition;
	}
	else
	{
		RootComponent->Details.Bounds.Max = LocalPosition;
	}

	MarkCellDirty();
}

void UNCellBoundsTool::SyncGizmosToBounds()
{
	if (GetCellRoot() == nullptr) return;

	// Mid-drag the gizmos are the thing driving the bounds, so writing back what we just read from them would fight
	// the interaction. Everything else that moves the bounds does so between drags.
	if (DragTransaction.IsValid()) return;

	SyncCornerGizmo(MinGizmo, ENCorner::Min);
	SyncCornerGizmo(MaxGizmo, ENCorner::Max);
}

void UNCellBoundsTool::SyncCornerGizmo(UCombinedTransformGizmo* Gizmo, const ENCorner Corner) const
{
	if (Gizmo == nullptr) return;

	const FVector TargetPosition = GetCornerWorldPosition(Corner);
	if (Gizmo->GetGizmoTransform().GetLocation().Equals(TargetPosition)) return;

	// ReinitializeGizmoTransform, not UTransformProxy::SetTransform: the gizmo caches its transform on its own actor
	// and only re-reads it while being dragged, so moving the proxy alone leaves the handles sitting where the bounds
	// used to be — which is what made Calculate Bounds appear to do nothing to them. This moves the gizmo and its
	// proxy together, and does it without firing the change callbacks that would write straight back into the cell.
	Gizmo->ReinitializeGizmoTransform(FTransform(TargetPosition));
}

FVector UNCellBoundsTool::GetCornerWorldPosition(const ENCorner Corner) const
{
	const UNCellRootComponent* RootComponent = GetCellRoot();
	if (RootComponent == nullptr) return FVector::ZeroVector;

	const FTransform OffsetTransform(RootComponent->GetOffsetRotator(), RootComponent->GetOffsetLocation());
	const FVector LocalCorner = Corner == ENCorner::Min
		? RootComponent->Details.Bounds.Min
		: RootComponent->Details.Bounds.Max;

	// Deliberately the transformed corner rather than the matching corner of UNWorldAssemblyEdMode's cached box: that
	// cache is the axis-aligned bound *of* the rotated box, whose corners sit outside the geometry once the cell root
	// carries an offset rotation.
	return OffsetTransform.TransformPosition(LocalCorner);
}

#undef LOCTEXT_NAMESPACE
