// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "CollisionQueryTest/NCollisionQueryTestWidget.h"

#include "JsonObjectConverter.h"
#include "NUIEditorMinimal.h"
#include "CollisionQueryTest/NCollisionQueryTestUtils.h"
#include "Selection.h"


void UNCollisionQueryTestWidget::NativeConstruct()
{
	StateIdentifier = NEXUS::UIEditor::CollisionQueryTest::Identifier;
	bIsPersistent = true;
	
	Super::NativeConstruct();
	
	// Bindings
	SelectStartButton->OnClicked.AddDynamic(this,&UNCollisionQueryTestWidget::SelectStartPoint);
	SelectEndButton->OnClicked.AddDynamic(this, &UNCollisionQueryTestWidget::SelectEndPoint);
	
	ObjectDetails->SetObject(this);
	ObjectDetails->bExpandedInDesigner = true;
	ObjectDetails->GetOnPropertyValueChangedRef()->AddDynamic(this, &UNCollisionQueryTestWidget::OnPropertyValueChanged);
	
	OnPIEStartedHandle = FWorldDelegates::OnPIEStarted.AddUObject(this, &UNCollisionQueryTestWidget::OnPIEStarted);
	OnPIEReadyHandle = FWorldDelegates::OnPIEReady.AddUObject(this, &UNCollisionQueryTestWidget::OnPIEReady);
	OnPIEEndedHandle = FWorldDelegates::OnPIEEnded.AddUObject(this, &UNCollisionQueryTestWidget::OnPIEEnded);
	
	CreateActor();
}

void UNCollisionQueryTestWidget::NativeDestruct()
{
	DestroyActor();
	
	FWorldDelegates::OnPIEStarted.Remove(OnPIEStartedHandle);
	FWorldDelegates::OnPIEReady.Remove(OnPIEReadyHandle);
	FWorldDelegates::OnPIEEnded.Remove(OnPIEEndedHandle);
	
	Super::NativeDestruct();
}

void UNCollisionQueryTestWidget::RestoreWidgetState(UObject* BlueprintWidget, FName Identifier, FNWidgetState& InState)
{
	UE_LOG(LogTemp, Warning, TEXT("RESTORING STATE"));
	if (InState.HasString("Settings"))
	{
		FNCollisionQueryTestSettings OutSettings;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(
			InState.GetString("Settings"), &OutSettings, 0, 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("LOADED SETTINGS"));
			Settings = OutSettings;
		}
	}
}

FNWidgetState UNCollisionQueryTestWidget::GetWidgetState(UObject* BlueprintWidget)
{
	FNWidgetState State;
	FString JsonOutput;
	if (FJsonObjectConverter::UStructToJsonObjectString(Settings, JsonOutput, 0, 0, 0, nullptr, false))
	{
		State.AddString(TEXT("Settings"), JsonOutput);	
	}
	return State;
}

void UNCollisionQueryTestWidget::OnPropertyValueChanged(FName Name)
{
	if (DoesPropertyAffectActor(Name))
	{
		PushSettings(QueryActor);
	}
}

void UNCollisionQueryTestWidget::OnWorldTick(const ANCollisionQueryTestActor* Actor)
{
	// Update Settings (for the one we know about)
	UpdateSettings(Actor);
	
	// Draw Actor w/ Settings
	using enum ECollisionQueryTestMethod;
	if (Settings.Query.QueryMethod == LineTrace)
	{
		using enum ECollisionQueryTestPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionQueryTestUtils::DoLineTraceSingle(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionQueryTestUtils::DoLineTraceMulti(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionQueryTestUtils::DoLineTraceTest(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition());
		}
	}
	else if (Settings.Query.QueryMethod == Sweep)
	{
		using enum ECollisionQueryTestPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionQueryTestUtils::DoSweepSingle(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition(), 
				Actor->GetRotation());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionQueryTestUtils::DoSweepMulti(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition(), 
				Actor->GetRotation());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionQueryTestUtils::DoSweepTest(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition(), 
				Actor->GetRotation());
		}
	}
	else if (Settings.Query.QueryMethod == Overlap)
	{
		using enum ECollisionQueryTestOverlapBlocking;
		if (Settings.Query.QueryOverlapBlocking == Blocking)
		{
			FNCollisionQueryTestUtils::DoOverlapBlocking(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetRotation());
		}
		else if (Settings.Query.QueryOverlapBlocking == Any)
		{
			FNCollisionQueryTestUtils::DoOverlapAny(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetRotation());
		}
		else if (Settings.Query.QueryOverlapBlocking == Multi)
		{
			FNCollisionQueryTestUtils::DoOverlapMulti(Settings, QueryActor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetRotation());
		}
	}
}

