// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellRootComponentCustomization.h"
#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Misc/AssertionMacros.h"
#include "DetailWidgetRow.h"
#include "Cell/NCellActor.h"
#include "NDetailLayoutBuilderUtils.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNCellRootComponentCustomization::MakeInstance()
{
	return MakeShareable(new FNCellRootComponentCustomization());
}

void FNCellRootComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create or get the category
	IDetailCategoryBuilder& NexusCategory = DetailBuilder.EditCategory(TEXT("NEXUS Notices"),
		FText::GetEmpty(), ECategoryPriority::Important);

	// Figure out if we are editing multiple objects, in this case we don't want a special inspector if 
	// more then one of the components is selected.
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}

	TWeakObjectPtr<UNCellRootComponent> CellRoot = Cast<UNCellRootComponent>(Objects[0].Get());

	auto OnBoundsNoCalcWarning = [CellRoot]
	{
		if (CellRoot->Details.BoundsSettings.bCalculateOnSave)
		{
			return EVisibility::Hidden;
		}
		return EVisibility::Visible;
	};
	NexusCategory.AddCustomRow(LOCTEXT("Validation", "Alerts"))
		.Visibility( TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(OnBoundsNoCalcWarning)))
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						//.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("RichTextBlock.Bold"))
						.Text(LOCTEXT("BoundsNoCalc", "- The NCell Bounds are not calculated on save."))
						.ColorAndOpacity(FSlateColor(EStyleColor::AccentWhite))
				]
		];


	auto OnHullNoCalcWarning = [CellRoot]
	{
		if (CellRoot->Details.HullSettings.bCalculateOnSave)
		{
			return EVisibility::Hidden;
		}
		return EVisibility::Visible;
	};
	NexusCategory.AddCustomRow(LOCTEXT("Validation", "Alerts"))
		.Visibility( TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(OnHullNoCalcWarning)))
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						//.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("RichTextBlock.Bold"))
						.Text(LOCTEXT("HullNoCalc", "- The NCell Convex Hull is not calculated on save."))
						.ColorAndOpacity(FSlateColor(EStyleColor::AccentWhite))
				]
		];

	FNDetailLayoutBuilderUtils::HideDefaultCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HideVisualCategories(DetailBuilder);
	FNDetailLayoutBuilderUtils::HidePhysicsCategories(DetailBuilder);
}

#undef LOCTEXT_NAMESPACE