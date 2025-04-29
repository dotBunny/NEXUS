// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferencesComponentCustomization.h"
#include "CoreMinimal.h"
#include "NDetailLayoutBuilderUtils.h"
#include "Misc/AssertionMacros.h"

#define LOCTEXT_NAMESPACE "NexusDynamicReferencesEditor"

TSharedRef<IDetailCustomization> FNDynamicReferencesComponentCustomization::MakeInstance()
{
	return MakeShareable(new FNDynamicReferencesComponentCustomization());
}

void FNDynamicReferencesComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE