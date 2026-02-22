// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesDisplayActor.h"
#include "NColor.h"
#include "NSamplesDisplayBuilder.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetStringLibrary.h"

TArray<ANSamplesDisplayActor*> ANSamplesDisplayActor::KnownDisplays;

void ANSamplesDisplayActor::SortKnownDisplays()
{
	KnownDisplays.Sort([](const ANSamplesDisplayActor& A, const ANSamplesDisplayActor& B)
	{
		if (A.ScreenshotSettings.FilenameOverride.IsEmpty() == B.ScreenshotSettings.FilenameOverride.IsEmpty())
		{
			return A.ScreenshotSettings.FilenameOverride > B.ScreenshotSettings.FilenameOverride;
		}
		if (A.ScreenshotSettings.CameraNameOverride.IsEmpty() == B.ScreenshotSettings.CameraNameOverride.IsEmpty())
		{
			return A.ScreenshotSettings.CameraNameOverride.ToString() > B.ScreenshotSettings.CameraNameOverride.ToString();
		}
		return A.GetName() > B.GetName();
	});
}

ANSamplesDisplayActor::ANSamplesDisplayActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Create reference storage
	Parts = MakeUnique<FNSamplesDisplayComponents>();
	Materials = MakeUnique<FNSamplesDisplayMaterials>();
	
	Parts->SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	Parts->SceneRoot->SetMobility(EComponentMobility::Static);
	RootComponent = Parts->SceneRoot;

	// This is really just for organizational purposes and OCD.
	Parts->PartRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Parts"));
	Parts->PartRoot->SetupAttachment(Parts->SceneRoot);
	Parts->PartRoot->SetMobility(EComponentMobility::Static);
	
	// Create Materials
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialGrey(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_Grey"));
	if (MaterialGrey.Succeeded())
	{
		Materials->DisplayMaterialInterface = MaterialGrey.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialGreyDark(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_GreyDark"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialWhite(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_White"));

	// Create Components
	Parts->NoticeDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	Parts->NoticeDecalComponent->bAutoActivate = false;
	Parts->NoticeDecalComponent->SetMobility(EComponentMobility::Static);
	Parts->NoticeDecalComponent->SetupAttachment(Parts->PartRoot);
	Parts->NoticeDecalComponent->DecalSize = FVector::ZeroVector;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalMaterial(TEXT("/NexusSharedSamples/M_NSamples_Notice"));
	if (DecalMaterial.Succeeded())
	{
		Materials->NoticeMaterialInterface = DecalMaterial.Object;
	}
	Parts->NoticeTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Notice"));
	Parts->NoticeTextComponent->SetupAttachment(Parts->PartRoot);
	Parts->NoticeTextComponent->SetWorldSize(24.f);
	Parts->NoticeTextComponent->HorizontalAlignment = EHTA_Center;
	Parts->NoticeTextComponent->VerticalAlignment = EVRTA_TextCenter;
	
	Parts->TitleTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Title"));
	Parts->TitleTextComponent->SetupAttachment(Parts->PartRoot);
	
	Parts->DescriptionTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Description"));
	Parts->DescriptionTextComponent->SetupAttachment(Parts->PartRoot);

	// These get stripped on servers
	if (!GIsServer)
	{
		static ConstructorHelpers::FObjectFinder<UFont> TextFont(TEXT("/Engine/EngineFonts/RobotoDistanceField"), LOAD_NoWarn);
		if (TextFont.Succeeded())
		{
			Parts->TitleTextComponent->SetFont(TextFont.Object);
			Parts->DescriptionTextComponent->SetFont(TextFont.Object);
			Parts->NoticeTextComponent->SetFont(TextFont.Object);
		}
	
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> TextMaterial(TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque"), LOAD_NoWarn);
		if (TextMaterial.Succeeded())
		{
			Parts->TitleTextComponent->SetMaterial(0, TextMaterial.Object);
			Parts->DescriptionTextComponent->SetMaterial(0, TextMaterial.Object);
			Parts->NoticeTextComponent->SetMaterial(0, TextMaterial.Object);
		}
	}
	
	Parts->SpotlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Parts->SpotlightComponent->bAutoActivate = false;
	Parts->SpotlightComponent->SetMobility(EComponentMobility::Type::Stationary);
	Parts->SpotlightComponent->SetupAttachment(Parts->PartRoot);
	Parts->SpotlightComponent->SetIntensity(0.f);
	Parts->SpotlightComponent->SetIntensityUnits(ELightUnits::Unitless);
	Parts->SpotlightComponent->SetAttenuationRadius(0.f);
	Parts->SpotlightComponent->SetInnerConeAngle(0.f);
	Parts->SpotlightComponent->SetOuterConeAngle(0.f);
	Parts->SpotlightComponent->bUseTemperature = true;
	Parts->SpotlightComponent->SetTemperature(0.f);
	static ConstructorHelpers::FObjectFinder<UTextureLightProfile> LightProfile(TEXT("/Engine/EngineLightProfiles/180Degree_IES"));
	if (LightProfile.Succeeded())
	{
		Materials->SpotlightLightProfile = LightProfile.Object;
	}
	Parts->SpotlightComponent->SetIESTexture(nullptr);
	Parts->SpotlightComponent->bAffectsWorld = false;
	
	Parts->TitleSpacerComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TitleSpacer"));
	Parts->TitleSpacerComponent->SetupAttachment(Parts->PartRoot);
	Parts->TitleSpacerComponent->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EngineCubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (EngineCubeMesh.Succeeded())
	{
		Parts->TitleSpacerComponent->SetStaticMesh(EngineCubeMesh.Object);
		Parts->TitleSpacerComponent->SetMaterial(0, MaterialWhite.Object); // divider color
	}
	
	// Create Display
	Parts->PanelCurveEdge = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Curve_Edge"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelCurveEdge, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EdgeCurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_EdgeCurve"));
	if (EdgeCurveMesh.Succeeded())
	{
		Parts->PanelCurveEdge->SetStaticMesh(EdgeCurveMesh.Object);
		Parts->PanelCurveEdge->SetMaterial(0, MaterialGreyDark.Object);
	}
	
	Parts->PanelSide = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Side"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelSide, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SideMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Side"));
	if (SideMesh.Succeeded())
	{
		Parts->PanelSide->SetStaticMesh(SideMesh.Object);
		Parts->PanelSide->SetMaterial(1, MaterialGreyDark.Object);
	}

	Parts->PanelCurve = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Curve"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelCurve, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Curve"));
	if (CurveMesh.Succeeded())
	{
		Parts->PanelCurve->SetStaticMesh(CurveMesh.Object);
	}

	Parts->PanelCorner = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Corner"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelCorner, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CornerMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Corner"));
	if (CornerMesh.Succeeded())
	{
		Parts->PanelCorner->SetStaticMesh(CornerMesh.Object);
		Parts->PanelCorner->SetMaterial(1, MaterialGreyDark.Object);
	}

	Parts->PanelMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Main"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelMain, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MainMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Main"));
	if (MainMesh.Succeeded())
	{
		Parts->PanelMain->SetStaticMesh(MainMesh.Object);
	}

	// Create Tag Bar
	Parts->TitleBarMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_Main"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->TitleBarMain, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarMainMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_Middle"));
	if (TitleBarMainMesh.Succeeded())
	{
		Parts->TitleBarMain->SetStaticMesh(TitleBarMainMesh.Object);
		Parts->TitleBarMain->SetMaterial(0, MaterialGreyDark.Object);
	}

	Parts->TitleBarEndLeft = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_End_Left"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->TitleBarEndLeft, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_R"));
	if (TitleBarEndMesh.Succeeded())
	{
		Parts->TitleBarEndLeft->SetStaticMesh(TitleBarEndMesh.Object);
		Parts->TitleBarEndLeft->SetMaterial(0, MaterialGreyDark.Object);
	}

	Parts->TitleBarEndRight = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_End_Right"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->TitleBarEndRight, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndRMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_L"));
	if (TitleBarEndRMesh.Succeeded())
	{
		Parts->TitleBarEndRight->SetStaticMesh(TitleBarEndRMesh.Object);
		Parts->TitleBarEndRight->SetMaterial(0, MaterialGreyDark.Object);
	}

	// Create ShadowBox
	Parts->ShadowBoxSide = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Side"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->ShadowBoxSide, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side_Ribbed"));
	if (ShadowStraightMesh.Succeeded())
	{
		Parts->ShadowBoxSide->SetStaticMesh(ShadowStraightMesh.Object);
		Parts->ShadowBoxSide->SetMaterial(0, MaterialWhite.Object);
		Parts->ShadowBoxSide->SetMaterial(1, MaterialWhite.Object);
	}

	Parts->ShadowBoxRound = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Round"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->ShadowBoxRound, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowRoundMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_RoundEdge"));
	if (ShadowRoundMesh.Succeeded())
	{
		Parts->ShadowBoxRound->SetStaticMesh(ShadowRoundMesh.Object);
		Parts->ShadowBoxRound->SetMaterial(0, MaterialWhite.Object);
		Parts->ShadowBoxRound->SetMaterial(1, MaterialWhite.Object);
	}

	Parts->ShadowBoxTop = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Top"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->ShadowBoxTop, Parts->PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightTopMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side"));
	if (ShadowStraightTopMesh.Succeeded())
	{
		Parts->ShadowBoxTop->SetStaticMesh(ShadowStraightTopMesh.Object);
		Parts->ShadowBoxTop->SetMaterial(0, MaterialWhite.Object);
		Parts->ShadowBoxTop->SetMaterial(1, MaterialWhite.Object);
	}

	// Watermark Mesh
	Parts->Watermark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Branding"));
	Parts->Watermark->SetupAttachment(Parts->PartRoot);
	Parts->Watermark->SetMobility(EComponentMobility::Static);
	Parts->Watermark->SetCollisionProfileName(FName("NoCollision"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>BrandingMesh(TEXT("/NexusSharedSamples/SM_NSamples_NEXUS"));
	if (BrandingMesh.Succeeded())
	{
		Parts->Watermark->SetStaticMesh(BrandingMesh.Object);
		Parts->Watermark->SetMaterial(0, MaterialWhite.Object);
	}
	
	// Screenshot Camera
	Parts->ScreenshotCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ScreenshotCamera"));
	Parts->ScreenshotCameraComponent->SetupAttachment(RootComponent);
	Parts->ScreenshotCameraComponent->SetMobility(EComponentMobility::Movable);
	Parts->ScreenshotCameraComponent->SetRelativeLocation(FVector(755.f, 0.f, 300.f));
	Parts->ScreenshotCameraComponent->SetRelativeRotation(FRotator(-20.f, -180.f, 0.f));
	Parts->ScreenshotCameraComponent->AspectRatio = 1.777778;
	Parts->ScreenshotCameraComponent->bConstrainAspectRatio = true;
}

void ANSamplesDisplayActor::OnConstruction(const FTransform& Transform)
{
	Rebuild();
	Super::OnConstruction(Transform);
}

void ANSamplesDisplayActor::BeginDestroy()
{
	if (TestInstance != nullptr)
	{
		TestInstance->ClearBindings();
		TestInstance->RemoveFromRoot();
		TestInstance = nullptr;
	}
	
	if (Parts != nullptr)
	{
		Parts.Reset();
	}
	if (Materials != nullptr)
	{
		Materials.Reset();	
	}
	
	Super::BeginDestroy();
}

void ANSamplesDisplayActor::BeginPlay()
{
	
	// If we are running in automation, we absolutely do not want timers in other places running, so we disable them
	if (!GIsAutomationTesting && TimerSettings.bTimerEnabled)
	{
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ANSamplesDisplayActor::TimerExpired, 
			TimerSettings.TimerDuration, true, 0);
	}

	// Add to the list of known displays so we can move between them easily.
	KnownDisplays.Add(this);
	Super::BeginPlay();
}

void ANSamplesDisplayActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove me from the list.
	KnownDisplays.Remove(this);
	
	if (TimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ANSamplesDisplayActor::Rebuild()
{
	// Fix stuff created in FunctionTest constructor
	RootComponent = Parts->SceneRoot;
	
	// Clear previous instances
	Parts->ClearInstances();

	// Ensure depth
	if (BaseSettings.Depth < 0.5f)
	{
		BaseSettings.Depth = 0.5f;
	}

	// Static Instance Meshes
	FNSamplesDisplayBuilder::CreateDisplayInstances(Parts.Get(), MainPanelTransform, FloorPanelTransform, BaseSettings.Depth, BaseSettings.Width, BaseSettings.Height);
	
	FNSamplesDisplayBuilder::CreateShadowBoxInstances(Parts.Get(), BaseSettings.ShadowBoxCoverDepthPercentage, BaseSettings.Depth, BaseSettings.Width, BaseSettings.Height);
	
	if (TitleSettings.bSeparateTitlePanel)
	{
		FNSamplesDisplayBuilder::CreateTitlePanelInstances(Parts.Get(), BaseSettings.Depth, BaseSettings.Width);
	}

	// Adjust Components
	UpdateNotice();
	UpdateSpotlight();
	UpdateDisplayColor();
	UpdateTitleText();
	BuildDescription();
	UpdateDescription();
	UpdateWatermark();
	
	Parts->UpdateCollisions(BaseSettings.bCollisionEnabled);
}

void ANSamplesDisplayActor::TimerExpired()
{
	OnTimerExpired();
}

void ANSamplesDisplayActor::BuildDescription()
{
	// Headless server
	if (Parts->DescriptionTextComponent == nullptr) return;
	
	// Setup Line Spacing
	FString ParagraphSpacingMarkup = TEXT("");
	for (int i = 0; i < DescriptionSettings.ParagraphSpacing + 1; i++)
	{
		ParagraphSpacingMarkup += TEXT("<br>");
	}

	FString LineSpacingMarkup = TEXT("");
	for (int i = 0; i < DescriptionSettings.LineSpacing + 1; i++)
	{
		LineSpacingMarkup += TEXT("<br>");
	}

	// Create word and break arrays
	TArray<FString> WordArray;
	TArray<bool> BreakArray;
	for (int i = 0; i < DescriptionSettings.DescriptionText.Num(); i++)
	{
		TArray<FString> TempParagraphArray =
			UKismetStringLibrary::ParseIntoArray(DescriptionSettings.DescriptionText[i].ToString(), TEXT(" "), true);
		
		WordArray.Append(TempParagraphArray);

		// Record our forced break points for the paragraph
		for (int j = 0; j < (TempParagraphArray.Num() - 1); j++)
		{
			BreakArray.Add(false);
		}
		BreakArray.Add(true);
	}

	// Add spaces to all the words
	for (int i = 0; i < WordArray.Num(); i++)
	{
		WordArray[i] = WordArray[i].Append(" ");
	}
	
	int CharacterCount = 0;
	const int LineWidth = ((100.f * BaseSettings.Width) - 100.f) -  DescriptionSettings.DescriptionTextPadding;
	const int LineLimit = FMath::Floor( LineWidth / (DescriptionSettings.DescriptionScale * 0.45f));
	
	// Sort the words into lines
	int WordCount = WordArray.Num();
	int WordCountIndex = WordCount - 1;
	for (int i = 0; i < WordCount; i++)
	{
		// We don't want break on the last
		if (i != WordCountIndex && BreakArray[i])
		{
			WordArray[i] = WordArray[i].Append(ParagraphSpacingMarkup);
			CharacterCount = 0;
		}
		else
		{
			const int CurrentLength = WordArray[i].Len();
			const int TempLength = CharacterCount + CurrentLength;
			
			if (TempLength >= LineLimit)
			{
				WordArray[i] = WordArray[i].Append(LineSpacingMarkup);
				CharacterCount = 0;
			}
			else
			{
				CharacterCount = TempLength;
			}
		}
	}

	// Final Assembly
	FString FinalString = TEXT("");
	for (int i = 0; i < WordArray.Num(); i++)
	{
		FinalString += WordArray[i];
	}
	
	DescriptionSettings.CachedDescription = FText::AsLocalizable_Advanced(
		TEXT("NSamplesDisplayActor"),
		TitleSettings.TitleText.ToString(),
		FinalString);
}

void ANSamplesDisplayActor::UpdateDescription() const
{
	if (DescriptionSettings.CachedDescription.IsEmpty())
	{
		Parts->DescriptionTextComponent->SetVisibility(false);
		return;
	}
	
	// Set Position
	const bool bHeightVSFloorText = (BaseSettings.Height < 2.f || (BaseSettings.bFloorText && BaseSettings.Depth > 2.f));

	FVector Origin;
	if (TitleSettings.bSeparateTitlePanel)
	{
		Origin = TitleTextTransform().GetLocation();
	}
	else
	{
		Origin = Parts->TitleTextComponent->GetRelativeLocation();
	}

	const float TitleSpacerOffset = (TitleSettings.TitleScale / 2.f) + (TitleSettings.bShowTitleHorizontalRule ? 10.f : 0.f);
	
	FVector WallLocation = FVector(15.f, TextAlignmentOffset(1.f, false) - (DescriptionSettings.DescriptionTextPadding * .5f),
		Origin.Z - TitleSpacerOffset - (DescriptionSettings.DescriptionTextPadding * .5f));
	FVector FloorLocation = FVector(Origin.X - TitleSpacerOffset - (DescriptionSettings.DescriptionTextPadding * .5f), WallLocation.Y, WallLocation.X);

	if (TitleSettings.bSeparateTitlePanel)
	{
		WallLocation.Z += (TitleSpacerOffset * 1.5f);
		FloorLocation.X += TitleSpacerOffset;
	}
	
	Parts->DescriptionTextComponent->SetRelativeTransform(
		FTransform(
		FRotator(bHeightVSFloorText ? 90.f : 0.f, 0.f, 0.f),
			bHeightVSFloorText ? FloorLocation : WallLocation, 
			FVector::OneVector));
	
	Parts->DescriptionTextComponent->SetText(DescriptionSettings.CachedDescription);
	Parts->DescriptionTextComponent->SetHorizontalAlignment(BaseSettings.TextAlignment);
	Parts->DescriptionTextComponent->SetWorldSize(DescriptionSettings.DescriptionScale);
	Parts->DescriptionTextComponent->SetTextRenderColor(FNColor::GetColor(DescriptionSettings.DescriptionColor));

	if (bHeightVSFloorText)
	{
		Parts->DescriptionTextComponent->SetVerticalAlignment(EVRTA_TextBottom);
	}
	else
	{
		Parts->DescriptionTextComponent->SetVerticalAlignment(EVRTA_TextTop);
	}
	Parts->DescriptionTextComponent->SetVisibility(true);
}

void ANSamplesDisplayActor::UpdateDisplayColor()
{
	if (Materials->DisplayMaterial == nullptr)
	{
		Materials->DisplayMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Materials->DisplayMaterialInterface, NAME_None, EMIDCreationFlags::Transient);
		
		// Update the static meshes with the dynamic material
		if (Materials->DisplayMaterial != nullptr)
		{
			Parts->PanelMain->SetMaterial(0, Materials->DisplayMaterial);
			Parts->PanelCorner->SetMaterial(0, Materials->DisplayMaterial);
			Parts->PanelCurve->SetMaterial(0, Materials->DisplayMaterial);
			Parts->PanelSide->SetMaterial(0, Materials->DisplayMaterial);
			Parts->PanelCurveEdge->SetMaterial(1, Materials->DisplayMaterial);
			Parts->Watermark->SetMaterial(0, Materials->DisplayMaterial);
		}
	}
	if (Materials->DisplayMaterial != nullptr)
	{
		Materials->DisplayMaterial->SetVectorParameterValue(FName("Base Color"), FNColor::GetLinearColor(BaseSettings.Color));
	}
}

void ANSamplesDisplayActor::UpdateNotice()
{
	// Headless server
	if (Parts->NoticeTextComponent == nullptr) return;
	
	Parts->NoticeDecalComponent->SetRelativeTransform(FTransform(
			FRotator(-90.f, 0.f, 0.f),
			FVector((BaseSettings.Depth * 100.f) + NoticeSettings.NoticeDepth, 0.f , 0.f),
			FVector::OneVector));
	Parts->NoticeTextComponent->SetRelativeTransform(FTransform(
			FRotator(90.f, 0.f, 0.f),
		FVector((BaseSettings.Depth * 100.f) + NoticeSettings.NoticeDepth, 0.f , 0.5f),
	FVector::OneVector)
	);

	if (NoticeSettings.bNoticeEnabled)
	{
		if (Materials->NoticeMaterial == nullptr)
		{
			Materials->NoticeMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Materials->NoticeMaterialInterface, NAME_None, EMIDCreationFlags::Transient);
			Parts->NoticeDecalComponent->SetMaterial(0, Materials->NoticeMaterial);
		}
		if (Materials->NoticeMaterial != nullptr)
		{
			Materials->NoticeMaterial->SetVectorParameterValue(FName("Base Color"), FNColor::GetLinearColor(NoticeSettings.NoticeColor));
		}
		
		Parts->NoticeDecalComponent->SetActive(true);
		Parts->NoticeDecalComponent->DecalSize = FVector(128.f, (50.f * BaseSettings.Width), NoticeSettings.NoticeDepth);
		Parts->NoticeDecalComponent->SetVisibility(true);
		Parts->NoticeDecalComponent->SetHiddenInGame(false);

		Parts->NoticeTextComponent->SetText(NoticeSettings.NoticeText);
		Parts->NoticeTextComponent->SetWorldSize(NoticeSettings.NoticeScale);
		Parts->NoticeTextComponent->SetTextRenderColor(FNColor::GetColor(NoticeSettings.NoticeColor));
		Parts->NoticeTextComponent->SetVisibility(true);
		Parts->NoticeTextComponent->SetHiddenInGame(false);
	}
	else
	{
		
		Parts->NoticeDecalComponent->DecalSize = FVector::ZeroVector;
		Parts->NoticeDecalComponent->SetVisibility(false);
		Parts->NoticeDecalComponent->SetActive(false);
		Parts->NoticeDecalComponent->SetHiddenInGame(true);

		Parts->NoticeTextComponent->SetVisibility(false);
		Parts->NoticeTextComponent->SetHiddenInGame(true);
	}
}

void ANSamplesDisplayActor::UpdateSpotlight() const
{
	// Headless server
	if (Parts->SpotlightComponent == nullptr) return;
	
	Parts->SpotlightComponent->SetRelativeLocationAndRotation(
			FVector(FMath::Max(BaseSettings.Depth, 2.5f) * 100.f, 0.f, FMath::Max(BaseSettings.Height, 2.5f) * 100.f),
			FRotator(-135.f, 0.f , 0.f));
	
	if (SpotlightSettings.bSpotlightEnabled)
	{
		Parts->SpotlightComponent->SetActive(true);
		
		Parts->SpotlightComponent->bAffectsWorld = true;
		Parts->SpotlightComponent->SetCastShadows(SpotlightSettings.bSpotlightCastShadows);
		Parts->SpotlightComponent->SetCastVolumetricShadow(SpotlightSettings.bSpotlightCastVolumetricShadow);
		Parts->SpotlightComponent->SetIntensity(SpotlightSettings.SpotlightIntensity);
		Parts->SpotlightComponent->SetInnerConeAngle(SpotlightSettings.SpotlightInnerConeAngle);
		Parts->SpotlightComponent->SetOuterConeAngle(SpotlightSettings.SpotlightOuterConeAngle);
		Parts->SpotlightComponent->SetAttenuationRadius(SpotlightSettings.SpotlightAttenuationRadius);
		Parts->SpotlightComponent->SetTemperature(SpotlightSettings.SpotlightTemperature);

		Parts->SpotlightComponent->SetVisibility(true);
		Parts->SpotlightComponent->SetHiddenInGame(false);
		Parts->SpotlightComponent->SetIESTexture(Materials->SpotlightLightProfile);
	}
	else
	{
		Parts->SpotlightComponent->SetIntensity(0.f);
		Parts->SpotlightComponent->SetInnerConeAngle(0.f);
		Parts->SpotlightComponent->SetOuterConeAngle(0.f);
		Parts->SpotlightComponent->SetAttenuationRadius(0.f);
		Parts->SpotlightComponent->SetTemperature(0.f);
		Parts->SpotlightComponent->bAffectsWorld = false;
		
		Parts->SpotlightComponent->SetVisibility(false);
		Parts->SpotlightComponent->SetHiddenInGame(true);
		Parts->SpotlightComponent->SetIESTexture(nullptr);
		Parts->SpotlightComponent->SetActive(false);
	}
}

