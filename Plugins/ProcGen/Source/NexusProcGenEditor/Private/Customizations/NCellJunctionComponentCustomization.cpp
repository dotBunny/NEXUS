// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellJunctionComponentCustomization.h"
#include "CoreMinimal.h"
#include "Misc/AssertionMacros.h"
#include "Cell/NCellActor.h"
#include "NDetailLayoutBuilderUtils.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNCellJunctionComponentCustomization::MakeInstance()
{
	return MakeShareable(new FNCellJunctionComponentCustomization());
}

void FNCellJunctionComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideVisualCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HidePhysicsCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE