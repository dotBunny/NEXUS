// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCollisionQueryTestWidget.h"
#include "NCollisionQueryTestUtils.h"

void UNCollisionQueryTestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bindings
	SelectStartButton->OnClicked.AddDynamic(this,&UNCollisionQueryTestWidget::OnSelectStartButtonClicked);
	SelectEndButton->OnClicked.AddDynamic(this, &UNCollisionQueryTestWidget::OnSelectEndButtonClicked);
	
	ObjectDetails->SetObject(this);
	
	OnPIEMapCreatedHandle = FWorldDelegates::OnPIEMapCreated.AddUObject(this, &UNCollisionQueryTestWidget::OnPIEMapCreated);
	
	CheckActors();
	
	// TODO: We need to check actors on recreate? 
	UE_LOG(LogTemp, Warning, TEXT("UNCollisionQueryTestWidget::NativeConstruct"));
}

void UNCollisionQueryTestWidget::NativeDestruct()
{
	QueryActor->Destroy();
	QueryActor = nullptr;
	
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
	UE_LOG(LogTemp, Warning, TEXT("UNCollisionQueryTestWidget::OnPIEMapCreated"));
	CheckActors();
}

void UNCollisionQueryTestWidget::OnWorldTick(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Sub Ticking"));
	//  Add update delay here ? 
	// TickTimer -= DeltaSeconds;
	// if (TickTimer <= 0)
	// {
	// 	TickTimer = UpdateTimer;
	// }
	
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

void UNCollisionQueryTestWidget::OnSelectStartButtonClicked()
{
	if (QueryActor != nullptr)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectComponent(QueryActor->GetStartComponent(), true, true, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Start actor is null"));
	}
}

void UNCollisionQueryTestWidget::OnSelectEndButtonClicked()
{
	if (QueryActor != nullptr)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectComponent(QueryActor->GetEndComponent(), true, true, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Start actor is null"));
	}
}

void UNCollisionQueryTestWidget::CheckActors()
{
	FActorSpawnParameters SpawnParams;
	// TODO: Do i want this?
	//SpawnParams.bHideFromSceneOutliner = true;
	
	if (QueryActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Creating actor!"));
		SpawnParams.Name = MakeUniqueObjectName(GetWorld(), ANCollisionQueryTestActor::StaticClass(), TEXT("CollisionQueryStart"));
		
		QueryActor = GetWorld()->SpawnActor<ANCollisionQueryTestActor>(
			FVector::Zero(), FRotator::ZeroRotator, SpawnParams);
		
		UE_LOG(LogTemp, Warning, TEXT("Bind Event"));
		QueryActor->Widget = this;
	}
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(QueryActor->GetEndComponent(), true, true, true);
}
