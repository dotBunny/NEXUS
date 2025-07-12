// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NProcGenComponentCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "NProcGenComponent.h"
#include "Organ/NOrganGenerator.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNProcGenComponentCustomization::MakeInstance()
{
	return MakeShareable(new FNProcGenComponentCustomization());
}

void FNProcGenComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& NexusCategory = DetailBuilder.EditCategory(TEXT("NProcGen Actions"),
		FText::GetEmpty(), ECategoryPriority::Important);

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	
	FDetailWidgetRow& NewRow = NexusCategory.AddCustomRow(FText::GetEmpty());

	NewRow.ValueContent()
		.MaxDesiredWidth(120.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			.VAlign(VAlign_Fill)
			[
				SNew(SButton)
				.OnClicked(this, &FNProcGenComponentCustomization::OnGenerateClicked, ObjectsBeingCustomized)
				.ToolTipText(FText::FromString("Generate content for volume and contained volumes."))
				.Visibility(this, &FNProcGenComponentCustomization::GenerateButtonVisible)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					.VAlign(VAlign_Center)
					.AutoWidth()
					// [
					// 	SNew(SImage)
					// 	.DesiredSizeOverride(FVector2D(16,16))
					// 	.Image(FPCGEditorStyle::Get().GetBrush("PCG.Command.ForceRegenClearCache"))
					// 	.ColorAndOpacity(FSlateColor::UseForeground())
					// ]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("GenerateButton", "Generate"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			.VAlign(VAlign_Fill)
			[
				SNew(SButton)
				.OnClicked(this, &FNProcGenComponentCustomization::OnCancelClicked, ObjectsBeingCustomized)
				.Visibility(this, &FNProcGenComponentCustomization::CancelButtonVisible)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(16, 16))
						//.Image(FPCGEditorStyle::Get().GetBrush("PCG.Command.StopRegen"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("CancelButton", "Cancel"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked(this, &FNProcGenComponentCustomization::OnCleanupClicked, ObjectsBeingCustomized)
				.ToolTipText(FText::FromString("Cleans up generated content."))
				.Visibility(this, &FNProcGenComponentCustomization::CleanupButtonVisible)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("CleanupButton", "Cleanup"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked(this, &FNProcGenComponentCustomization::OnRefreshClicked, ObjectsBeingCustomized)
				.ToolTipText(FText::FromString("Cleanup previously generated content and generate it again."))
				.Visibility(this, &FNProcGenComponentCustomization::RefreshButtonVisible)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("RefreshButton", "Refresh"))
				]
			]
		];
}

FReply FNProcGenComponentCustomization::OnGenerateClicked(TArray<TWeakObjectPtr<UObject>> Objects)
{

	// Create our generation graph
	UNOrganGenerator* OrganGenerator = NewObject<UNOrganGenerator>();

	// Add components to the generation context
	for (TWeakObjectPtr<UObject> WeakObject : Objects)
	{
		if (UObject* Object = WeakObject.Get())
		{
			if (UNProcGenComponent* Component = Cast<UNProcGenComponent>(Object))
			{
				OrganGenerator->AddToContext(Component);
			}
		}
	}

	// Do some preprocessing, figure out order of generation, no more context can be added after this
	OrganGenerator->LockContext();

	// Add generation context to...
	OrganGenerator->Generate();
	
	return FReply::Handled();
}

FReply FNProcGenComponentCustomization::OnCancelClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	return FReply::Handled();
}

FReply FNProcGenComponentCustomization::OnCleanupClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	return FReply::Handled();
}

FReply FNProcGenComponentCustomization::OnRefreshClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	return FReply::Handled();
}

EVisibility FNProcGenComponentCustomization::GenerateButtonVisible() const
{
	return EVisibility::Visible;
}

EVisibility FNProcGenComponentCustomization::CancelButtonVisible() const
{
	return EVisibility::Collapsed;
}

EVisibility FNProcGenComponentCustomization::CleanupButtonVisible() const
{
	return EVisibility::Visible;
}

EVisibility FNProcGenComponentCustomization::RefreshButtonVisible() const
{
	return EVisibility::Visible;
}

#undef LOCTEXT_NAMESPACE