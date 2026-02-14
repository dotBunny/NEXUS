// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "CollisionVisualizer/NCollisionVisualizerWidget.h"

#include "JsonObjectConverter.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "NToolingEditorMinimal.h"
#include "CollisionVisualizer/NCollisionVisualizerUtils.h"
#include "Selection.h"
#include "Macros/NFlagsMacros.h"

void UNCollisionVisualizerWidget::NativeConstruct()
{
	UniqueIdentifier = NEXUS::ToolingEditor::CollisionVisualizer::Identifier;
	bIsPersistent = true;
	
	Super::NativeConstruct();
	
	RestoreState();
	
	// Bindings
	SelectStartButton->OnClicked.AddDynamic(this,&UNCollisionVisualizerWidget::SelectStartPoint);
	SelectEndButton->OnClicked.AddDynamic(this, &UNCollisionVisualizerWidget::SelectEndPoint);
	
	ObjectDetails->SetObject(this);
	ObjectDetails->bExpandedInDesigner = true;
	ObjectDetails->GetOnPropertyValueChangedRef()->AddDynamic(this, &UNCollisionVisualizerWidget::OnPropertyValueChanged);
	
	OnPIEStartedHandle = FWorldDelegates::OnPIEStarted.AddUObject(this, &UNCollisionVisualizerWidget::OnPIEStarted);
	OnPIEReadyHandle = FWorldDelegates::OnPIEReady.AddUObject(this, &UNCollisionVisualizerWidget::OnPIEReady);
	OnPIEEndedHandle = FWorldDelegates::OnPIEEnded.AddUObject(this, &UNCollisionVisualizerWidget::OnPIEEnded);
	
	CreateActor();
}

void UNCollisionVisualizerWidget::NativeDestruct()
{	
	DestroyActor();
	
	FWorldDelegates::OnPIEStarted.Remove(OnPIEStartedHandle);
	FWorldDelegates::OnPIEReady.Remove(OnPIEReadyHandle);
	FWorldDelegates::OnPIEEnded.Remove(OnPIEEndedHandle);
	
	Super::NativeDestruct();
}


void UNCollisionVisualizerWidget::OnPropertyValueChanged(FName Name)
{
	if (DoesPropertyAffectActor(Name))
	{
		PushSettings(QueryActor);
	}
	SaveState();
}

void UNCollisionVisualizerWidget::OnWorldTick(const ANCollisionVisualizerActor* Actor)
{
	// Update Settings (for the one we know about)
	UpdateSettings(Actor);
	
	// Draw Actor w/ Settings
	using enum ECollisionVisualizerMethod;
	if (Settings.Query.QueryMethod == LineTrace)
	{
		using enum ECollisionVisualizerPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionVisualizerUtils::DoLineTraceSingle(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionVisualizerUtils::DoLineTraceMulti(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionVisualizerUtils::DoLineTraceTest(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition());
		}
	}
	else if (Settings.Query.QueryMethod == Sweep)
	{
		using enum ECollisionVisualizerPrefix;
		if (Settings.Query.QueryPrefix == Single)
		{
			FNCollisionVisualizerUtils::DoSweepSingle(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition(), 
				Actor->GetRotation());
		}
		else if (Settings.Query.QueryPrefix == Multi)
		{
			FNCollisionVisualizerUtils::DoSweepMulti(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition(), 
				Actor->GetRotation());
		}
		else if (Settings.Query.QueryPrefix == Test)
		{
			FNCollisionVisualizerUtils::DoSweepTest(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetEndPosition(), 
				Actor->GetRotation());
		}
	}
	else if (Settings.Query.QueryMethod == Overlap)
	{
		using enum ECollisionVisualizerOverlapBlocking;
		if (Settings.Query.QueryOverlapBlocking == Blocking)
		{
			FNCollisionVisualizerUtils::DoOverlapBlocking(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetRotation());
		}
		else if (Settings.Query.QueryOverlapBlocking == Any)
		{
			FNCollisionVisualizerUtils::DoOverlapAny(Settings, Actor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetRotation());
		}
		else if (Settings.Query.QueryOverlapBlocking == Multi)
		{
			FNCollisionVisualizerUtils::DoOverlapMulti(Settings, QueryActor->GetWorld(), 
				Actor->GetStartPosition(), Actor->GetRotation());
		}
	}
}

void UNCollisionVisualizerWidget::PushSettings(ANCollisionVisualizerActor* Actor) const
{
	if (Actor != nullptr)
	{
		Actor->GetStartComponent()->SetWorldLocation(Settings.Points.StartPoint);
		Actor->GetEndComponent()->SetRelativeLocation(Settings.Points.EndPoint);
		Actor->GetStartComponent()->SetWorldRotation(Settings.Points.Rotation.Quaternion());
		
		Actor->SetActorTickInterval(Settings.Drawing.DrawTimer);
		
		Actor->SetTickInEditor(N_FLAGS_HAS_UINT8(Settings.Drawing.DrawMode, ECollisionVisualizerDrawMode::EditorOnly));
		Actor->SetTickInGame(N_FLAGS_HAS_UINT8(Settings.Drawing.DrawMode, ECollisionVisualizerDrawMode::PlayInEditor));
		Actor->SetTickInSimulation(N_FLAGS_HAS_UINT8(Settings.Drawing.DrawMode, ECollisionVisualizerDrawMode::SimulateInEditor));
	}
}

void UNCollisionVisualizerWidget::UpdateSettings(const ANCollisionVisualizerActor* Actor)
{
	// Update Settings (for the one we know about)
	bool bFoundChanged = false;
	if (Actor == QueryActor)
	{
		const FVector ActorStart = Actor->GetStartPosition();
		if (Settings.Points.StartPoint != ActorStart)
		{
			Settings.Points.StartPoint = ActorStart;
			bFoundChanged = true;
		}

		const FVector ActorEnd = Actor->GetRelativeEndPosition();
		if (Settings.Points.EndPoint != ActorEnd)
		{
			Settings.Points.EndPoint = ActorEnd;
			bFoundChanged = true;
		}
		
		const FRotator ActorRotation = Actor->GetRotation().Rotator();
		if (Settings.Points.Rotation != ActorRotation)
		{
			Settings.Points.Rotation = ActorRotation;
			bFoundChanged = true;
		}
	}

	if (bFoundChanged)
	{
		SaveState();
	}
}

void UNCollisionVisualizerWidget::OnPIEStarted(UGameInstance* GameInstance)
{
	DestroyActor();
}

void UNCollisionVisualizerWidget::OnPIEReady(UGameInstance* GameInstance)
{
	CreateActor();
}

void UNCollisionVisualizerWidget::OnPIEEnded(UGameInstance* GameInstance)
{
	DestroyActor();
	CreateActor();
}

void UNCollisionVisualizerWidget::SelectStartPoint()
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

void UNCollisionVisualizerWidget::SelectEndPoint()
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

void UNCollisionVisualizerWidget::RestoreState()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System != nullptr && System->HasWidgetState(GetUniqueIdentifier()))
	{
		FNCollisionVisualizerSettings OutSettings;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(
			System->GetWidgetState(GetUniqueIdentifier()).GetString("Settings"), &OutSettings, 0, 0))
		{
			Settings = OutSettings;
		}
	}
}

void UNCollisionVisualizerWidget::SaveState() const
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System)
	{
		FString JsonOutput;
		if (FJsonObjectConverter::UStructToJsonObjectString(Settings, JsonOutput, 0, 0, 0, nullptr, false))
		{
			FNWidgetState State;
			State.AddString(TEXT("Settings"), JsonOutput);	
			System->AddWidgetState(GetUniqueIdentifier(), State);
		}
	}
}

void UNCollisionVisualizerWidget::CreateActor(UWorld* TargetWorld)
{
	if (QueryActor == nullptr)
	{
		UWorld* World = nullptr;
		if (TargetWorld != nullptr) World = TargetWorld;
		if (World == nullptr) World = GEngine->GetCurrentPlayWorld();
		if (World == nullptr) World = GetWorld();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.bDeferConstruction = false;
		SpawnParams.InitialActorLabel = TEXT("NCollisionVisualizer");
		SpawnParams.Name = MakeUniqueObjectName(World, ANCollisionVisualizerActor::StaticClass(), TEXT("NCollisionVisualizer"));
		
		QueryActor = World->SpawnActor<ANCollisionVisualizerActor>(
			Settings.Points.StartPoint, Settings.Points.Rotation, SpawnParams);
		QueryActor->SetFlags(RF_Transient);
		
		PushSettings(QueryActor);
		
		QueryActor->Widget = this;

		const FString ActorLabel = FString::Printf(TEXT("%s / %s (%s)"), *QueryActor->GetActorLabel(), *QueryActor->GetName(), *QueryActor->GetWorld()->GetName());
		ActorNameText->SetText(FText::FromString(ActorLabel));
		
		SelectStartPoint();
	}
}

void UNCollisionVisualizerWidget::DestroyActor()
{
	if (QueryActor != nullptr)
	{
		QueryActor->Widget = nullptr;
		
		USelection* ComponentSelection = GEditor->GetSelectedComponents();
		ComponentSelection->Deselect(QueryActor->GetStartComponent());
		ComponentSelection->Deselect(QueryActor->GetEndComponent());
		
		GEditor->GetSelectedActors()->Deselect(QueryActor);

		QueryActor->Destroy(true, false);
	}
	QueryActor = nullptr;
}

bool UNCollisionVisualizerWidget::DoesPropertyAffectActor(FName Name)
{
	return	Name == TEXT("X") || Name == TEXT("Y") || Name == TEXT("Z") ||
			Name == TEXT("Pitch") || Name == TEXT("Yaw") || Name == TEXT("Roll") || 
			Name == TEXT("Rotation") || 
			Name == TEXT("StartPoint") || Name == TEXT("EndPoint") || Name == TEXT("Points") || 
			Name == TEXT("Drawing") || Name == TEXT("DrawMode") || Name == TEXT("DrawTimer") ||
			Name == TEXT("Settings");
}

