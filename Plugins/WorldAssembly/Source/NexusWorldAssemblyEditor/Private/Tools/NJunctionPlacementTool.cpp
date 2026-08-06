// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NJunctionPlacementTool.h"

#include "Editor.h"
#include "InteractiveToolManager.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Cell/NCellJunctionComponent.h"
#include "Engine/World.h"
#include "NWorldAssemblySettings.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

namespace
{
	/** How far along the click ray a junction lands when the click misses world geometry entirely. */
	constexpr double FallbackPlacementDistance = 500.0;

	/** How far the trace reaches before giving up and using the fallback. */
	constexpr double TraceDistance = 1000000.0;
}

UInteractiveTool* UNJunctionPlacementToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UNJunctionPlacementTool>(SceneState.ToolManager);
}

void UNJunctionPlacementTool::Setup()
{
	UNCellToolBase::Setup();

	USingleClickInputBehavior* ClickBehavior = NewObject<USingleClickInputBehavior>(this);
	ClickBehavior->Initialize(this);
	AddInputBehavior(ClickBehavior);

	GetToolManager()->DisplayMessage(
		LOCTEXT("NJunctionPlacementTool_Message", "Click a surface to place a junction on the focused cell, facing out of it."),
		EToolMessageLevel::UserNotification);
}

void UNJunctionPlacementTool::Shutdown(const EToolShutdownType ShutdownType)
{
	GetToolManager()->DisplayMessage(FText::GetEmpty(), EToolMessageLevel::UserNotification);

	UNCellToolBase::Shutdown(ShutdownType);
}

FInputRayHit UNJunctionPlacementTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	// A placement tool accepts a click anywhere in the viewport — missing geometry is a placement in open space, not a
	// miss — so this always reports a hit and lets TracePlacement decide the depth.
	FNJunctionPlacement Placement;
	const bool bTraceHit = TracePlacement(ClickPos.WorldRay, Placement);

	return FInputRayHit(static_cast<float>(bTraceHit
		? FVector::Distance(ClickPos.WorldRay.Origin, Placement.Location)
		: FallbackPlacementDistance));
}

void UNJunctionPlacementTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	FNJunctionPlacement Placement;
	TracePlacement(ClickPos.WorldRay, Placement);

	PlaceJunction(Placement);
}

bool UNJunctionPlacementTool::TracePlacement(const FRay& Ray, FNJunctionPlacement& OutPlacement) const
{
	const ANCellActor* Actor = CellActor.Get();
	UWorld* World = Actor != nullptr ? Actor->GetWorld() : nullptr;

	OutPlacement.Location = Ray.Origin + Ray.Direction * FallbackPlacementDistance;
	OutPlacement.Rotation = MakeUprightRotation(Ray, FVector::ZeroVector);
	OutPlacement.Owner = nullptr;

	if (World == nullptr) return false;

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NJunctionPlacementTool), true);

	if (!World->LineTraceSingleByChannel(Hit, Ray.Origin, Ray.Origin + Ray.Direction * TraceDistance,
		ECC_WorldStatic, QueryParams))
	{
		return false;
	}

	OutPlacement.Location = Hit.ImpactPoint;
	OutPlacement.Rotation = MakeUprightRotation(Ray, Hit.ImpactNormal);
	OutPlacement.Owner = Hit.GetActor();
	return true;
}

FRotator UNJunctionPlacementTool::MakeUprightRotation(const FRay& Ray, const FVector& SurfaceNormal)
{
	// A junction is an opening walked through, not a decal laid on a surface, so it is always built upright: world up
	// is its up axis regardless of what was clicked, and only its facing comes from the click.
	//
	// On a wall the surface normal already points out of the opening, which is the direction a mating junction on the
	// neighboring cell has to meet, so its horizontal component is the facing.
	FVector Forward = FVector::VectorPlaneProject(SurfaceNormal, FVector::UpVector);

	if (!Forward.Normalize())
	{
		// A floor or ceiling normal is vertical and leaves nothing horizontal to face along. Fall back to the
		// direction the click was cast in, flattened — a junction dropped on the ground then faces into the scene,
		// the way the viewport is looking.
		Forward = FVector::VectorPlaneProject(Ray.Direction, FVector::UpVector);

		if (!Forward.Normalize())
		{
			// Looking straight down with no horizontal component at all; nothing in the click implies a facing.
			Forward = FVector::ForwardVector;
		}
	}

	return FRotationMatrix::MakeFromXZ(Forward, FVector::UpVector).Rotator();
}

void UNJunctionPlacementTool::PlaceJunction(const FNJunctionPlacement& Placement) const
{
	// The junction becomes a component of whatever was clicked, so a junction dropped on a wall mesh travels with that
	// mesh rather than with the cell actor. Only a click into empty space falls back to the cell actor, which has no
	// geometry of its own to have been hit.
	AActor* Owner = Placement.Owner.IsValid() ? Placement.Owner.Get() : CellActor.Get();
	if (Owner == nullptr || Owner->GetRootComponent() == nullptr) return;

	const FScopedTransaction Transaction(LOCTEXT("NJunctionPlacementTool_PlaceJunction", "Place Junction"));

	Owner->Modify();

	UNCellJunctionComponent* NewComponent = Cast<UNCellJunctionComponent>(Owner->AddComponentByClass(
		UNCellJunctionComponent::StaticClass(), true, FTransform(Placement.Rotation, Placement.Location), false));
	if (NewComponent == nullptr) return;

	// The socket rectangle is centered on the junction's own location, so placing the component at the clicked point
	// would bury the lower half of the opening in whatever was clicked. Lift it by half the socket height — along
	// world up, which MakeUprightRotation guarantees is the socket's up axis — so the sill sits on the surface.
	const FVector2D UnitSize = UNWorldAssemblySettings::Get()->SocketSize;
	const double SocketHeight = NewComponent->Details.SocketSize.Y * UnitSize.Y;
	NewComponent->SetWorldLocation(Placement.Location + FVector::UpVector * (SocketHeight * 0.5));

	NewComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	Owner->AddInstanceComponent(NewComponent);
	NewComponent->SetFlags(RF_Transactional);
	NewComponent->RegisterComponent();

	// Select the new junction so its details are immediately editable, matching what the add-component command did.
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(NewComponent, true, true, true);
}

#undef LOCTEXT_NAMESPACE
