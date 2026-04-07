// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NOrganComponentCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "NProcGenEditorSubsystem.h"
#include "NProcGenOperation.h"
#include "Organ/NOrganComponent.h"

TSharedRef<IDetailCustomization> FNOrganComponentCustomization::MakeInstance()
{
	return MakeShared<FNOrganComponentCustomization>();
}

void FNOrganComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
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
				.OnClicked(this, &FNOrganComponentCustomization::OnGenerateClicked, ObjectsBeingCustomized)
				.ToolTipText(FText::FromString("Generate content for volume and contained volumes."))
				.Visibility(this, &FNOrganComponentCustomization::GenerateButtonVisible)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					.VAlign(VAlign_Center)
					.AutoWidth()
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(NSLOCTEXT("NexusProcGenEditor", "GenerateButton", "Generate"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			.VAlign(VAlign_Fill)
			[
				SNew(SButton)
				.OnClicked(this, &FNOrganComponentCustomization::OnCancelClicked, ObjectsBeingCustomized)
				.Visibility(this, &FNOrganComponentCustomization::CancelButtonVisible)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(16, 16))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(NSLOCTEXT("NexusProcGenEditor", "CancelButton", "Cancel"))
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
				.OnClicked(this, &FNOrganComponentCustomization::OnCleanupClicked, ObjectsBeingCustomized)
				.ToolTipText(FText::FromString("Clears previously generated content for operation involving this component."))
				.Visibility(this, &FNOrganComponentCustomization::CleanupButtonVisible)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(NSLOCTEXT("NexusProcGenEditor", "CleanupButton", "Cleanup"))
				]
			]
		];
}

FReply FNOrganComponentCustomization::OnGenerateClicked(const TArray<TWeakObjectPtr<UObject>> Objects)
{
	// TODO: should we be clearing the previous?
	
	// Create and start generation operation
	
	UNProcGenEditorSubsystem::Get()->StartOperation(UNProcGenOperation::CreateInstance(Objects));
	return FReply::Handled();
}

FReply FNOrganComponentCustomization::OnCancelClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	return FReply::Handled();
}

FReply FNOrganComponentCustomization::OnCleanupClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(Object);
	TArray<FName> UniqueGenerations;
	for (auto Component : OrganComponents)
	{
		FName LastGenerationName = Component->GetLastGenerationOperationKey();
		if (LastGenerationName != NAME_None && !UniqueGenerations.Contains(LastGenerationName))
		{
			UniqueGenerations.Add(LastGenerationName);
		}
	}
	for (auto Key : UniqueGenerations)
	{
		UNProcGenEditorSubsystem::Get()->ClearGeneratedProxies(Key);
	}
	
	return FReply::Handled();
}


EVisibility FNOrganComponentCustomization::GenerateButtonVisible() const
{
	return EVisibility::Visible;
}

EVisibility FNOrganComponentCustomization::CancelButtonVisible() const
{
	return EVisibility::Collapsed;
}

EVisibility FNOrganComponentCustomization::CleanupButtonVisible() const
{
	return EVisibility::Visible;
}