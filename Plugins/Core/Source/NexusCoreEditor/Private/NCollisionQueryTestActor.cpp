// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCollisionQueryTestActor.h"

ANCollisionQueryTestActor::ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer)
{
	bIsEditorOnlyActor = true;
	
	StartPointComponent= CreateDefaultSubobject<USceneComponent>(TEXT("Start Point"));
	StartPointComponent->SetMobility(EComponentMobility::Movable);
	StartPointComponent->bIsEditorOnly = true;
	RootComponent = StartPointComponent;
	
	EndPointComponent= CreateDefaultSubobject<USceneComponent>(TEXT("End Point"));
	EndPointComponent->SetMobility(EComponentMobility::Movable);
	EndPointComponent->SetupAttachment(RootComponent);
	EndPointComponent->bIsEditorOnly = true;
}

void ANCollisionQueryTestActor::Query()
{
	switch (QueryMethod)
	{
		using enum ECollisionQueryMethod;
	case LineTrace:
		{
			
		}
		break;
	case Sweep:
		{
			
		}
		break;
	case Overlap:
		{
			
		}
		break;
	}
}
