// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesPawn.h"

#include "HighResScreenshot.h"
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
	WidgetInteraction->InteractionSource = EWidgetInteractionSource::Mouse;
	WidgetInteraction->SetupAttachment(RootComponent);
}

void ANSamplesPawn::BeginPlay()
{
	InputComponent->BindKey(EKeys::Tab, IE_Released, this, &ANSamplesPawn::OnNextDisplay);
	InputComponent->BindKey(EKeys::LeftBracket, IE_Released, this, &ANSamplesPawn::OnPreviousDisplay);
	InputComponent->BindKey(EKeys::RightBracket, IE_Released, this, &ANSamplesPawn::OnNextDisplay);
	InputComponent->BindKey(EKeys::BackSpace, IE_Released, this, &ANSamplesPawn::OnToggleHUD);
	InputComponent->BindKey(EKeys::F12, IE_Released, this, &ANSamplesPawn::OnScreenshot);
	InputComponent->BindKey(EKeys::Backslash, IE_Released, this, &ANSamplesPawn::OnReturnToPawn);
	InputComponent->BindKey(EKeys::Hyphen, IE_Released, this, &ANSamplesPawn::OnResolutionDecrease);
	InputComponent->BindKey(EKeys::Equals, IE_Released, this, &ANSamplesPawn::OnResolutionIncrease);
	
	InputComponent->BindKey(EKeys::Slash, IE_Released, this, &ANSamplesPawn::OnSortDisplays); // Hidden
	InputComponent->BindKey(EKeys::F7, IE_Released, this, &ANSamplesPawn::OnAutoScreenshot);

	ChangeView(nullptr);
	
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

void ANSamplesPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bAutoScreenshotMode)
	{
		if (ScreenshotFrameWait > 0)
		{
			ScreenshotFrameWait--;
			return;
		}
		
		if (!bReadyToShoot)
		{
			// Change view
			ChangeView(ANSamplesDisplayActor::KnownDisplays[CurrentScreenshotIndex]);
			bReadyToShoot = true;
			
			// We need to do it next frame			
			return;
		}
		
		if (ScreenshotFrameWait > 0)
		{
			ScreenshotFrameWait--;
			return;
		}
		
		if (bReadyToShoot)
		{
			GIsHighResScreenshot = true;
			
			GScreenshotResolutionX = 1920 * ResolutionMultiplier;
			GScreenshotResolutionY = 1080 * ResolutionMultiplier;

			const ANSamplesDisplayActor* Display = ANSamplesDisplayActor::KnownDisplays[CurrentScreenshotIndex];
			if (Display->BaseSettings.AutoScreenshotFilename.IsEmpty())
			{
				GetHighResScreenshotConfig().FilenameOverride = Display->GetName();
			}
			else
			{
				GetHighResScreenshotConfig().FilenameOverride =  Display->BaseSettings.AutoScreenshotFilename;
			}
			FScreenshotRequest::RequestScreenshot(false);
			
			CurrentScreenshotIndex++;
			bReadyToShoot = false;
		}
		
		if (CurrentScreenshotIndex >= ANSamplesDisplayActor::KnownDisplays.Num())
		{
			bAutoScreenshotMode = false;
		}
		
	}
}

void ANSamplesPawn::OnNextDisplay()
{
	const int CameraCount = ANSamplesDisplayActor::KnownDisplays.Num();
	if (CameraCount == 0) return;

	CameraIndex++;
	if (CameraIndex >= CameraCount)
	{
		CameraIndex = 0;
	}
	ChangeView(ANSamplesDisplayActor::KnownDisplays[CameraIndex]);
}

void ANSamplesPawn::OnPreviousDisplay()
{
	const int CameraCount = ANSamplesDisplayActor::KnownDisplays.Num();
	if (CameraCount == 0) return;

	CameraIndex--;
	if (CameraIndex < 0)
	{
		CameraIndex = CameraCount - 1;
	}
	ChangeView(ANSamplesDisplayActor::KnownDisplays[CameraIndex]);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ANSamplesPawn::OnToggleHUD()
{
	ANSamplesHUD* HUD = Cast<ANSamplesHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->ToggleVisibility();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void ANSamplesPawn::OnScreenshot()
{
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	GIsHighResScreenshot = true;
	
	if (ResolutionMultiplier != 1)
	{
		GScreenshotResolutionX = ViewportSize.X * ResolutionMultiplier;
		GScreenshotResolutionY = ViewportSize.Y * ResolutionMultiplier;
	}
	else
	{
		GScreenshotResolutionX = ViewportSize.X;
		GScreenshotResolutionY = ViewportSize.Y;
	}
	
	FScreenshotRequest::RequestScreenshot(false);
}

void ANSamplesPawn::OnReturnToPawn()
{
	ChangeView(nullptr);
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

void ANSamplesPawn::OnSortDisplays()
{
	ANSamplesDisplayActor::SortKnownDisplays();	
}

void ANSamplesPawn::OnAutoScreenshot()
{
	if (ANSamplesDisplayActor::KnownDisplays.Num() == 0) return;
	
	OnSortDisplays();
	
	ANSamplesHUD* HUD = Cast<ANSamplesHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	HUD->SetVisibility(false);
	
	CameraIndex = 0;
	ScreenshotFrameWait = 60;
	bAutoScreenshotMode = true;
	bReadyToShoot = false;
	CurrentScreenshotIndex = 0;
}

void ANSamplesPawn::ChangeView(ANSamplesDisplayActor* DisplayActor)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr) return;
	ANSamplesHUD* HUD = Cast<ANSamplesHUD>(PlayerController->GetHUD());
	
	// No display, go back to pawn
	if (DisplayActor == nullptr)
	{
		PlayerController->SetViewTargetWithBlend(this);
		HUD->SetCurrentCameraName(TEXT("Pawn"));
		return;
	}
	
	PlayerController->SetViewTargetWithBlend(DisplayActor);
	if (!DisplayActor->ScreenshotSettings.ScreenshotCameraName.IsEmpty())
	{
		HUD->SetCurrentCameraName(DisplayActor->ScreenshotSettings.ScreenshotCameraName.ToString());
	}
	else
	{
		HUD->SetCurrentCameraName(DisplayActor->GetName());
	}
}
