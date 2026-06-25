// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellJunctionComponentCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "Selection.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "Cell/INCellJunctionFiller.h"
#include "Cell/NCellJunctionComponent.h"
#include "Engine/World.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FNCellJunctionComponentCustomization::MakeInstance()
{
	return MakeShared<FNCellJunctionComponentCustomization>();
}

FNCellJunctionComponentCustomization::~FNCellJunctionComponentCustomization()
{
	if (SelectionChangedHandle.IsValid())
	{
		USelection::SelectionChangedEvent.Remove(SelectionChangedHandle);
	}

	// Junction is no longer being inspected; the throwaway preview should not outlive the panel.
	DestroyPreview();
}

void FNCellJunctionComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& FillerCategory = DetailBuilder.EditCategory(TEXT("Filler Visualizer"),
		FText::GetEmpty(), ECategoryPriority::Important);

	// Single-junction only: each junction has its own Fillers list, so a shared dropdown across a multi-selection
	// is meaningless.
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		FillerCategory.AddCustomRow(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerVisualizer", "Filler Visualizer"))
			.WholeRowContent()
			[
				SNew(STextBlock)
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerSelectSingle", "Select a single junction to preview its fillers."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
			];
		return;
	}

	Junction = Cast<UNCellJunctionComponent>(Objects[0].Get());
	const UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr) return;

	// Dropdown source is index-based so the live Fillers entry (Actor/Offset) is read on demand.
	FillerOptions.Reset();
	for (int32 Index = 0; Index < Component->Fillers.Num(); ++Index)
	{
		FillerOptions.Add(MakeShared<int32>(Index));
	}
	SelectedOption = FillerOptions.Num() > 0 ? FillerOptions[0] : nullptr;

	// Backstop cleanup: the destructor is the primary "junction deselected" path (the panel rebuilds and drops this
	// customization), but this also covers the panel keeping the layout alive after a viewport selection change.
	SelectionChangedHandle = USelection::SelectionChangedEvent.AddSP(this,
		&FNCellJunctionComponentCustomization::OnEditorSelectionChanged);

	FDetailWidgetRow& Row = FillerCategory.AddCustomRow(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerVisualizer", "Filler Visualizer"));
	Row.NameContent()
		[
			SNew(STextBlock)
				.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerPreview", "Preview"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		];
	Row.ValueContent()
		.MinDesiredWidth(500.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(0, 0, 5, 0)
			[
				SNew(SComboBox<TSharedPtr<int32>>)
					.OptionsSource(&FillerOptions)
					.InitiallySelectedItem(SelectedOption)
					.OnGenerateWidget(this, &FNCellJunctionComponentCustomization::OnGenerateFillerWidget)
					.OnSelectionChanged(this, &FNCellJunctionComponentCustomization::OnFillerSelectionChanged)
					[
						SNew(STextBlock)
							.Text(this, &FNCellJunctionComponentCustomization::GetSelectedFillerLabel)
							.Font(IDetailLayoutBuilder::GetDetailFont())
					]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 5, 0)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerFill", "Fill"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerFillTooltip", "Spawn the selected filler as a transient preview at this junction."))
					.IsEnabled(this, &FNCellJunctionComponentCustomization::IsFillEnabled)
					.OnClicked(this, &FNCellJunctionComponentCustomization::OnFillClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerClear", "Clear"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerClearTooltip", "Remove the spawned filler preview from the level."))
					.IsEnabled(this, &FNCellJunctionComponentCustomization::IsClearEnabled)
					.OnClicked(this, &FNCellJunctionComponentCustomization::OnClearClicked)
			]
		];
}

FText FNCellJunctionComponentCustomization::GetFillerLabel(int32 FillerIndex) const
{
	const UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr || !Component->Fillers.IsValidIndex(FillerIndex))
	{
		return NSLOCTEXT("NexusWorldAssemblyEditor", "FillerNone", "(None)");
	}

	const UClass* FillerClass = Component->Fillers[FillerIndex].Actor.Get();
	if (FillerClass == nullptr)
	{
		return NSLOCTEXT("NexusWorldAssemblyEditor", "FillerNone", "(None)");
	}

	// Friendly display name: drop the Blueprint generated-class "_C" suffix, then humanize.
	FString ClassName = FillerClass->GetName();
	ClassName.RemoveFromEnd(TEXT("_C"));
	return FText::FromString(FName::NameToDisplayString(ClassName, false));
}

