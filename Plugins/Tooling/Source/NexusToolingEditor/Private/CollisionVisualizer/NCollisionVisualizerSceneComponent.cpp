// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CollisionVisualizer/NCollisionVisualizerSceneComponent.h"

#include "Components/BillboardComponent.h"
#include "UObject/ICookInfo.h"


void UNCollisionVisualizerSceneComponent::OnRegister()
{
	Super::OnRegister();

#if WITH_EDITOR
	CreateCustomSpriteComponent();
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void UNCollisionVisualizerSceneComponent::CreateCustomSpriteComponent()
{
	UTexture2D* EditorSpriteTexture = nullptr;
	{
		FCookLoadScope EditorOnlyScope(ECookLoadType::EditorOnly);
		if (bIsEnd)
		{
			EditorSpriteTexture = LoadObject<UTexture2D>(nullptr,EndSpritePath);
		}
		else
		{
			EditorSpriteTexture = LoadObject<UTexture2D>(nullptr,StartSpritePath);
		}
	
	}

	if (EditorSpriteTexture)
	{
		bVisualizeComponent = true;
		if (SpriteComponent)
		{
			SpriteComponent->SetSprite(EditorSpriteTexture);
		}
		else
		{
			CreateSpriteComponent(EditorSpriteTexture);
		}
		
	}

	if (SpriteComponent)
	{
		SpriteComponent->SpriteInfo.Category = TEXT("NCollisionVisualizer");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("NexusUIEditor", "NCollisionVisualizer", "Sprite");
		SpriteComponent->SetRelativeScale3D(FVector3d(0.25f));
	}
}
#endif // WITH_EDITOR