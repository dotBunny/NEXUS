// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Customizations/NActorPoolSpawnerComponentCustomization.h"
#include "CoreMinimal.h"
#include "Misc/AssertionMacros.h"
#include "NDetailLayoutBuilderUtils.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNActorPoolSpawnerComponentCustomization::MakeInstance()
{
	return MakeShareable(new FNActorPoolSpawnerComponentCustomization());
}

void FNActorPoolSpawnerComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideVisualCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HidePhysicsCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE