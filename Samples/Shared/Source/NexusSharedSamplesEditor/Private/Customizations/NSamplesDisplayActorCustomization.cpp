// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Customizations/NSamplesDisplayActorCustomization.h"
#include "CoreMinimal.h"
#include "NDetailLayoutBuilderUtils.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNSamplesDisplayActorCustomization::MakeInstance()
{
	return MakeShareable(new FNSamplesDisplayActorCustomization());
}

void FNSamplesDisplayActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideActorCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideLevelCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideVisualCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HidePhysicsCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideNetworkingCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE