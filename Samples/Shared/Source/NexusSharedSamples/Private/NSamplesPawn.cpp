// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesPawn.h"
#include "Components/WidgetInteractionComponent.h"

ANSamplesPawn::ANSamplesPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Interaction"));
	WidgetInteraction->bTickInEditor = true;
	WidgetInteraction->bShowDebug = false;
	WidgetInteraction->InteractionDistance = 1000.0;
	WidgetInteraction->InteractionSource = EWidgetInteractionSource::Mouse; // TODO: We could change this if using Controller?
	WidgetInteraction->SetupAttachment(RootComponent);
}
