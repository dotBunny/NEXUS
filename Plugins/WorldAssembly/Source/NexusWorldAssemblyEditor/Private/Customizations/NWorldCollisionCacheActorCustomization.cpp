// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Customizations/NWorldCollisionCacheActorCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "IPropertyUtilities.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldCollisionBaker.h"
#include "NWorldCollisionCacheActor.h"
#include "NWorldCollisionPreview.h"
#include "Developer/NDebugActor.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedRef<IDetailCustomization> FNWorldCollisionCacheActorCustomization::MakeInstance()
{
	return MakeShared<FNWorldCollisionCacheActorCustomization>();
}

FNWorldCollisionCacheActorCustomization::~FNWorldCollisionCacheActorCustomization()
{
	if (UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get())
	{
		Subsystem->OnCollisionVisualizerChanged.Remove(VisualizerChangedHandle);
	}
	FNWorldCollisionBaker::OnBaked.Remove(BakedHandle);
}

void FNWorldCollisionCacheActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();

	// Both rows report state nothing in this panel necessarily changed: the visualizer can be toggled from the World
	// rail while this is on screen, and a bake can come from the rail, the save hook, or a preview notification's
	// Bake button. Under Slate global invalidation a bound attribute is only re-polled on incidental invalidation, so
	// without these the label and the summary would sit stale until reselection.
	if (UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get())
	{
		VisualizerChangedHandle = Subsystem->OnCollisionVisualizerChanged.AddSP(
			this, &FNWorldCollisionCacheActorCustomization::RequestRefresh);
	}
	BakedHandle = FNWorldCollisionBaker::OnBaked.AddSPLambda(this,
		[this](const UWorld*) { RequestRefresh(); });

	IDetailCategoryBuilder& CacheCategory = DetailBuilder.EditCategory(TEXT("World Collision Cache"),
		LOCTEXT("WorldCollisionCacheCategory", "World Collision Cache"), ECategoryPriority::Important);

	// Read-only: the pool is derived state with no editable properties, and what is worth knowing about it is not the
	// data but whether a run right now could use it.
	FDetailWidgetRow& StatusRow = CacheCategory.AddCustomRow(
		LOCTEXT("WorldCollisionCacheStatusFilter", "World Collision Cache Status"));

	StatusRow.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("WorldCollisionCacheStatusLabel", "Status"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		];
	StatusRow.ValueContent()
		.MinDesiredWidth(500.f)
		[
			SNew(STextBlock)
				.Text(this, &FNWorldCollisionCacheActorCustomization::GetCacheSummary)
				.ToolTipText(LOCTEXT("WorldCollisionCacheStatusTooltip",
					"What this level's baked collision pool holds, and whether it still matches the world. A stale or empty cache is not an error - the assembly gathers the world fresh instead."))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		];

	FDetailWidgetRow& ActionsRow = CacheCategory.AddCustomRow(
		LOCTEXT("WorldCollisionCacheActionsFilter", "World Collision Cache Actions"));

	ActionsRow.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("WorldCollisionCacheActions", "Actions"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		];
	ActionsRow.ValueContent()
		.MinDesiredWidth(500.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 5, 0, 0)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Text(this, &FNWorldCollisionCacheActorCustomization::GetVisualizerButtonText)
					.ToolTipText(this, &FNWorldCollisionCacheActorCustomization::GetVisualizerButtonToolTip)
					.OnClicked(this, &FNWorldCollisionCacheActorCustomization::OnToggleVisualizerClicked)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 5, 0, 0)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("WorldCollisionCacheSelectVisualizer", "Select Visualizer"))
					.ToolTipText(LOCTEXT("WorldCollisionCacheSelectVisualizerTooltip",
						"Select the live collision visualizer actor. This replaces the current selection, so these details are replaced by the visualizer's."))
					.IsEnabled(this, &FNWorldCollisionCacheActorCustomization::CanSelectVisualizer)
					.OnClicked(this, &FNWorldCollisionCacheActorCustomization::OnSelectVisualizerClicked)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 5, 0, 5)
			[
				SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton"))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("WorldCollisionCacheBake", "Bake World Collision"))
					.ToolTipText(LOCTEXT("WorldCollisionCacheBakeTooltip",
						"Re-bake every organ in this level now. Always a forced re-bake, whether or not the stored fingerprints still match."))
					.IsEnabled(this, &FNWorldCollisionCacheActorCustomization::CanBake)
					.OnClicked(this, &FNWorldCollisionCacheActorCustomization::OnBakeClicked)
			]
		];
}

UWorld* FNWorldCollisionCacheActorCustomization::GetCustomizedWorld() const
{
	if (CustomizedObjects.Num() != 1) return nullptr;

	const ANWorldCollisionCacheActor* CacheActor = Cast<ANWorldCollisionCacheActor>(CustomizedObjects[0].Get());
	return CacheActor != nullptr ? CacheActor->GetWorld() : nullptr;
}