void UNCollisionQueryTestWidget::PushSettings(ANCollisionQueryTestActor* Actor) const
{
	if (Actor != nullptr)
	{
		Actor->GetStartComponent()->SetWorldLocation(Settings.Points.StartPoint);
		Actor->GetEndComponent()->SetRelativeLocation(Settings.Points.EndPoint);
		Actor->GetStartComponent()->SetWorldRotation(Settings.Points.Rotation.Quaternion());
		Actor->SetActorTickInterval(Settings.Options.UpdateTimer);
	}
}

void UNCollisionQueryTestWidget::UpdateSettings(const ANCollisionQueryTestActor* Actor)
{
	// Update Settings (for the one we know about)
	if (Actor == QueryActor)
	{
		Settings.Points.StartPoint = Actor->GetStartPosition();
		Settings.Points.EndPoint = Actor->GetRelativeEndPosition();
		Settings.Points.Rotation = Actor->GetRotation().Rotator();
	}
}

void UNCollisionQueryTestWidget::OnPIEStarted(UGameInstance* GameInstance)
{
	DestroyActor();
}

void UNCollisionQueryTestWidget::OnPIEReady(UGameInstance* GameInstance)
{
	CreateActor();
}

void UNCollisionQueryTestWidget::OnPIEEnded(UGameInstance* GameInstance)
{
	DestroyActor();
	CreateActor();
}

void UNCollisionQueryTestWidget::SelectStartPoint()
{
	if (QueryActor != nullptr)
	{
		USelection* ActorSelection = GEditor->GetSelectedActors();
		ActorSelection->Modify();
		ActorSelection->DeselectAll();
		
		USelection* ComponentSelection = GEditor->GetSelectedComponents();
		ComponentSelection->Modify();
		ComponentSelection->DeselectAll();
		
		GEditor->SelectComponent(QueryActor->GetStartComponent(), true, true, true);
		GEditor->SelectActor(QueryActor, true, true, true, true);
	}
}

void UNCollisionQueryTestWidget::SelectEndPoint()
{
	if (QueryActor != nullptr)
	{
		USelection* ActorSelection = GEditor->GetSelectedActors();
		ActorSelection->Modify();
		ActorSelection->DeselectAll();
		
		USelection* ComponentSelection = GEditor->GetSelectedComponents();
		ComponentSelection->Modify();
		ComponentSelection->DeselectAll();
		
		GEditor->SelectComponent(QueryActor->GetEndComponent(), true, true, true);
		GEditor->SelectActor(QueryActor, true, true, true, true);
	}
}

void UNCollisionQueryTestWidget::CreateActor(UWorld* TargetWorld)
{
	if (QueryActor == nullptr)
	{
		UWorld* World = nullptr;
		if (TargetWorld != nullptr) World = TargetWorld;
		if (World == nullptr) World = GEngine->GetCurrentPlayWorld();
		if (World == nullptr) World = GetWorld();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.bDeferConstruction = false;
		SpawnParams.InitialActorLabel = TEXT("NCollisionQueryTest");
		SpawnParams.Name = MakeUniqueObjectName(World, ANCollisionQueryTestActor::StaticClass(), TEXT("NCollisionQueryTest"));
		
		QueryActor = World->SpawnActor<ANCollisionQueryTestActor>(
			Settings.Points.StartPoint, Settings.Points.Rotation, SpawnParams);
		
		
		QueryActor->SetActorTickInterval(Settings.Options.UpdateTimer);
		
		QueryActor->AddToRoot();
		
		
		// Move our end point to where we think it should be
		QueryActor->GetEndComponent()->SetRelativeLocation(Settings.Points.EndPoint);
		
		QueryActor->Widget = this;

		const FString ActorLabel = FString::Printf(TEXT("%s / %s (%s)"), *QueryActor->GetActorLabel(), *QueryActor->GetName(), *QueryActor->GetWorld()->GetName());
		ActorNameText->SetText(FText::FromString(ActorLabel));
		
		SelectStartPoint();
	}
}

void UNCollisionQueryTestWidget::DestroyActor()
{
	if (QueryActor != nullptr && QueryActor->IsValidLowLevel())
	{
		QueryActor->Widget = nullptr;
		
		QueryActor->RemoveFromRoot();
		
		USelection* ComponentSelection = GEditor->GetSelectedComponents();
		ComponentSelection->Deselect(QueryActor->GetStartComponent());
		ComponentSelection->Deselect(QueryActor->GetEndComponent());
		
		GEditor->GetSelectedActors()->Deselect(QueryActor);
		
		QueryActor->Destroy(true, false);
	}
	QueryActor = nullptr;
}

bool UNCollisionQueryTestWidget::DoesPropertyAffectActor(FName Name)
{
	return	Name == TEXT("X") || Name == TEXT("Y") || Name == TEXT("Z") ||
			Name == TEXT("Pitch") || Name == TEXT("Yaw") || Name == TEXT("Roll") ||
			Name == TEXT("StartPoint") || Name == TEXT("EndPoint") || Name == TEXT("Rotation") ||
			Name == TEXT("UpdateTimer") || Name == TEXT("Points") || 
			Name == TEXT("Options") || Name == TEXT("Settings");
}

