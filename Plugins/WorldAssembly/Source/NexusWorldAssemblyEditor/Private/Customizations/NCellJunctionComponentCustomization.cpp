// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellJunctionComponentCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Editor.h"
#include "Selection.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblySettings.h"
#include "Cell/INCellJunctionFiller.h"
#include "Cell/NCellJunctionComponent.h"
#include "Engine/World.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	/** Sentinel combo index representing the project-wide default junction filler (UNWorldAssemblySettings), not a Fillers entry. */
	constexpr int32 DefaultFillerIndex = INDEX_NONE;

	/** Humanizes a generated-class name for display: drops the Blueprint "_C" suffix, then NameToDisplayString. */
	FText MakeFillerDisplayText(const FString& InRawClassName)
	{
		FString ClassName = InRawClassName;
		ClassName.RemoveFromEnd(TEXT("_C"));
		return FText::FromString(FName::NameToDisplayString(ClassName, false));
	}

	/** @return The configured project-wide default junction filler class (loading it on demand), or null when unset. */
	UClass* ResolveDefaultFillerClass()
	{
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		return Settings != nullptr ? Settings->AssemblySpawningDefaultJunctionFiller.LoadSynchronous() : nullptr;
	}
}

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
	// Always offer the project-wide default junction filler (UNWorldAssemblySettings) as a trailing fallback option,
	// labeled "(Default)" — it is the actor Fill() spawns when none of a junction's authored fillers are eligible.
	FillerOptions.Add(MakeShared<int32>(DefaultFillerIndex));
	// Prefer the first authored filler as the initial selection, falling back to the default when none are authored.
	SelectedOption = FillerOptions[0];

	// Backstop cleanup: the destructor is the primary "junction deselected" path (the panel rebuilds and drops this
	// customization), but this also covers the panel keeping the layout alive after a viewport selection change.
	SelectionChangedHandle = USelection::SelectionChangedEvent.AddSP(this,
		&FNCellJunctionComponentCustomization::OnEditorSelectionChanged);

	// Keep a live preview in sync with edits to the authored Fillers data — child changes catch nested Offset edits,
	// the top-level change catches add/remove/reorder of entries.
	const TSharedRef<IPropertyHandle> FillersHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UNCellJunctionComponent, Fillers));
	if (FillersHandle->IsValidHandle())
	{
		const FSimpleDelegate OnFillersChanged = FSimpleDelegate::CreateSP(this,
			&FNCellJunctionComponentCustomization::OnFillersPropertyChanged);
		FillersHandle->SetOnPropertyValueChanged(OnFillersChanged);
		FillersHandle->SetOnChildPropertyValueChanged(OnFillersChanged);
	}

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
	// Default option: resolve the configured class from settings without forcing a load (use the soft path's asset name),
	// and tag it "(Default)" so it reads distinctly from the junction's authored fillers.
	if (FillerIndex == DefaultFillerIndex)
	{
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		const FSoftObjectPath DefaultPath = Settings != nullptr
			? Settings->AssemblySpawningDefaultJunctionFiller.ToSoftObjectPath() : FSoftObjectPath();
		if (DefaultPath.IsNull())
		{
			return NSLOCTEXT("NexusWorldAssemblyEditor", "FillerDefaultNone", "(None) (Default)");
		}
		return FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "FillerDefaultFormat", "{0} (Default)"),
			MakeFillerDisplayText(DefaultPath.GetAssetName()));
	}

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

	return MakeFillerDisplayText(FillerClass->GetName());
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
	if (Component == nullptr || !SelectedOption.IsValid())
	{
		return false;
	}

	const int32 Index = *SelectedOption;
	if (Index == DefaultFillerIndex)
	{
		// Enabled when a default filler is configured; avoid loading it just to gate the button (check the soft path).
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		return Settings != nullptr && !Settings->AssemblySpawningDefaultJunctionFiller.IsNull();
	}

	return Component->Fillers.IsValidIndex(Index) && Component->Fillers[Index].Actor != nullptr;
}