FText FNCellJunctionComponentCustomization::GetSelectedFillerLabel() const
{
	return SelectedOption.IsValid() ? GetFillerLabel(*SelectedOption) : FText::GetEmpty();
}

TSharedRef<SWidget> FNCellJunctionComponentCustomization::OnGenerateFillerWidget(TSharedPtr<int32> InOption) const
{
	return SNew(STextBlock)
		.Text(InOption.IsValid() ? GetFillerLabel(*InOption) : FText::GetEmpty())
		.Font(IDetailLayoutBuilder::GetDetailFont());
}

void FNCellJunctionComponentCustomization::OnFillerSelectionChanged(TSharedPtr<int32> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedOption = NewSelection;

	// Replace a live preview so the spawned actor always reflects the current dropdown selection.
	if (PreviewActor.IsValid() && SelectedOption.IsValid())
	{
		SpawnPreview(*SelectedOption);
	}
}

FReply FNCellJunctionComponentCustomization::OnFillClicked()
{
	if (SelectedOption.IsValid())
	{
		SpawnPreview(*SelectedOption);
	}
	return FReply::Handled();
}

FReply FNCellJunctionComponentCustomization::OnClearClicked()
{
	DestroyPreview();
	return FReply::Handled();
}

bool FNCellJunctionComponentCustomization::IsFillEnabled() const
{
	const UNCellJunctionComponent* Component = Junction.Get();
	return Component != nullptr && SelectedOption.IsValid() &&
		Component->Fillers.IsValidIndex(*SelectedOption) &&
		Component->Fillers[*SelectedOption].Actor != nullptr;
}

bool FNCellJunctionComponentCustomization::IsClearEnabled() const
{
	return PreviewActor.IsValid();
}

void FNCellJunctionComponentCustomization::SpawnPreview(int32 FillerIndex)
{
	// Always start from a clean slate so Fill / dropdown-change never leak a second preview.
	DestroyPreview();

	UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr || !Component->Fillers.IsValidIndex(FillerIndex)) return;

	const FNCellJunctionFillerEntry& Entry = Component->Fillers[FillerIndex];
	if (Entry.Actor == nullptr) return;

	UWorld* World = Component->GetWorld();
	if (World == nullptr) return;

	// Mirror the placement applied by UNCellJunctionComponent::Fill(): the location offset is authored in the
	// junction's frame (rotate it by the junction's orientation before adding), the rotation offset then spins the
	// preview in place at that spot, and the scale offset multiplies the spawned actor's own scale.
	const FQuat JunctionRotation = Component->GetComponentRotation().Quaternion();
	const FVector SpawnLocation = Component->GetComponentLocation() + JunctionRotation.RotateVector(Entry.Offset.GetLocation());
	const FRotator SpawnRotation = (JunctionRotation * FQuat(Entry.Offset.Rotator())).Rotator();

	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = Component->GetComponentLevel();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	AActor* SpawnedActor = World->SpawnActor<AActor>(Entry.Actor, SpawnLocation, SpawnRotation, SpawnParams);
	if (SpawnedActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to spawn filler preview actor for junction '%s'."), *Component->GetName());
		return;
	}

	SpawnedActor->SetActorScale3D(SpawnedActor->GetActorScale3D() * Entry.Offset.GetScale3D());
	PreviewActor = SpawnedActor;

	// Notify the filler as the runtime path does (FinalizeFillerSpawn). There is no generated cell at author time,
	// so the level-instance context is null; the junction and its persisted identifier are still meaningful. The
	// interface event is CallInEditor, so this is safe to invoke here.
	if (SpawnedActor->Implements<UNCellJunctionFiller>())
	{
		INCellJunctionFiller::Execute_OnInitializedFromJunction(SpawnedActor, nullptr, Component, Component->Details.InstanceIdentifier);
	}
}

void FNCellJunctionComponentCustomization::DestroyPreview()
{
	AActor* Actor = PreviewActor.Get();
	if (Actor != nullptr)
	{
		if (UWorld* World = Actor->GetWorld())
		{
			// Transient preview only — do not modify/dirty the level on teardown.
			World->EditorDestroyActor(Actor, /*bShouldModifyLevel*/ false);
		}
	}
	PreviewActor.Reset();
}

void FNCellJunctionComponentCustomization::OnEditorSelectionChanged(UObject* NewSelection)
{
	const UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr)
	{
		DestroyPreview();
		return;
	}

	const AActor* Owner = Component->GetOwner();
	if (Owner == nullptr || !GEditor->GetSelectedActors()->IsSelected(Owner))
	{
		DestroyPreview();
	}
}
