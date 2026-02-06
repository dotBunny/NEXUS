// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCollisionQueryTestWidget.h"

#include "NCollisionQueryTestUtils.h"

void UNCollisionQueryTestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.bHideFromSceneOutliner = true;
	
	SpawnParams.Name = MakeUniqueObjectName(GetWorld(), ANTransientActor::StaticClass(), TEXT("CollisionQueryStart"));
	StartActor = GetWorld()->SpawnActor<ANTransientActor>(
		FVector::Zero(), FRotator::ZeroRotator, SpawnParams);
	SpawnParams.Name = MakeUniqueObjectName(GetWorld(), ANTransientActor::StaticClass(), TEXT("CollisionQueryEnd"));
	EndActor = GetWorld()->SpawnActor<ANTransientActor>(
		FVector(0,0,500.f), FRotator::ZeroRotator, SpawnParams);
	
	GEditor->SelectActor(EndActor, true, true, true, true);
	
	// Bindings
	SelectStartButton->OnClicked.AddDynamic(this,&UNCollisionQueryTestWidget::OnSelectStartButtonClicked);
	SelectEndButton->OnClicked.AddDynamic(this, &UNCollisionQueryTestWidget::OnSelectEndButtonClicked);
	
	ObjectDetails->SetObject(this);
	
	// We're going to have the actor handle pumping the query
	StartActor->OnTick.BindUObject(this, &UNCollisionQueryTestWidget::OnWorldTick); 
}

void UNCollisionQueryTestWidget::NativeDestruct()
{
	StartActor->Destroy();
	StartActor = nullptr;
	
	EndActor->Destroy();
	EndActor = nullptr;
	
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

void UNCollisionQueryTestWidget::OnWorldTick(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Ticking"));
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
			FNCollisionQueryTestUtils::DoLineTraceSingle(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), EndActor->GetActorLocation());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionQueryTestUtils::DoLineTraceMulti(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), EndActor->GetActorLocation());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionQueryTestUtils::DoLineTraceTest(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), EndActor->GetActorLocation());
		}
	}
	else if (Settings.Query.QueryMethod == Sweep)
	{
		using enum ECollisionQueryTestPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionQueryTestUtils::DoSweepSingle(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), EndActor->GetActorLocation(), 
				StartActor->GetActorQuat());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionQueryTestUtils::DoSweepMulti(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), EndActor->GetActorLocation(), 
				StartActor->GetActorQuat());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionQueryTestUtils::DoSweepTest(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), EndActor->GetActorLocation(), 
				StartActor->GetActorQuat());
		}
	}
	else if (Settings.Query.QueryMethod == Overlap)
	{
		using enum ECollisionQueryTestOverlapBlocking;
		if (Settings.Query.QueryOverlapBlocking == Blocking)
		{
			FNCollisionQueryTestUtils::DoOverlapBlocking(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), StartActor->GetActorQuat());
		}
		else if (Settings.Query.QueryOverlapBlocking == Any)
		{
			FNCollisionQueryTestUtils::DoOverlapAny(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), StartActor->GetActorQuat());
		}
		else if (Settings.Query.QueryOverlapBlocking == Multi)
		{
			FNCollisionQueryTestUtils::DoOverlapMulti(Settings, StartActor->GetWorld(), 
				StartActor->GetActorLocation(), StartActor->GetActorQuat());
		}
	}
}

void UNCollisionQueryTestWidget::OnSelectStartButtonClicked()
{
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(StartActor, true, true, true, true);
}

void UNCollisionQueryTestWidget::OnSelectEndButtonClicked()
{
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(EndActor, true, true, true, true);
	
}