void ANSamplesDisplayActor::UpdateTitleText() const
{
	// Headless server
	if (Parts->TitleTextComponent == nullptr) return;
	
	// Title Text
	if (!TitleSettings.TitleText.IsEmpty())
	{
		Parts->TitleTextComponent->SetVisibility(true);
		if (TitleSettings.bSeparateTitlePanel)
		{
			
			Parts->TitleTextComponent->SetRelativeTransform(
				FTransform(
					FRotator(40.f, 0.f, 0.f),
					FVector(((BaseSettings.Depth - 1.f) * 100.f) + 88.f + TitleSettings.TitleTextOffset.Y, 
						TextAlignmentOffset(3.f, TitleSettings.bSeparateTitlePanel), 22.f) + TitleSettings.TitleTextOffset.X,
					FVector::OneVector));
			
			Parts->TitleTextComponent->SetHorizontalAlignment(EHTA_Center);
			Parts->TitleTextComponent->SetWorldSize(TitleSettings.TitleScale * .5f);
		}
		else
		{
			Parts->TitleTextComponent->SetRelativeTransform(TitleTextTransform());
			Parts->TitleTextComponent->SetHorizontalAlignment(BaseSettings.TextAlignment);
			Parts->TitleTextComponent->SetWorldSize(TitleSettings.TitleScale);
		}
		Parts->TitleTextComponent->SetText(TitleSettings.TitleText);
		Parts->TitleTextComponent->SetTextRenderColor(FNColor::GetColor(TitleSettings.TitleColor));
		Parts->TitleTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	}
	else
	{
		Parts->TitleTextComponent->SetVisibility(false);
	}

	// Title Spacer
	if (!TitleSettings.bSeparateTitlePanel)
	{
		const FVector TitleLocation = Parts->TitleTextComponent->GetRelativeLocation();
		FVector LocationOffset = FVector::ZeroVector;

		if (BaseSettings.Height < 2.f || (BaseSettings.bFloorText && BaseSettings.Depth > 2.f))
		{
			LocationOffset.X = -1.f * (TitleSettings.TitleScale / 2.f);
		}
		else
		{
			LocationOffset.Z = -1 * (TitleSettings.TitleScale / 2.f);
		}
		
		Parts->TitleSpacerComponent->SetRelativeTransform(
			FTransform(
				FRotator::ZeroRotator,
				FVector(TitleLocation.X, 0.f, TitleLocation.Z) +
				LocationOffset,
				FVector(0.01f, BaseSettings.Width - 1.f, 0.02f)
				));
		Parts->TitleSpacerComponent->SetVisibility(TitleSettings.bShowTitleHorizontalRule);
	}
	else
	{
		Parts->TitleSpacerComponent->SetVisibility(false);
	}
}


void ANSamplesDisplayActor::UpdateWatermark() const
{
	if (!WatermarkSettings.bWatermarkEnabled)
	{
		Parts->Watermark->SetVisibility(false);
		return;
	}

	Parts->Watermark->SetRelativeTransform(MainPanelTransform);
	if (BaseSettings.Height < 2.f)
	{
		FVector NewLocation = Parts->Watermark->GetRelativeLocation();
		NewLocation += FVector(BaseSettings.Depth * 50, 0, 9);
		Parts->Watermark->SetRelativeLocation(NewLocation);
		FRotator NewRotation = Parts->Watermark->GetRelativeRotation();
		NewRotation.Pitch += 180.f;
		Parts->Watermark->SetRelativeRotation(NewRotation);
	}
	else
	{
		FVector NewLocation = Parts->Watermark->GetRelativeLocation();
		NewLocation += FVector(9, 0, 100 + (BaseSettings.Height * 50));
		Parts->Watermark->SetRelativeLocation(NewLocation);
	}
	Parts->Watermark->SetWorldScale3D(FVector(1, WatermarkSettings.WatermarkScale, WatermarkSettings.WatermarkScale));
	Parts->Watermark->SetVisibility(true);
}

float ANSamplesDisplayActor::TextAlignmentOffset(const float WidthAdjustment, const bool bForceCenter) const
{
	if (bForceCenter) return 0.f;
	switch (BaseSettings.TextAlignment)
	{
	case EHTA_Center:
		return 0.f;
	case EHTA_Right:
		return (((BaseSettings.Width - WidthAdjustment) * 0.5f) * -100.f);
	case EHTA_Left:
	default:
		return (((BaseSettings.Width - WidthAdjustment) * 0.5f) * 100.f);
	}
}

FTransform ANSamplesDisplayActor::TitleTextTransform() const
{
	if (BaseSettings.Height < 2.f || (BaseSettings.bFloorText && BaseSettings.Depth > 2.f))
	{
		return FTransform(
			FRotator(90.f, 0.f, 0.f),
			FVector(BaseSettings.Depth * 100.f - (TitleSettings.TitleScale + 50.f * 0.5f) + TitleSettings.TitleTextOffset.Y, 
				TextAlignmentOffset(1.0, false) + TitleSettings.TitleTextOffset.X, 
				15.f),
			FVector::OneVector);
	}
	return FTransform(
			FRotator::ZeroRotator,
			FVector(15.f, TextAlignmentOffset(1.0, false) + TitleSettings.TitleTextOffset.Y, 
				BaseSettings.Height * 100.f - (TitleSettings.TitleScale + 50.f * 0.5f)) + TitleSettings.TitleTextOffset.X,
			FVector::OneVector);
}

void ANSamplesDisplayActor::PrepareTest()
{
	if (TestInstance != nullptr)
	{
		TestInstance->RemoveFromRoot();
	}
	TestInstance = NewObject<UNSamplesDisplayTest>(this, UNSamplesDisplayTest::StaticClass(), NAME_None, RF_Transient);
	TestInstance->AddToRoot();
	
	if (TimerSettings.bTimerEnabled && !TimerHandle.IsValid())
	{
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ANSamplesDisplayActor::TimerExpired, TimerSettings.TimerDuration, true, 0);
	}

	// Disable for test
	if (TimerSettings.bTimerEnabled && TestingSettings.bTestDisableTimer && TimerHandle.IsValid())
	{
		GetWorldTimerManager().PauseTimer(TimerHandle);
	}

	ReceivePrepareTest();
}

void ANSamplesDisplayActor::StartTest()
{
	ReceiveStartTest();
}

void ANSamplesDisplayActor::CleanupTest()
{
	ReceiveTestFinished();
	
	if (TestInstance != nullptr)
	{
		TestInstance->ClearBindings();
		TestInstance->RemoveFromRoot();
		TestInstance = nullptr;
	}

	if (!GIsAutomationTesting && TimerSettings.bTimerEnabled && TestingSettings.bTestDisableTimer && TimerHandle.IsValid())
	{
		GetWorldTimerManager().UnPauseTimer(TimerHandle);
	}
}