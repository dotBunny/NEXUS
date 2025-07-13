// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Customizations/NSamplesLevelActorCustomization.h"
#include "CoreMinimal.h"
#include "NDetailLayoutBuilderUtils.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNSamplesLevelActorCustomization::MakeInstance()
{
	return MakeShareable(new FNSamplesLevelActorCustomization());
}

void FNSamplesLevelActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideActorCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideLevelCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideVisualCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HidePhysicsCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideNetworkingCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE