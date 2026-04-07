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
	IDetailCategoryBuilder& NexusCategory = DetailBuilder.EditCategory(TEXT("Actions"),
		FText::GetEmpty(), ECategoryPriority::Important);

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	
	FDetailWidgetRow& NewRow = NexusCategory.AddCustomRow(FText::FromString("actions"));
	
	NewRow.NameContent()
		[
			SNew(STextBlock)
				.Text(NSLOCTEXT("NexusProcGenEditor", "OrganComponent", "Organ"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		];
	NewRow.ValueContent()
		.MinDesiredWidth(500.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.5)
			.Padding(0,5,5,5)
			[
				SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton"))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.Text(NSLOCTEXT("NexusProcGenEditor", "OrganComponentGenerate", "Generate"))
					.ToolTipText(NSLOCTEXT("NexusProcGenEditor", "OrganComponentGenerateTooltip", "Generate content for volume and contained volumes."))
					.Visibility(this, &FNOrganComponentCustomization::GenerateButtonVisible)
					.OnClicked(this, &FNOrganComponentCustomization::OnGenerateClicked, ObjectsBeingCustomized)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.5)
			.Padding(0,5,5,5)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.Text(NSLOCTEXT("NexusProcGenEditor", "OrganComponentCancel", "Cancel"))
					.ToolTipText(NSLOCTEXT("NexusProcGenEditor", "OrganComponentCancelTooltip", "Cancel on-going generation for this."))
					.Visibility(this, &FNOrganComponentCustomization::CancelButtonVisible)
					.OnClicked(this, &FNOrganComponentCustomization::OnCancelClicked, ObjectsBeingCustomized)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.5)
			.Padding(0,5,5,5)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.Text(NSLOCTEXT("NexusProcGenEditor", "OrganComponentClear", "Clear"))
					.ToolTipText(NSLOCTEXT("NexusProcGenEditor", "OrganComponentClearTooltip", "Clears previously generated content for operation involving this component."))
					.Visibility(this, &FNOrganComponentCustomization::ClearButtonVisible)
					.OnClicked(this, &FNOrganComponentCustomization::OnClearClicked, ObjectsBeingCustomized)
			]
		];
}

FReply FNOrganComponentCustomization::OnGenerateClicked(const TArray<TWeakObjectPtr<UObject>> Objects)
{
	UNProcGenEditorSubsystem::Get()->StartOperation(UNProcGenOperation::CreateInstance(Objects));
	return FReply::Handled();
}

FReply FNOrganComponentCustomization::OnCancelClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	return FReply::Handled();
}

FReply FNOrganComponentCustomization::OnClearClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(Object);
	TArray<FName> UniqueGenerations;
	for (auto Component : OrganComponents)
	{
		FName LastGenerationName = Component->GetAndResetGenerationOperationKey();
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

EVisibility FNOrganComponentCustomization::ClearButtonVisible() const
{
	return EVisibility::Visible;
}