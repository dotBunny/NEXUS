// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "PCG/NTargetPointComponent.h"

#include "Components/BillboardComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Macros/NActorMacros.h"

UNTargetPointComponent::UNTargetPointComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

#if WITH_EDITOR
void UNTargetPointComponent::OnRegister()
{
	// The engine's own target point icon, the one ATargetPoint uses, so a marker reads as the same kind of thing as
	// the built-in it stands in for. Scaled to match the other NEXUS marker icons — these arrive in sets.
	N_WORLD_ICON_ON_REGISTER("/Engine/EditorResources/S_TargetPoint", 0.35f)

	Super::OnRegister();
}

void UNTargetPointComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	N_WORLD_ICON_CLEANUP(bDestroyingHierarchy)
}
#endif // WITH_EDITOR
