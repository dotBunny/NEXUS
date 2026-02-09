// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.
#pragma once

#include "CollisionQueryTest/NCollisionQueryTestActor.h"
#include "CollisionQueryTest/NCollisionQueryTestWidget.h"
#include "Components/ActorComponent.h"

ANCollisionQueryTestActor::ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Setting the USceneComponents or AActor to bIsEditorOnlyActor will only cause warnings to throw in the editor when 
	// existing simulation / play mode.

	// Tick everywhere!
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	AActor::SetActorTickEnabled(true);
	SetTickableWhenPaused(true);
	
	// Extra setup for Actor
	bActorSeamlessTraveled = true;
	SetCanBeDamaged(false);
	SetActorEnableCollision(false);
		
	// Setup Start
	StartPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Start Point"));
	StartPointComponent->TransformUpdated.AddUObject(this, &ANCollisionQueryTestActor::OnTransformChanged);
#if WITH_EDITORONLY_DATA
	StartPointComponent->bVisualizeComponent = true;
#endif
	RootComponent = StartPointComponent;
		
	// Setup End
	EndPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("End Point"));
	EndPointComponent->TransformUpdated.AddUObject(this, &ANCollisionQueryTestActor::OnTransformChanged);
#if WITH_EDITORONLY_DATA
	EndPointComponent->bVisualizeComponent = true;
#endif
	EndPointComponent->SetupAttachment(StartPointComponent);
	EndPointComponent->SetRelativeLocation(FVector(500.f, 0.f, 0.f));
}

void ANCollisionQueryTestActor::Tick(float DeltaTime)
{
	if (Widget != nullptr)
	{
		Widget->OnWorldTick(this);
	}
	Super::Tick(DeltaTime);
}

void ANCollisionQueryTestActor::BeginDestroy()
{
	if (Widget != nullptr && Widget->QueryActor == this)
	{
		Widget->QueryActor = nullptr;
	}
	
	// Safety
	if (this->IsRooted())
	{
		RemoveFromRoot();
	}
	
	Super::BeginDestroy();
}

void ANCollisionQueryTestActor::OnTransformChanged(USceneComponent* Component, EUpdateTransformFlags Flags, ETeleportType Teleport)
{
	if (Widget != nullptr)
	{
		Widget->UpdateSettings(this);
	}
}