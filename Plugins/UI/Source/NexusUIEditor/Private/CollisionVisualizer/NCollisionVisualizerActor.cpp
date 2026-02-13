// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CollisionVisualizer/NCollisionVisualizerActor.h"

#include "CollisionVisualizer/NCollisionVisualizerSceneComponent.h"
#include "CollisionVisualizer/NCollisionVisualizerWidget.h"
#include "Components/ActorComponent.h"

ANCollisionVisualizerActor::ANCollisionVisualizerActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
	StartPointComponent = CreateDefaultSubobject<UNCollisionVisualizerSceneComponent>(TEXT("Start Point"));
	StartPointComponent->TransformUpdated.AddUObject(this, &ANCollisionVisualizerActor::OnTransformChanged);
	RootComponent = StartPointComponent;
		
	// Setup End
	EndPointComponent = CreateDefaultSubobject<UNCollisionVisualizerSceneComponent>(TEXT("End Point"));
	EndPointComponent->bIsEnd = true;
	EndPointComponent->TransformUpdated.AddUObject(this, &ANCollisionVisualizerActor::OnTransformChanged);
	EndPointComponent->SetupAttachment(StartPointComponent);
	EndPointComponent->SetRelativeLocation(FVector(500.f, 0.f, 0.f));
}

void ANCollisionVisualizerActor::Tick(float DeltaTime)
{
	// Handle Draw Mode
	const UWorld* World = GetWorld();
	if (World)
	{
		if (World->IsPlayInEditor() && !bTickInGame)
		{
			return;
		}
		if (World->WorldType == EWorldType::Editor && World->HasBegunPlay() && !bTickInSimulation)
		{
			return;
		}
		if (World->WorldType == EWorldType::Editor && !bTickInEditor)
		{
			return;
		}
	}
	
	if (Widget != nullptr)
	{
		Widget->OnWorldTick(this);
	}
	Super::Tick(DeltaTime);
}

void ANCollisionVisualizerActor::BeginDestroy()
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

void ANCollisionVisualizerActor::SetTickInGame(bool bTick)
{
	bTickInGame = bTick;
}

void ANCollisionVisualizerActor::SetTickInSimulation(bool bTick)
{
	bTickInSimulation = bTick;
}

void ANCollisionVisualizerActor::SetTickInEditor(bool bTick)
{
	bTickInEditor = bTick;
}

void ANCollisionVisualizerActor::OnTransformChanged(USceneComponent* Component, EUpdateTransformFlags Flags, ETeleportType Teleport)
{
	if (Widget != nullptr)
	{
		Widget->UpdateSettings(this);
	}
}
