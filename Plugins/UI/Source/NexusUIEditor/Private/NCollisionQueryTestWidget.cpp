// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCollisionQueryTestWidget.h"
#include "NCollisionQueryTestUtils.h"
#include "Selection.h"

void UNCollisionQueryTestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bindings
	SelectStartButton->OnClicked.AddDynamic(this,&UNCollisionQueryTestWidget::SelectStartPoint);
	SelectEndButton->OnClicked.AddDynamic(this, &UNCollisionQueryTestWidget::SelectEndPoint);
	ImportSettingsButton->OnClicked.AddDynamic(this, &UNCollisionQueryTestWidget::OnImportSettingsClicked);
	ExportSettingsButton->OnClicked.AddDynamic(this, &UNCollisionQueryTestWidget::OnExportSettingsClicked);
	
	ObjectDetails->SetObject(this);
	ObjectDetails->bExpandedInDesigner = true;
	
	OnPIEMapCreatedHandle = FWorldDelegates::OnPIEMapCreated.AddUObject(this, &UNCollisionQueryTestWidget::OnPIEMapCreated);
	
	CheckProxyActor();
}

void UNCollisionQueryTestWidget::NativeDestruct()
{
	if (QueryActor != nullptr && QueryActor->IsValidLowLevel())
	{
		
		GEditor->GetSelectedActors()->Deselect(QueryActor);
		QueryActor->Destroy(true, false);
		QueryActor = nullptr;
	}
	
	FWorldDelegates::OnPIEStarted.Remove(OnPIEMapCreatedHandle);
	
	Super::NativeDestruct();
}

void UNCollisionQueryTestWidget::RestoreWidgetState(UObject* BlueprintWidget, FName Identifier, FNWidgetState& InState)
{
	// Load start/end position/ settings? 
	Super::RestoreWidgetState(BlueprintWidget, Identifier, InState);
}

FNWidgetState UNCollisionQueryTestWidget::GetWidgetState(UObject* BlueprintWidget)
{
	// Save settings?
	return Super::GetWidgetState(BlueprintWidget);
}

void UNCollisionQueryTestWidget::OnPIEMapCreated(UGameInstance* GameInstance)
{
	CheckProxyActor(true);
}

void UNCollisionQueryTestWidget::OnWorldTick()
{
	using enum ECollisionQueryTestMethod;
	if (Settings.Query.QueryMethod == LineTrace)
	{
		using enum ECollisionQueryTestPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionQueryTestUtils::DoLineTraceSingle(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetEndPosition());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionQueryTestUtils::DoLineTraceMulti(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetEndPosition());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionQueryTestUtils::DoLineTraceTest(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetEndPosition());
		}
	}
	else if (Settings.Query.QueryMethod == Sweep)
	{
		using enum ECollisionQueryTestPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionQueryTestUtils::DoSweepSingle(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetEndPosition(), 
				QueryActor->GetRotation());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionQueryTestUtils::DoSweepMulti(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetEndPosition(), 
				QueryActor->GetRotation());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionQueryTestUtils::DoSweepTest(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetEndPosition(), 
				QueryActor->GetRotation());
		}
	}
	else if (Settings.Query.QueryMethod == Overlap)
	{
		using enum ECollisionQueryTestOverlapBlocking;
		if (Settings.Query.QueryOverlapBlocking == Blocking)
		{
			FNCollisionQueryTestUtils::DoOverlapBlocking(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetRotation());
		}
		else if (Settings.Query.QueryOverlapBlocking == Any)
		{
			FNCollisionQueryTestUtils::DoOverlapAny(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetRotation());
		}
		else if (Settings.Query.QueryOverlapBlocking == Multi)
		{
			FNCollisionQueryTestUtils::DoOverlapMulti(Settings, QueryActor->GetWorld(), 
				QueryActor->GetStartPosition(), QueryActor->GetRotation());
		}
	}
}

void UNCollisionQueryTestWidget::OnImportSettingsClicked()
{
}

void UNCollisionQueryTestWidget::OnExportSettingsClicked()
{
}

void UNCollisionQueryTestWidget::SelectStartPoint()
{
	if (QueryActor != nullptr)
	{
		GEditor->GetSelectedActors()->DeselectAll();
		GEditor->GetSelectedComponents()->DeselectAll();
		
		GEditor->SelectActor(QueryActor, true, true, true, true);
		GEditor->SelectComponent(QueryActor->GetStartComponent(), true, true, true);
	}
}

void UNCollisionQueryTestWidget::SelectEndPoint()
{
	if (QueryActor != nullptr)
	{
		GEditor->GetSelectedActors()->DeselectAll();
		GEditor->GetSelectedComponents()->DeselectAll();
		
		GEditor->SelectActor(QueryActor, true, true, true, true);
		GEditor->SelectComponent(QueryActor->GetEndComponent(), true, true, true);
	}
}

void UNCollisionQueryTestWidget::CheckProxyActor(bool bWithDestroyExisting)
{
	GEditor->GetSelectedComponents()->DeselectAll();
	
	if (bWithDestroyExisting && QueryActor != nullptr)
	{
		QueryActor->Destroy(true, false);
		QueryActor = nullptr;
	}
	
	if (QueryActor == nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bTemporaryEditorActor = true;
		//SpawnParams.bHideFromSceneOutliner = true;
		SpawnParams.Name = MakeUniqueObjectName(GetWorld(), ANCollisionQueryTestActor::StaticClass(), TEXT("CollisionQueryTest"));
		
		QueryActor = GetWorld()->SpawnActor<ANCollisionQueryTestActor>(
			FVector::Zero(), FRotator::ZeroRotator, SpawnParams);
		
		QueryActor->Widget = this;
		
		ActorNameText->SetText(FText::FromString(*QueryActor->GetActorLabel()));
	}
	
	SelectStartPoint();
}