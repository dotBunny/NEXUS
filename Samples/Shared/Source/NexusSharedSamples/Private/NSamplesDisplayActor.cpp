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

ANSamplesDisplayActor::ANSamplesDisplayActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	RootComponent = SceneRoot;

	// This is really just for organizational purposes and OCD.
	PartRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Parts"));
	PartRoot->SetupAttachment(SceneRoot);
	PartRoot->SetMobility(EComponentMobility::Static);
	
	Parts = MakeUnique<FNSamplesDisplayComponents>();
	Materials = MakeUnique<FNSamplesDisplayMaterials>();

	
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
	Parts->NoticeDecalComponent->SetupAttachment(PartRoot);
	Parts->NoticeDecalComponent->DecalSize = FVector::ZeroVector;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalMaterial(TEXT("/NexusSharedSamples/M_NSamples_Notice"));
	if (DecalMaterial.Succeeded())
	{
		Materials->NoticeMaterialInterface = DecalMaterial.Object;
	}
	Parts->NoticeTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Notice"));
	Parts->NoticeTextComponent->SetupAttachment(PartRoot);
	Parts->NoticeTextComponent->SetWorldSize(24.f);
	Parts->NoticeTextComponent->HorizontalAlignment = EHTA_Center;
	Parts->NoticeTextComponent->VerticalAlignment = EVRTA_TextCenter;
	
	Parts->TitleTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Title"));
	Parts->TitleTextComponent->SetupAttachment(PartRoot);
	
	Parts->DescriptionTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Description"));
	Parts->DescriptionTextComponent->SetupAttachment(PartRoot);

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
	Parts->SpotlightComponent->SetupAttachment(PartRoot);
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
		SpotlightLightProfile = LightProfile.Object;
	}
	Parts->SpotlightComponent->SetIESTexture(nullptr);
	Parts->SpotlightComponent->bAffectsWorld = false;
	// Turn off the sprite?
	
	Parts->TitleSpacerComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TitleSpacer"));
	Parts->TitleSpacerComponent->SetupAttachment(PartRoot);
	Parts->TitleSpacerComponent->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EngineCubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (EngineCubeMesh.Succeeded())
	{
		Parts->TitleSpacerComponent->SetStaticMesh(EngineCubeMesh.Object);
		Parts->TitleSpacerComponent->SetMaterial(0, MaterialWhite.Object); // divider color
	}
	
	// Create Display
	Parts->PanelCurveEdge = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Curve_Edge"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelCurveEdge, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EdgeCurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_EdgeCurve"));
	if (EdgeCurveMesh.Succeeded())
	{
		Parts->PanelCurveEdge->SetStaticMesh(EdgeCurveMesh.Object);
		Parts->PanelCurveEdge->SetMaterial(0, MaterialGreyDark.Object);
	}
	
	Parts->PanelSide = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Side"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelSide, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SideMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Side"));
	if (SideMesh.Succeeded())
	{
		Parts->PanelSide->SetStaticMesh(SideMesh.Object);
		Parts->PanelSide->SetMaterial(1, MaterialGreyDark.Object);
	}

	Parts->PanelCurve = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Curve"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelCurve, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Curve"));
	if (CurveMesh.Succeeded())
	{
		Parts->PanelCurve->SetStaticMesh(CurveMesh.Object);
	}

	Parts->PanelCorner = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Corner"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelCorner, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CornerMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Corner"));
	if (CornerMesh.Succeeded())
	{
		Parts->PanelCorner->SetStaticMesh(CornerMesh.Object);
		Parts->PanelCorner->SetMaterial(1, MaterialGreyDark.Object);
	}

	Parts->PanelMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Main"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->PanelMain, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MainMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Main"));
	if (MainMesh.Succeeded())
	{
		Parts->PanelMain->SetStaticMesh(MainMesh.Object);
	}

	// Create Tag Bar
	Parts->TitleBarMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_Main"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->TitleBarMain, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarMainMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_Middle"));
	if (TitleBarMainMesh.Succeeded())
	{
		Parts->TitleBarMain->SetStaticMesh(TitleBarMainMesh.Object);
		Parts->TitleBarMain->SetMaterial(0, MaterialGreyDark.Object);
	}

	Parts->TitleBarEndLeft = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_End_Left"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->TitleBarEndLeft, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_R"));
	if (TitleBarEndMesh.Succeeded())
	{
		Parts->TitleBarEndLeft->SetStaticMesh(TitleBarEndMesh.Object);
		Parts->TitleBarEndLeft->SetMaterial(0, MaterialGreyDark.Object);
	}

	Parts->TitleBarEndRight = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_End_Right"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->TitleBarEndRight, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndRMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_L"));
	if (TitleBarEndRMesh.Succeeded())
	{
		Parts->TitleBarEndRight->SetStaticMesh(TitleBarEndRMesh.Object);
		Parts->TitleBarEndRight->SetMaterial(0, MaterialGreyDark.Object);
	}

	// Create ShadowBox
	Parts->ShadowBoxSide = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Side"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->ShadowBoxSide, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side_Ribbed"));
	if (ShadowStraightMesh.Succeeded())
	{
		Parts->ShadowBoxSide->SetStaticMesh(ShadowStraightMesh.Object);
		Parts->ShadowBoxSide->SetMaterial(0, MaterialWhite.Object);
		Parts->ShadowBoxSide->SetMaterial(1, MaterialWhite.Object);
	}

	Parts->ShadowBoxRound = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Round"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->ShadowBoxRound, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowRoundMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_RoundEdge"));
	if (ShadowRoundMesh.Succeeded())
	{
		Parts->ShadowBoxRound->SetStaticMesh(ShadowRoundMesh.Object);
		Parts->ShadowBoxRound->SetMaterial(0, MaterialWhite.Object);
		Parts->ShadowBoxRound->SetMaterial(1, MaterialWhite.Object);
	}

	Parts->ShadowBoxTop = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Top"));
	FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(Parts->ShadowBoxTop, PartRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightTopMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side"));
	if (ShadowStraightTopMesh.Succeeded())
	{
		Parts->ShadowBoxTop->SetStaticMesh(ShadowStraightTopMesh.Object);
		Parts->ShadowBoxTop->SetMaterial(0, MaterialWhite.Object);
		Parts->ShadowBoxTop->SetMaterial(1, MaterialWhite.Object);
	}

	// Watermark Mesh
	Parts->Watermark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Branding"));
	Parts->Watermark->SetupAttachment(PartRoot);
	Parts->Watermark->SetMobility(EComponentMobility::Static);
	Parts->Watermark->SetCollisionProfileName(FName("NoCollision"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>BrandingMesh(TEXT("/NexusSharedSamples/SM_NSamples_NEXUS"));
	if (BrandingMesh.Succeeded())
	{
		Parts->Watermark->SetStaticMesh(BrandingMesh.Object);
		Parts->Watermark->SetMaterial(0, MaterialWhite.Object);
	}
	
	// Screenshot Camera
	ScreenshotCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ScreenshotCamera"));
	ScreenshotCameraComponent->SetupAttachment(RootComponent);
	ScreenshotCameraComponent->SetMobility(EComponentMobility::Movable);
	ScreenshotCameraComponent->SetRelativeLocation(FVector(755.f, 0.f, 300.f));
	ScreenshotCameraComponent->SetRelativeRotation(FRotator(-20.f, -180.f, 0.f));
	ScreenshotCameraComponent->AspectRatio = 1.777778;
	ScreenshotCameraComponent->bConstrainAspectRatio = true;
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
		Parts->ClearInstances();
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
	if (bTimerEnabled)
	{
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ANSamplesDisplayActor::TimerExpired, TimerDuration, true, 0);
	}

	// Add to list of known displays so we can move between them easily.
	KnownDisplays.Add(this);
	Super::BeginPlay();
}

void ANSamplesDisplayActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove me from the list.
	KnownDisplays.Remove(this);
	
	Super::EndPlay(EndPlayReason);
}

void ANSamplesDisplayActor::Rebuild()
{
	// Fix stuff created in FunctionTest constructor
	RootComponent = SceneRoot;
	
	// Clear previous instances
	Parts->ClearInstances();

	// Ensure depth
	if (Depth < 0.5f)
	{
		Depth = 0.5f;
	}

	// Static Instance Meshes
	FNSamplesDisplayBuilder::CreateDisplayInstances(Parts->PanelCurve, Parts->PanelCurveEdge, Parts->PanelMain, Parts->PanelCorner, 
		Parts->PanelSide, MainPanelTransform, FloorPanelTransform, Depth, Width, Height);
	
	FNSamplesDisplayBuilder::CreateShadowBoxInstances(Parts->ShadowBoxSide, Parts->ShadowBoxTop, Parts->ShadowBoxRound, 
		ShadowBoxCoverDepthPercentage, Depth, Width, Height);
	
	if (bSeparateTitlePanel)
	{
		FNSamplesDisplayBuilder::CreateTitlePanelInstances(Parts->TitleBarMain, Parts->TitleBarEndLeft, Parts->TitleBarEndRight, 
			Depth, Width);
	}

	// Adjust Components
	UpdateNotice();
	UpdateSpotlight();
	UpdateDisplayColor();
	UpdateTitleText();
	BuildDescription();
	UpdateDescription();
	UpdateWatermark();
	
	Parts->UpdateCollisions(bCollisionEnabled);
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
	for (int i = 0; i < ParagraphSpacing + 1; i++)
	{
		ParagraphSpacingMarkup += TEXT("<br>");
	}

	FString LineSpacingMarkup = TEXT("");
	for (int i = 0; i < LineSpacing + 1; i++)
	{
		LineSpacingMarkup += TEXT("<br>");
	}

	// Create word and break arrays
	TArray<FString> WordArray;
	TArray<bool> BreakArray;
	for (int i = 0; i < DescriptionText.Num(); i++)
	{
		TArray<FString> TempParagraphArray =
			UKismetStringLibrary::ParseIntoArray(DescriptionText[i].ToString(), TEXT(" "), true);
		
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
	const int LineWidth = ((100.f * Width) - 100.f) -  DescriptionTextPadding;
	const int LineLimit = FMath::Floor( LineWidth / (DescriptionScale * 0.45f));
	
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
	
	CachedDescription = FText::AsLocalizable_Advanced(
		TEXT("NSamplesDisplayActor"),
		TitleText.ToString(),
		FinalString);
}

void ANSamplesDisplayActor::UpdateDescription() const
{
	if (CachedDescription.IsEmpty())
	{
		Parts->DescriptionTextComponent->SetVisibility(false);
		return;
	}
	
	// Set Position
	const bool bHeightVSFloorText = (Height < 2.f || (bFloorText && Depth > 2.f));

	FVector Origin;
	if (bSeparateTitlePanel)
	{
		Origin = TitleTextTransform().GetLocation();
	}
	else
	{
		Origin = Parts->TitleTextComponent->GetRelativeLocation();
	}

	const float TitleSpacerOffset = (TitleScale / 2.f) + 10.f;
	
	FVector WallLocation = FVector(15.f, TextAlignmentOffset(1.f, false) - (DescriptionTextPadding * .5f),
		Origin.Z - TitleSpacerOffset - (DescriptionTextPadding * .5f));
	FVector FloorLocation = FVector(Origin.X - TitleSpacerOffset - (DescriptionTextPadding * .5f), WallLocation.Y, WallLocation.X);

	if (bSeparateTitlePanel)
	{
		WallLocation.Z += (TitleSpacerOffset * 1.5f);
		FloorLocation.X += TitleSpacerOffset;
	}
	
	Parts->DescriptionTextComponent->SetRelativeTransform(
		FTransform(
		FRotator(bHeightVSFloorText ? 90.f : 0.f, 0.f, 0.f),
			bHeightVSFloorText ? FloorLocation : WallLocation, 
			FVector::OneVector));
	
	Parts->DescriptionTextComponent->SetText(CachedDescription);
	Parts->DescriptionTextComponent->SetHorizontalAlignment(TextAlignment);
	Parts->DescriptionTextComponent->SetWorldSize(DescriptionScale);
	Parts->DescriptionTextComponent->SetTextRenderColor(FNColor::GetColor(DescriptionColor));

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
		Materials->DisplayMaterial->SetVectorParameterValue(FName("Base Color"), FNColor::GetLinearColor(Color));
	}
}

void ANSamplesDisplayActor::UpdateNotice()
{
	// Headless server
	if (Parts->NoticeTextComponent == nullptr) return;
	
	Parts->NoticeDecalComponent->SetRelativeTransform(FTransform(
			FRotator(-90.f, 0.f, 0.f),
			FVector((Depth * 100.f) + NoticeDepth, 0.f , 0.f),
			FVector::OneVector));
	Parts->NoticeTextComponent->SetRelativeTransform(FTransform(
			FRotator(90.f, 0.f, 0.f),
		FVector((Depth * 100.f) + NoticeDepth, 0.f , 0.5f),
	FVector::OneVector)
	);

	if (bNoticeEnabled)
	{
		if (Materials->NoticeMaterial == nullptr)
		{
			Materials->NoticeMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Materials->NoticeMaterialInterface, NAME_None, EMIDCreationFlags::Transient);
			Parts->NoticeDecalComponent->SetMaterial(0, Materials->NoticeMaterial);
		}
		if (Materials->NoticeMaterial != nullptr)
		{
			Materials->NoticeMaterial->SetVectorParameterValue(FName("Base Color"), FNColor::GetLinearColor(NoticeColor));
		}
		
		Parts->NoticeDecalComponent->SetActive(true);
		Parts->NoticeDecalComponent->DecalSize = FVector(128.f, (50.f * Width), NoticeDepth);
		Parts->NoticeDecalComponent->SetVisibility(true);
		Parts->NoticeDecalComponent->SetHiddenInGame(false);

		Parts->NoticeTextComponent->SetText(NoticeText);
		Parts->NoticeTextComponent->SetWorldSize(NoticeScale);
		Parts->NoticeTextComponent->SetTextRenderColor(FNColor::GetColor(NoticeColor));
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
			FVector(FMath::Max(Depth, 2.5f) * 100.f, 0.f, FMath::Max(Height, 2.5f) * 100.f),
			FRotator(-135.f, 0.f , 0.f));
	
	if (bSpotlightEnabled)
	{
		Parts->SpotlightComponent->SetActive(true);
		
		Parts->SpotlightComponent->bAffectsWorld = true;
		Parts->SpotlightComponent->SetCastShadows(bSpotlightCastShadows);
		Parts->SpotlightComponent->SetCastVolumetricShadow(bSpotlightCastVolumetricShadow);
		Parts->SpotlightComponent->SetIntensity(SpotlightIntensity);
		Parts->SpotlightComponent->SetInnerConeAngle(SpotlightInnerConeAngle);
		Parts->SpotlightComponent->SetOuterConeAngle(SpotlightOuterConeAngle);
		Parts->SpotlightComponent->SetAttenuationRadius(SpotlightAttenuationRadius);
		Parts->SpotlightComponent->SetTemperature(SpotlightTemperature);

		Parts->SpotlightComponent->SetVisibility(true);
		Parts->SpotlightComponent->SetHiddenInGame(false);
		Parts->SpotlightComponent->SetIESTexture(SpotlightLightProfile);
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
	if (!TitleText.IsEmpty())
	{
		Parts->TitleTextComponent->SetVisibility(true);
		if (bSeparateTitlePanel)
		{
			
			Parts->TitleTextComponent->SetRelativeTransform(
				FTransform(
					FRotator(40.f, 0.f, 0.f),
					FVector(((Depth - 1.f) * 100.f) + 88.f, 
						TextAlignmentOffset(3.f, bSeparateTitlePanel), 22.f),
					FVector::OneVector));
			
			Parts->TitleTextComponent->SetHorizontalAlignment(EHTA_Center);
			Parts->TitleTextComponent->SetWorldSize(TitleScale * .5f);
		}
		else
		{
			Parts->TitleTextComponent->SetRelativeTransform(TitleTextTransform());
			Parts->TitleTextComponent->SetHorizontalAlignment(TextAlignment);
			Parts->TitleTextComponent->SetWorldSize(TitleScale);
		}
		Parts->TitleTextComponent->SetText(TitleText);
		Parts->TitleTextComponent->SetTextRenderColor(FNColor::GetColor(TitleColor));
		Parts->TitleTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	}
	else
	{
		Parts->TitleTextComponent->SetVisibility(false);
	}

	// Title Spacer
	if (!bSeparateTitlePanel)
	{
		const FVector TitleLocation = Parts->TitleTextComponent->GetRelativeLocation();
		FVector LocationOffset = FVector::ZeroVector;

		if (Height < 2.f || (bFloorText && Depth > 2.f))
		{
			LocationOffset.X = -1.f * (TitleScale / 2.f);
		}
		else
		{
			LocationOffset.Z = -1 * (TitleScale / 2.f);
		}
		
		Parts->TitleSpacerComponent->SetRelativeTransform(
			FTransform(
				FRotator::ZeroRotator,
				FVector(TitleLocation.X, 0.f, TitleLocation.Z) +
				LocationOffset,
				FVector(0.01f, Width - 1.f, 0.02f)
				));
		Parts->TitleSpacerComponent->SetVisibility(true);
	}
	else
	{
		Parts->TitleSpacerComponent->SetVisibility(false);
	}
}


void ANSamplesDisplayActor::UpdateWatermark() const
{
	if (!bWatermarkEnabled)
	{
		Parts->Watermark->SetVisibility(false);
		return;
	}

	Parts->Watermark->SetRelativeTransform(MainPanelTransform);
	if (Height < 2.f)
	{
		FVector NewLocation = Parts->Watermark->GetRelativeLocation();
		NewLocation += FVector(Depth * 50, 0, 9);
		Parts->Watermark->SetRelativeLocation(NewLocation);
		FRotator NewRotation = Parts->Watermark->GetRelativeRotation();
		NewRotation.Pitch += 180.f;
		Parts->Watermark->SetRelativeRotation(NewRotation);
	}
	else
	{
		FVector NewLocation = Parts->Watermark->GetRelativeLocation();
		NewLocation += FVector(9, 0, 100 + (Height * 50));
		Parts->Watermark->SetRelativeLocation(NewLocation);
	}
	Parts->Watermark->SetWorldScale3D(FVector(1, WatermarkScale, WatermarkScale));
	Parts->Watermark->SetVisibility(true);
}

float ANSamplesDisplayActor::TextAlignmentOffset(const float WidthAdjustment, const bool bForceCenter) const
{
	if (bForceCenter) return 0.f;
	switch (TextAlignment)
	{
	case EHTA_Center:
		return 0.f;
	case EHTA_Right:
		return ((Width - WidthAdjustment) * 0.5f) * -100.f;
	case EHTA_Left:
	default:
		return ((Width - WidthAdjustment) * 0.5f) * 100.f;
	}
}

FTransform ANSamplesDisplayActor::TitleTextTransform() const
{
	if (Height < 2.f || (bFloorText && Depth > 2.f))
	{
		return FTransform(
			FRotator(90.f, 0.f, 0.f),
			FVector(Depth * 100.f - (TitleScale + 50.f * 0.5f), TextAlignmentOffset(1.0, false), 15.f),
			FVector::OneVector);
	}
	return FTransform(
			FRotator::ZeroRotator,
			FVector(15.f, TextAlignmentOffset(1.0, false), Height * 100.f - (TitleScale + 50.f * 0.5f)),
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
	
	if (bTimerEnabled && bTestDisableTimer && TimerHandle.IsValid())
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

	if (bTimerEnabled && bTestDisableTimer && TimerHandle.IsValid())
	{
		GetWorldTimerManager().UnPauseTimer(TimerHandle);
	}
}