FText FNWorldCollisionCacheActorCustomization::GetCacheSummary() const
{
	if (CustomizedObjects.Num() > 1)
	{
		// One cache actor per level, so this means several levels at once and no single answer to give.
		return LOCTEXT("WorldCollisionCacheMultiple", "Multiple levels selected.");
	}

	const ANWorldCollisionCacheActor* CacheActor = CustomizedObjects.Num() == 1
		? Cast<ANWorldCollisionCacheActor>(CustomizedObjects[0].Get())
		: nullptr;
	if (CacheActor == nullptr) return FText::GetEmpty();

	const int32 ElementCount = CacheActor->Pool.Num();
	FString Summary = CacheActor->Pool.IsEmpty()
		? FString(TEXT("Empty"))
		: FString::Printf(TEXT("%d element%s"), ElementCount, ElementCount == 1 ? TEXT("") : TEXT("s"));

	// Asked of the preview rather than worked out here, so this row and the visualizer always agree about whether the
	// level is showable — they are answering from the same memoized judgement.
	switch (FNWorldCollisionPreview::GetState(CacheActor->GetWorld()))
	{
		case FNWorldCollisionPreview::EState::Available:
			Summary += TEXT(" - current");
			break;
		case FNWorldCollisionPreview::EState::Stale:
			Summary += TEXT(" - stale, will gather fresh");
			break;
		case FNWorldCollisionPreview::EState::NotBaked:
			Summary += TEXT(" - not baked, the world is gathered on every run");
			break;
	}

	return FText::FromString(Summary);
}

FText FNWorldCollisionCacheActorCustomization::GetVisualizerButtonText() const
{
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	return Subsystem != nullptr && Subsystem->HasCollisionVisualizer()
		? LOCTEXT("WorldCollisionCacheRemoveVisualizer", "Remove Visualizer")
		: LOCTEXT("WorldCollisionCacheCreateVisualizer", "Create Visualizer");
}

FText FNWorldCollisionCacheActorCustomization::GetVisualizerButtonToolTip() const
{
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	return Subsystem != nullptr && Subsystem->HasCollisionVisualizer()
		? LOCTEXT("WorldCollisionCacheRemoveVisualizerTooltip",
			"Destroy the transient actor drawing this level's baked collision geometry.")
		: LOCTEXT("WorldCollisionCacheCreateVisualizerTooltip",
			"Spawn a transient actor drawing this level's baked collision geometry, as an assembly run will see it. An unbaked level has nothing to draw and will offer a bake instead.");
}

FReply FNWorldCollisionCacheActorCustomization::OnToggleVisualizerClicked()
{
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	if (Subsystem == nullptr) return FReply::Handled();

	if (Subsystem->HasCollisionVisualizer())
	{
		Subsystem->DestroyCollisionVisualizer();
		return FReply::Handled();
	}

	// The cache actor's own world, not the current editor world: this panel is bound to one level's cache, and that
	// is the pool the visualizer would be drawing.
	Subsystem->CreateCollisionVisualizer(GetCustomizedWorld());

	// No selection change here, unlike the rail's tile. That one selects what it spawned because it is acting on the
	// level at large; this panel is already showing something the user chose to look at, and stealing the selection
	// would close the panel they pressed the button in.
	return FReply::Handled();
}

FReply FNWorldCollisionCacheActorCustomization::OnSelectVisualizerClicked()
{
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	if (Subsystem == nullptr) return FReply::Handled();

	ANDebugActor* Visualizer = Subsystem->GetCollisionVisualizer();
	if (Visualizer == nullptr) return FReply::Handled();

	// Exclusively, matching the rail: the visualizer is one actor standing in for the whole level's collision, and an
	// additive selection would leave the gizmo on geometry the user has just replaced their view of.
	//
	// Notified, unlike the deselect above: with SelectNone passing false, nothing else would call NoteSelectionChange
	// and the details panel would sit on the outgoing selection.
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(Visualizer, true, true, true, true);

	return FReply::Handled();
}

bool FNWorldCollisionCacheActorCustomization::CanSelectVisualizer() const
{
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	return Subsystem != nullptr && Subsystem->HasCollisionVisualizer();
}

FReply FNWorldCollisionCacheActorCustomization::OnBakeClicked()
{
	// Always the whole level, never a selection. The rail narrows to selected organs because the selection is what the
	// user was pointing at when they reached for it; here the selection is this actor, which stands for the level's
	// entire pool — so a bake from this panel is a bake of everything that pool answers for.
	FNWorldAssemblyEditorUtils::CacheWorldCollision(GetCustomizedWorld(), {});
	return FReply::Handled();
}

bool FNWorldCollisionCacheActorCustomization::CanBake() const
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// No edit-mode gate, unlike the rail's equivalent: the rail can assume its mode is up, and this panel cannot. What
	// remains of that test is the part that is about the level rather than the UI — without an organ there is nothing
	// whose collision could be baked.
	return GetCustomizedWorld() != nullptr && FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

void FNWorldCollisionCacheActorCustomization::RequestRefresh() const
{
	if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
	{
		Utilities->RequestForceRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
