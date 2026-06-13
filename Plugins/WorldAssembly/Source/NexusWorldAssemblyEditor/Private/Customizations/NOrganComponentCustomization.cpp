// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NOrganComponentCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyRegistry.h"
#include "Assembly/NAssemblyOperation.h"
#include "Organ/NOrganComponent.h"

TSharedRef<IDetailCustomization> FNOrganComponentCustomization::MakeInstance()
{
	return MakeShared<FNOrganComponentCustomization>();
}

FNOrganComponentCustomization::~FNOrganComponentCustomization()
{
	FNWorldAssemblyRegistry::OnOperationStateChanged.Remove(OperationStateChangedHandle);
}

EVisibility FNOrganComponentCustomization::GenerateButtonVisible() const
{
	return GetActiveOperations().IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed;
}

void FNOrganComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Refresh the panel when operation state changes so the gated action buttons (e.g. "Generate")
	// re-evaluate their IsEnabled attribute; under Slate global invalidation a bound attribute is
	// otherwise only re-polled on incidental invalidation, leaving the button stuck until reselection.
	// We listen to the registry rather than the editor subsystem because the buttons gate on
	// FNWorldAssemblyRegistry::HasOperations(): the registry broadcasts after it mutates its operation
	// set, so the refresh always re-polls against up-to-date state.
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();
	OperationStateChangedHandle = FNWorldAssemblyRegistry::OnOperationStateChanged.AddSP(
		this, &FNOrganComponentCustomization::HandleOperationStateChanged);

	IDetailCategoryBuilder& NexusCategory = DetailBuilder.EditCategory(TEXT("Organ Component"),
FText::FromString("Organ Component"), ECategoryPriority::Important);

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	CustomizedObjects = ObjectsBeingCustomized;

	FDetailWidgetRow& NewRow = NexusCategory.AddCustomRow(FText::FromString("Actions"));
	
	NewRow.NameContent()
		[
			SNew(STextBlock)
				.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganActions", "Actions"))
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
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganComponentGenerate", "Generate"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganComponentGenerateTooltip", "Generate content for volume and contained volumes."))
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
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganComponentCancel", "Cancel"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganComponentCancelTooltip", "Cancel on-going generation for this."))
					.Visibility(this, &FNOrganComponentCustomization::CancelButtonVisible)
					.OnClicked(this, &FNOrganComponentCustomization::OnCancelClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.5)
			.Padding(0,5,5,5)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganComponentClear", "Clear"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganComponentClearTooltip", "Clears previously generated content for operation involving this component."))
					.Visibility(this, &FNOrganComponentCustomization::ClearButtonVisible)
					.OnClicked(this, &FNOrganComponentCustomization::OnClearClicked, ObjectsBeingCustomized)
			]
		];
}

FReply FNOrganComponentCustomization::OnGenerateClicked(const TArray<TWeakObjectPtr<UObject>> Objects)
{
	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	UNWorldAssemblyEditorSubsystem::Get()->StartOperation(UNAssemblyOperation::CreateInstance(Objects, EditorSettings));
	return FReply::Handled();
}

FReply FNOrganComponentCustomization::OnCancelClicked()
{
	for (UNAssemblyOperation* Operation : GetActiveOperations())
	{
		Operation->Cancel();
	}
	return FReply::Handled();
}

FReply FNOrganComponentCustomization::OnClearClicked(TArray<TWeakObjectPtr<UObject>> Object)
{
	TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(Object);
	TArray<int32> UniqueGenerations;
	for (auto Component : OrganComponents)
	{
		int32 LastOperationTicket = Component->GetAndResetLastOperationTicket();
		if (LastOperationTicket != 0 && !UniqueGenerations.Contains(LastOperationTicket))
		{
			UniqueGenerations.Add(LastOperationTicket);
		}
	}
	for (auto OperationTicket : UniqueGenerations)
	{
		UNWorldAssemblyEditorSubsystem::Get()->ClearGenerated(OperationTicket);
	}
	
	return FReply::Handled();
}

EVisibility FNOrganComponentCustomization::CancelButtonVisible() const
{
	return GetActiveOperations().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

TArray<UNAssemblyOperation*> FNOrganComponentCustomization::GetActiveOperations() const
{
	TArray<UNAssemblyOperation*> ActiveOperations;
	const TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(CustomizedObjects);
	if (OrganComponents.IsEmpty())
	{
		return ActiveOperations;
	}

	for (UNAssemblyOperation* Operation : FNWorldAssemblyRegistry::GetOperations())
	{
		if (Operation == nullptr || !Operation->IsRunning())
		{
			continue;
		}
		for (const UNOrganComponent* Component : OrganComponents)
		{
			if (Operation->ContainsComponent(Component))
			{
				ActiveOperations.Add(Operation);
				break;
			}
		}
	}
	return ActiveOperations;
}

EVisibility FNOrganComponentCustomization::ClearButtonVisible() const
{
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (const UNOrganComponent* Component : UNOrganComponent::GetOrganComponents(CustomizedObjects))
	{
		const int32 LastOperationTicket = Component->GetLastOperationTicket();
		if (LastOperationTicket != 0 && Subsystem->HasGeneratedProxies(LastOperationTicket))
		{
			return EVisibility::Visible;
		}
	}
	return EVisibility::Collapsed;
}

bool FNOrganComponentCustomization::CanGenerate() const
{
	if (!FNEditorUtils::IsNotPlayInEditor() || FNWorldAssemblyRegistry::HasOperations())
	{
		return false;
	}
	return !UNOrganComponent::GetOrganComponents(CustomizedObjects).IsEmpty();
}

void FNOrganComponentCustomization::HandleOperationStateChanged(UNAssemblyOperation* Operation, ENWorldAssemblyOperationState NewState)
{
	if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
	{
		Utilities->RequestForceRefresh();
	}
}