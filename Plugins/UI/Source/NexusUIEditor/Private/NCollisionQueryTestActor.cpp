// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.
#pragma once

#include "NCollisionQueryTestActor.h"
#include "NCollisionQueryTestWidget.h"


ANCollisionQueryTestActor::ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Setting the USceneComponents or AActor to bIsEditorOnlyActor will only cause warnings to throw in the editor when 
	// existing simulation / play mode.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetTickableWhenPaused(true);
		
	// Setup Start
	StartPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Start Point"));
#if WITH_EDITORONLY_DATA
	StartPointComponent->bVisualizeComponent = true;
#endif
	RootComponent = StartPointComponent;
		
	// Setup End
	EndPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("End Point"));
#if WITH_EDITORONLY_DATA
	EndPointComponent->bVisualizeComponent = true;
#endif
	EndPointComponent->SetupAttachment(StartPointComponent);
	EndPointComponent->SetRelativeLocation(FVector(500.f, 0.f, 0.f));
}

void ANCollisionQueryTestActor::Tick(float DeltaSeconds)
{
	if (Widget != nullptr)
	{
		Widget->OnWorldTick();
	}
	Super::Tick(DeltaSeconds);
}