bool FNCellJunctionComponentCustomization::IsClearEnabled() const
{
	return PreviewActor.IsValid();
}

void FNCellJunctionComponentCustomization::ComputePreviewPlacement(const FTransform& Offset, FVector& OutLocation, FQuat& OutRotation) const
{
	const UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr)
	{
		OutLocation = FVector::ZeroVector;
		OutRotation = FQuat::Identity;
		return;
	}

	// Mirror UNCellJunctionComponent::Fill(): the location offset is authored in the junction's frame (rotate it by
	// the junction's orientation before adding); the rotation offset then spins the preview in place at that spot.
	const FQuat JunctionRotation = Component->GetComponentRotation().Quaternion();
	OutLocation = Component->GetComponentLocation() + JunctionRotation.RotateVector(Offset.GetLocation());
	OutRotation = JunctionRotation * FQuat(Offset.Rotator());
}

void FNCellJunctionComponentCustomization::OnFillersPropertyChanged()
{
	// The default option reads its placement from settings (identity offset), so authored-Fillers edits never move it.
	if (!PreviewActor.IsValid() || !SelectedOption.IsValid() || *SelectedOption == DefaultFillerIndex)
	{
		return;
	}

	UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr || !Component->Fillers.IsValidIndex(*SelectedOption))
	{
		return;
	}

	const FNCellJunctionFillerEntry& Entry = Component->Fillers[*SelectedOption];

	// A changed (or cleared) Actor needs a fresh spawn; an Offset-only edit just re-places the existing preview.
	AActor* Preview = PreviewActor.Get();
	if (Preview->GetClass() != Entry.Actor.Get())
	{
		SpawnPreview(*SelectedOption);
		return;
	}

	FVector NewLocation;
	FQuat NewRotation;
	ComputePreviewPlacement(Entry.Offset, NewLocation, NewRotation);
	Preview->SetActorLocationAndRotation(NewLocation, NewRotation);
	Preview->SetActorScale3D(PreviewBaseScale * Entry.Offset.GetScale3D());
}

void FNCellJunctionComponentCustomization::SpawnPreview(int32 FillerIndex)
{
	// Always start from a clean slate so Fill / dropdown-change never leak a second preview.
	DestroyPreview();

	UNCellJunctionComponent* Component = Junction.Get();
	if (Component == nullptr) return;

	// Resolve the class to spawn and its placement offset. The default option carries no authored offset, so it
	// previews at the junction's transform exactly as Fill() spawns the project-wide fallback.
	UClass* FillerClass = nullptr;
	FTransform Offset = FTransform::Identity;
	if (FillerIndex == DefaultFillerIndex)
	{
		FillerClass = ResolveDefaultFillerClass();
	}
	else
	{
		if (!Component->Fillers.IsValidIndex(FillerIndex)) return;
		const FNCellJunctionFillerEntry& Entry = Component->Fillers[FillerIndex];
		FillerClass = Entry.Actor.Get();
		Offset = Entry.Offset;
	}
	if (FillerClass == nullptr) return;

	UWorld* World = Component->GetWorld();
	if (World == nullptr) return;

	// Location & rotation follow the junction frame; the scale offset multiplies the spawned actor's own scale.
	FVector SpawnLocation;
	FQuat SpawnRotation;
	ComputePreviewPlacement(Offset, SpawnLocation, SpawnRotation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = Component->GetComponentLevel();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	AActor* SpawnedActor = World->SpawnActor<AActor>(FillerClass, SpawnLocation, SpawnRotation.Rotator(), SpawnParams);
	if (SpawnedActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to spawn filler preview actor for junction '%s'."), *Component->GetName());
		return;
	}

	// Capture the actor's intrinsic scale before applying the offset so later offset edits re-scale from the same base.
	PreviewBaseScale = SpawnedActor->GetActorScale3D();
	SpawnedActor->SetActorScale3D(PreviewBaseScale * Offset.GetScale3D());
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
