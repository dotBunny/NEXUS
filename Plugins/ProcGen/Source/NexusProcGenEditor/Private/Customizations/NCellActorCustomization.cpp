// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellActorCustomization.h"
#include "CoreMinimal.h"
#include "Misc/AssertionMacros.h"
#include "Cell/NCellActor.h"
#include "NDetailLayoutBuilderUtils.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNCellActorCustomization::MakeInstance()
{
	return MakeShareable(new FNCellActorCustomization());
}

void FNCellActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideActorCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideLevelCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideVisualCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HidePhysicsCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideNetworkingCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE