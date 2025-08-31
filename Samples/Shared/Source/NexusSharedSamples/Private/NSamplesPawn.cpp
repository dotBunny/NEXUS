// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesPawn.h"

#include "NSamplesDisplayActor.h"
#include "NSamplesHUD.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/GameplayStatics.h"

ANSamplesPawn::ANSamplesPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Interaction"));
	WidgetInteraction->bTickInEditor = true;
	WidgetInteraction->bShowDebug = false;
	WidgetInteraction->InteractionDistance = 1000.0;
	WidgetInteraction->InteractionSource = EWidgetInteractionSource::Mouse; // TODO: We could change this if using Controller?
	WidgetInteraction->SetupAttachment(RootComponent);
}

void ANSamplesPawn::BeginPlay()
{
	InputComponent->BindKey(EKeys::Tab, IE_Released, this, &ANSamplesPawn::OnNextCamera);
	InputComponent->BindKey(EKeys::LeftBracket, IE_Released, this, &ANSamplesPawn::OnPreviousCamera);
	InputComponent->BindKey(EKeys::RightBracket, IE_Released, this, &ANSamplesPawn::OnNextCamera);
	InputComponent->BindKey(EKeys::BackSpace, IE_Released, this, &ANSamplesPawn::OnToggleHUD);
	InputComponent->BindKey(EKeys::F12, IE_Released, this, &ANSamplesPawn::OnScreenshot);
	InputComponent->BindKey(EKeys::Backslash, IE_Released, this, &ANSamplesPawn::OnReturnToPawn);
	InputComponent->BindKey(EKeys::Hyphen, IE_Released, this, &ANSamplesPawn::OnResolutionDecrease);
	InputComponent->BindKey(EKeys::Equals, IE_Released, this, &ANSamplesPawn::OnResolutionIncrease);
	
	Super::BeginPlay();
}

void ANSamplesPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InputComponent != nullptr)
	{
		InputComponent->ClearBindingsForObject(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ANSamplesPawn::OnNextCamera()
{
	const int CameraCount = ANSamplesDisplayActor::KnownDisplays.Num();
	if (CameraCount == 0) return;

	CameraIndex++;
	if (CameraIndex >= CameraCount)
	{
		CameraIndex = 0;
	}
	ANSamplesDisplayActor* TargetCamera = ANSamplesDisplayActor::KnownDisplays[CameraIndex];
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr) return;

	PlayerController->SetViewTargetWithBlend(TargetCamera);

}

void ANSamplesPawn::OnPreviousCamera()
{
	const int CameraCount = ANSamplesDisplayActor::KnownDisplays.Num();
	if (CameraCount == 0) return;

	CameraIndex--;
	if (CameraIndex < 0)
	{
		CameraIndex = CameraCount - 1;
	}
	ANSamplesDisplayActor* TargetCamera = ANSamplesDisplayActor::KnownDisplays[CameraIndex];
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr) return;

	PlayerController->SetViewTargetWithBlend(TargetCamera);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ANSamplesPawn::OnToggleHUD()
{
	ANSamplesHUD* HUD = Cast<ANSamplesHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->ToggleVisibility();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ANSamplesPawn::OnScreenshot()
{

	const FString Folder = *(FPaths::ProjectSavedDir() + TEXT("Screenshots/NEXUS/"));
	IFileManager::Get().MakeDirectory(*Folder);

	FDateTime DateTime = FDateTime::Now();
	const FString TimeStamp = FString::Printf(TEXT("%04d%02d%02d_%02d%02d%02d"),
		DateTime.GetYear(),
		DateTime.GetMonth(),
		DateTime.GetDay(),
		DateTime.GetHour(),
		DateTime.GetMinute(),
		DateTime.GetSecond()
	);
	const FString FilePath = Folder + TEXT("Screenshot_") + TimeStamp + TEXT(".png");

	// TODO Set resolution size based on current ?


	
	FScreenshotRequest::RequestScreenshot(FilePath, false, false);
// GIsHighResScreenshot = true;
// GScreenshotResolutionX = 3000;
// GScreenshotResolutionY = 2000;

}

void ANSamplesPawn::OnReturnToPawn()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr) return;
	PlayerController->SetViewTargetWithBlend(this);
}

void ANSamplesPawn::OnResolutionIncrease()
{
	ResolutionMultiplier++;
	ANSamplesHUD* HUD = Cast<ANSamplesHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->SetScreenshotMultiplier(ResolutionMultiplier);
}

void ANSamplesPawn::OnResolutionDecrease()
{
	ResolutionMultiplier--;
	if (ResolutionMultiplier < 1) ResolutionMultiplier = 1;
	ANSamplesHUD* HUD = Cast<ANSamplesHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->SetScreenshotMultiplier(ResolutionMultiplier);

	
}
