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
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionBaker.h"
#include "NWorldCollisionFingerprint.h"
#include "Organ/NOrganComponent.h"
#include "ScopedTransaction.h"

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

	// The collision cache is derived state with no editable properties, so it gets a read-only summary rather than a
	// property row: what it holds, and whether it still matches the world. Without this the only way to know an organ
	// is quietly gathering fresh every run is to read the log.
	FDetailWidgetRow& CacheRow = NexusCategory.AddCustomRow(
		NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCache", "World Collision Cache"));

	CacheRow.NameContent()
		[
			SNew(STextBlock)
				.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCacheLabel", "World Collision Cache"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		];
	CacheRow.ValueContent()
		.MinDesiredWidth(500.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(this, &FNOrganComponentCustomization::GetCollisionCacheSummary)
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCacheTooltip",
						"What world collision this organ has baked, and whether it still matches the level. A stale or empty cache is not an error - the assembly gathers the world fresh instead."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 5, 0, 5)
			[
				SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCacheBake", "Bake"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCacheBakeTooltip",
						"Re-bake this organ's world collision now."))
					.OnClicked(this, &FNOrganComponentCustomization::OnBakeCollisionCacheClicked, ObjectsBeingCustomized)
			]
		];
}

FText FNOrganComponentCustomization::GetCollisionCacheSummary() const
{
	const TArray<UNOrganComponent*> Organs = UNOrganComponent::GetOrganComponents(CustomizedObjects);
	if (Organs.IsEmpty())
	{
		return FText::GetEmpty();
	}

	// Multi-select reports a count rather than trying to reconcile several organs' states into one line.
	if (Organs.Num() > 1)
	{
		int32 BakedCount = 0;
		for (const UNOrganComponent* Organ : Organs)
		{
			if (IsValid(Organ) && Organ->CollisionCache.HasData())
			{
				BakedCount++;
			}
		}
		return FText::FromString(FString::Printf(TEXT("%d of %d organs baked"), BakedCount, Organs.Num()));
	}

	const UNOrganComponent* Organ = Organs[0];
	if (!IsValid(Organ) || !Organ->CollisionCache.HasData())
	{
		return NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCacheNone", "Not baked - the world is gathered on every run.");
	}

	const int32 ElementCount = Organ->CollisionCache.SourceKeys.Num();
	const FString BakedAt = Organ->CollisionCache.BakeTime.ToString(TEXT("%Y-%m-%d %H:%M"));

	// Two lines: what the cache holds, then when it was made. The status rides the first line because whether a run
	// can actually use this is the headline, not a footnote to the timestamp.
	FString Summary = FString::Printf(TEXT("%d element%s"), ElementCount, ElementCount == 1 ? TEXT("") : TEXT("s"));

	// Fingerprinted live so the row answers the question that actually matters — not "was this ever baked" but
	// "would a run right now be able to use it".
	const UWorld* World = Organ->GetWorld();
	if (World != nullptr)
	{
		TArray<FBoxSphereBounds> Bounds;
		const FNWorldCollisionBaker::EOrganBoundsKind BoundsKind = FNWorldCollisionBaker::GetOrganBounds(Organ, Bounds);
		if (BoundsKind != FNWorldCollisionBaker::EOrganBoundsKind::None)
		{
			const uint64 Current = FNWorldCollisionFingerprint::Compute(World, Bounds,
				UNWorldAssemblySettings::Get()->WorldCollisionSettings);

			Summary += Organ->CollisionCache.IsValidFor(Current)
				? TEXT(" - current")
				: TEXT(" - stale, will gather fresh");
		}
	}

	Summary += FString::Printf(TEXT("\nBaked on %s"), *BakedAt);

	return FText::FromString(Summary);
}

FReply FNOrganComponentCustomization::OnBakeCollisionCacheClicked(const TArray<TWeakObjectPtr<UObject>> Objects)
{
	const TArray<UNOrganComponent*> Organs = UNOrganComponent::GetOrganComponents(Objects);
	if (Organs.IsEmpty())
	{
		return FReply::Handled();
	}

	UWorld* World = Organs[0]->GetWorld();
	if (World == nullptr)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(
		NSLOCTEXT("NexusWorldAssemblyEditor", "OrganCollisionCacheBakeTransaction", "Bake Organ World Collision"));

	FNWorldCollisionBaker::BakeOrgans(World, Organs,
		UNWorldAssemblySettings::Get()->WorldCollisionSettings, true);

	return FReply::Handled();
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