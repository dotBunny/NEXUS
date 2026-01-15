// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesDisplayActor.h"
#include "NColor.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
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

	// Create Materials
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialGrey(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_Grey"));
	if (MaterialGrey.Succeeded())
	{
		DisplayMaterialInterface = MaterialGrey.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialGreyDark(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_GreyDark"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialWhite(TEXT("/NexusBlockout/MaterialLibrary/MaterialInstances/Debug/MI_NDebug_White"));


	// Create Components
	NoticeDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	NoticeDecalComponent->bAutoActivate = false;
	NoticeDecalComponent->SetMobility(EComponentMobility::Static);
	NoticeDecalComponent->SetupAttachment(PartRoot);
	NoticeDecalComponent->DecalSize = FVector::ZeroVector;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalMaterial(TEXT("/NexusSharedSamples/M_NSamples_Notice"));
	if (DecalMaterial.Succeeded())
	{
		NoticeMaterialInterface = DecalMaterial.Object;
	}
	NoticeTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Notice"));
	NoticeTextComponent->SetupAttachment(PartRoot);
	NoticeTextComponent->SetWorldSize(24.f);
	NoticeTextComponent->HorizontalAlignment = EHTA_Center;
	NoticeTextComponent->VerticalAlignment = EVRTA_TextCenter;
	
	TitleTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Title"));
	TitleTextComponent->SetupAttachment(PartRoot);
	
	DescriptionTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Description"));
	DescriptionTextComponent->SetupAttachment(PartRoot);

	// These get stripped on servers
	if (!GIsServer)
	{
		static ConstructorHelpers::FObjectFinder<UFont> TextFont(TEXT("/Engine/EngineFonts/RobotoDistanceField"), LOAD_NoWarn);
		if (TextFont.Succeeded())
		{
			TitleTextComponent->SetFont(TextFont.Object);
			DescriptionTextComponent->SetFont(TextFont.Object);
			NoticeTextComponent->SetFont(TextFont.Object);
		}
	
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> TextMaterial(TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque"), LOAD_NoWarn);
		if (TextMaterial.Succeeded())
		{
			TitleTextComponent->SetMaterial(0, TextMaterial.Object);
			DescriptionTextComponent->SetMaterial(0, TextMaterial.Object);
			NoticeTextComponent->SetMaterial(0, TextMaterial.Object);
		}
	}
	
	SpotlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	SpotlightComponent->bAutoActivate = false;
	SpotlightComponent->SetMobility(EComponentMobility::Type::Stationary);
	SpotlightComponent->SetupAttachment(PartRoot);
	SpotlightComponent->SetIntensity(0.f);
	SpotlightComponent->SetIntensityUnits(ELightUnits::Unitless);
	SpotlightComponent->SetAttenuationRadius(0.f);
	SpotlightComponent->SetInnerConeAngle(0.f);
	SpotlightComponent->SetOuterConeAngle(0.f);
	SpotlightComponent->bUseTemperature = true;
	SpotlightComponent->SetTemperature(0.f);
	static ConstructorHelpers::FObjectFinder<UTextureLightProfile> LightProfile(TEXT("/Engine/EngineLightProfiles/180Degree_IES"));
	if (LightProfile.Succeeded())
	{
		SpotlightLightProfile = LightProfile.Object;
	}
	SpotlightComponent->SetIESTexture(nullptr);
	SpotlightComponent->bAffectsWorld = false;
	// Turn off the sprite?
	
	TitleSpacerComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TitleSpacer"));
	TitleSpacerComponent->SetupAttachment(PartRoot);
	TitleSpacerComponent->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EngineCubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (EngineCubeMesh.Succeeded())
	{
		TitleSpacerComponent->SetStaticMesh(EngineCubeMesh.Object);
		TitleSpacerComponent->SetMaterial(0, MaterialWhite.Object); // divider color
	}
	
	// Create Display
	PanelCurveEdge = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Curve_Edge"));
	DefaultInstanceStaticMesh(PanelCurveEdge);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EdgeCurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_EdgeCurve"));
	if (EdgeCurveMesh.Succeeded())
	{
		PanelCurveEdge->SetStaticMesh(EdgeCurveMesh.Object);
		PanelCurveEdge->SetMaterial(0, MaterialGreyDark.Object);
	}
	
	PanelSide = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Side"));
	DefaultInstanceStaticMesh(PanelSide);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SideMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Side"));
	if (SideMesh.Succeeded())
	{
		PanelSide->SetStaticMesh(SideMesh.Object);
		PanelSide->SetMaterial(1, MaterialGreyDark.Object);
	}

	PanelCurve = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Curve"));
	DefaultInstanceStaticMesh(PanelCurve);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Curve"));
	if (CurveMesh.Succeeded())
	{
		PanelCurve->SetStaticMesh(CurveMesh.Object);
	}

	PanelCorner = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Corner"));
	DefaultInstanceStaticMesh(PanelCorner);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CornerMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Corner"));
	if (CornerMesh.Succeeded())
	{
		PanelCorner->SetStaticMesh(CornerMesh.Object);
		PanelCorner->SetMaterial(1, MaterialGreyDark.Object);
	}

	PanelMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Panel_Main"));
	DefaultInstanceStaticMesh(PanelMain);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MainMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Main"));
	if (MainMesh.Succeeded())
	{
		PanelMain->SetStaticMesh(MainMesh.Object);
	}

	// Create Tag Bar
	TitleBarMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_Main"));
	DefaultInstanceStaticMesh(TitleBarMain);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarMainMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_Middle"));
	if (TitleBarMainMesh.Succeeded())
	{
		TitleBarMain->SetStaticMesh(TitleBarMainMesh.Object);
		TitleBarMain->SetMaterial(0, MaterialGreyDark.Object);
	}

	TitleBarEndLeft = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_End_Left"));
	DefaultInstanceStaticMesh(TitleBarEndLeft);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_R"));
	if (TitleBarEndMesh.Succeeded())
	{
		TitleBarEndLeft->SetStaticMesh(TitleBarEndMesh.Object);
		TitleBarEndLeft->SetMaterial(0, MaterialGreyDark.Object);
	}

	TitleBarEndRight = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TitleBar_End_Right"));
	DefaultInstanceStaticMesh(TitleBarEndRight);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndRMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_L"));
	if (TitleBarEndRMesh.Succeeded())
	{
		TitleBarEndRight->SetStaticMesh(TitleBarEndRMesh.Object);
		TitleBarEndRight->SetMaterial(0, MaterialGreyDark.Object);
	}

	// Create ShadowBox
	ShadowBoxSide = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Side"));
	DefaultInstanceStaticMesh(ShadowBoxSide);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side_Ribbed"));
	if (ShadowStraightMesh.Succeeded())
	{
		ShadowBoxSide->SetStaticMesh(ShadowStraightMesh.Object);
		ShadowBoxSide->SetMaterial(0, MaterialWhite.Object);
		ShadowBoxSide->SetMaterial(1, MaterialWhite.Object);
	}

	ShadowBoxRound = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Round"));
	DefaultInstanceStaticMesh(ShadowBoxRound);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowRoundMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_RoundEdge"));
	if (ShadowRoundMesh.Succeeded())
	{
		ShadowBoxRound->SetStaticMesh(ShadowRoundMesh.Object);
		ShadowBoxRound->SetMaterial(0, MaterialWhite.Object);
		ShadowBoxRound->SetMaterial(1, MaterialWhite.Object);
	}

	ShadowBoxTop = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ShadowBox_Top"));
	DefaultInstanceStaticMesh(ShadowBoxTop);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightTopMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side"));
	if (ShadowStraightTopMesh.Succeeded())
	{
		ShadowBoxTop->SetStaticMesh(ShadowStraightTopMesh.Object);
		ShadowBoxTop->SetMaterial(0, MaterialWhite.Object);
		ShadowBoxTop->SetMaterial(1, MaterialWhite.Object);
	}

	// Watermark Mesh
	Watermark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Branding"));
	Watermark->SetupAttachment(PartRoot);
	Watermark->SetMobility(EComponentMobility::Static);
	Watermark->SetCollisionProfileName(FName("NoCollision"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>BrandingMesh(TEXT("/NexusSharedSamples/SM_NSamples_NEXUS"));
	if (BrandingMesh.Succeeded())
	{
		Watermark->SetStaticMesh(BrandingMesh.Object);
		Watermark->SetMaterial(0, MaterialWhite.Object);
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
	UpdateTestComponents();
	
	// Clear previous instances
	PanelMain->ClearInstances();
	PanelCorner->ClearInstances();
	PanelSide->ClearInstances();
	PanelCurve->ClearInstances();
	PanelCurveEdge->ClearInstances();

	TitleBarMain->ClearInstances();
	TitleBarEndLeft->ClearInstances();
	TitleBarEndRight->ClearInstances();

	ShadowBoxSide->ClearInstances();
	ShadowBoxTop->ClearInstances();
	ShadowBoxRound->ClearInstances();

	// Ensure depth
	if (Depth < 0.5f)
	{
		Depth = 0.5f;
	}

	// Static Instance Meshes
	CreateDisplayInstances();
	CreateShadowBoxInstances();
	CreateTitlePanelInstances();

	// Adjust Components
	UpdateNotice();
	UpdateSpotlight();
	UpdateDisplayColor();
	UpdateTitleText();
	BuildDescription();
	UpdateDescription();
	UpdateCollisions();
	UpdateWatermark();
}

UActorComponent* ANSamplesDisplayActor::GetComponentInstance(TSubclassOf<UActorComponent> ComponentClass)
{
	return this->GetComponentByClass(ComponentClass);
}

void ANSamplesDisplayActor::TimerExpired()
{
	OnTimerExpired();
}

void ANSamplesDisplayActor::CreateDisplayInstances()
{
	// Ensure width
	if (Width < 2.f) Width = 2.f;
	
	if (Height >= 2.f && Depth >= 2.f)
	{
		const float WidthY = (Width - 2.f) * 0.5f;
		PanelCurve->AddInstance(
			FTransform(
				FRotator::ZeroRotator,
				FVector::ZeroVector,
				FVector(1.f, WidthY, 1.f)), false);

		TArray<FTransform> InstanceTransforms;
		InstanceTransforms.Add(FTransform(
					
			FRotator::ZeroRotator,
			FVector(0.f, WidthY * 100.f, 0.f),
			FVector::OneVector));
		
		InstanceTransforms.Add(FTransform(
		FRotator(90.f, 360.f, 180.f),
		FVector(0.f, WidthY * -100.f, 0.f),
		FVector::OneVector));
		PanelCurveEdge->AddInstances(InstanceTransforms, false, false, true);
	}
	
	// Panels
	if (Depth >= 2.f)
	{
		if (Height >= 2.f)
		{
			// Traditional Display
			MainPanelTransform = FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, -100.f), FVector::OneVector);
			CreateScalablePanelInstances(MainPanelTransform, Height);
			FloorPanelTransform = FTransform(FRotator(90.f, 180.f, 0.f), FVector(0.f, 0.f, -100.f), FVector::OneVector);
			CreateScalablePanelInstances(FloorPanelTransform, Depth);
		}
		else
		{
			// Floor
			MainPanelTransform = FTransform(FRotator(-90.f, 0.f, 0.f), FVector::ZeroVector, FVector::OneVector);
			CreateScalablePanelInstances(MainPanelTransform, 1.f, true);
			FloorPanelTransform = FTransform(FRotator(90.f, 180.f, 0.f), FVector(0.f, 0.f, -100), FVector::OneVector); 
			CreateScalablePanelInstances(FloorPanelTransform, Depth);
		}
	}
	else
	{

		if (Height < 2.f)
		{
			Height = 2.f;
		}
		FloorPanelTransform = FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, ((Height * 0.5f)) - 102.f), FVector::OneVector);
		CreateScalablePanelInstances(FloorPanelTransform, Height);
		MainPanelTransform = FTransform(FRotator(180.f, 180.f, 0.f), FVector(0.f, 0.f, 100.f), FVector::OneVector);
		CreateScalablePanelInstances(MainPanelTransform, 2.f, false);
	}
}

void ANSamplesDisplayActor::CreateScalablePanelInstances(const FTransform& BaseTransform, const float Length, bool bIgnoreMainPanel) const
{
	const float PrimaryScaleY = (Width - 2) * 0.5f;
	const float PrimaryScaleZ = (Length - 2) * 0.5f;
	const float SecondaryY = ((Width - 2.f) * .5f) * 100.f;
	const float SecondaryZ = (Length - 2.f) * 100.f;
	const float ScaledLocationY = PrimaryScaleY * 100.f;
	const float ScaledLocationZ = PrimaryScaleZ * 100.f;

	if (!bIgnoreMainPanel)
	{
		PanelMain->AddInstance(
			UKismetMathLibrary::MakeRelativeTransform(
				FTransform(
					FRotator::ZeroRotator,
					FVector::ZeroVector,
					FVector(1.f, PrimaryScaleY, PrimaryScaleZ)),
					BaseTransform), false);
	}
		
	// Corners
	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Add(UKismetMathLibrary::MakeRelativeTransform(FTransform(
		FRotator::ZeroRotator,
		FVector(0.f, SecondaryY, SecondaryZ),
		FVector::OneVector), BaseTransform));
	
	InstanceTransforms.Add(UKismetMathLibrary::MakeRelativeTransform(FTransform(
	FRotator(0.f,0.f, -90.f),
	FVector(0.f, SecondaryY * -1.f, SecondaryZ),
	FVector::OneVector), BaseTransform));
	
	PanelCorner->AddInstances(InstanceTransforms, false, false, true);

	// Sides
	ScaleSafeInstance(PanelSide, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator(0.f,0.f, 90.f),
			FVector(0.f, ScaledLocationY, ScaledLocationZ),
			FVector(1.f, PrimaryScaleZ, 1.f)), BaseTransform));
	
	ScaleSafeInstance(PanelSide, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator(0.f,0.f, -90.f),
			FVector(0.f, ScaledLocationY * -1.f, ScaledLocationZ),
			FVector(1.f, PrimaryScaleZ, 1.f)), BaseTransform));
	
	ScaleSafeInstance(PanelSide, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator::ZeroRotator,
			FVector(0.f,0.f, (Length - 2.f) * 100.f),
			FVector(1.f, PrimaryScaleY, 1.f)), BaseTransform));
}

void ANSamplesDisplayActor::CreateShadowBoxInstances() const
{
	// Ensure the cover is only there when we need it and it will behave appropriately
	if (!(ShadowBoxCoverDepthPercentage > 0.05f && Height > 2.0f) || Depth <= 1.f) return;

	TArray<FTransform> InstanceTransforms;
	
	InstanceTransforms.Add(
		FTransform(FRotator::ZeroRotator,
			FVector(0.f, Width * 50.f, 0.f),
			FVector((Depth - 1.f) * ShadowBoxCoverDepthPercentage, 1.f, (Height - 1.f) / 5.f)));
	InstanceTransforms.Add(
			FTransform(FRotator(0.f, 0.f, 180.f),
			(InstanceTransforms[0].GetLocation() * -1.f) + FVector(0.f, 0.f, (Height * 100) - 100.f),
			InstanceTransforms[0].GetScale3D()));
	
	ShadowBoxSide->AddInstances(InstanceTransforms, false, false, true);
	InstanceTransforms.Empty();
	
	ShadowBoxTop->AddInstance(FTransform(
		FRotator(0.f, 0.f, -90.f),
		FVector(0.f, (Width - 2.0f) * 50.f, Height * 100.f),
		FVector((Depth - 1.f) * ShadowBoxCoverDepthPercentage, 1.f, (Width - 2.f) / 5.f)));
	
	InstanceTransforms.Add(FTransform(
		FRotator::ZeroRotator,
		FVector(0.f, Width * 50.f, (Height - 1.0) * 100.f),
		FVector((Depth - 1.f) * ShadowBoxCoverDepthPercentage, 1.f, 1.f)));
	
	const FVector Location = InstanceTransforms[0].GetLocation();
	InstanceTransforms.Add(FTransform(
		FRotator(0.f, 0.f, -90.f),
		FVector(Location.X, (Location.Y * -1.f) + 100.f, Location.Z + 100.f),
		InstanceTransforms[0].GetScale3D()));
	
	ShadowBoxRound->AddInstances(InstanceTransforms, false, false, true);
}
void ANSamplesDisplayActor::CreateTitlePanelInstances() const
{
	if (!bSeparateTitlePanel) return;

	// Ensure width doesn't make it look bad
	float WidthLimited = Width;
	if (WidthLimited < 3.f)
	{
		WidthLimited = 3;
	}
	
	const FTransform BaseTransform = FTransform(
		FRotator::ZeroRotator,
		FVector((Depth - 1.f) * 100.f, 0.f, 0.f),
		FVector(1.f, WidthLimited - 3.f, 1.f));
	
	TitleBarMain->AddInstance(BaseTransform, false);
	const FVector BaseLocation = BaseTransform.GetLocation();
	
	TitleBarEndLeft->AddInstance(FTransform(
			FRotator::ZeroRotator,
			FVector(BaseLocation.X, BaseLocation.Y + (((WidthLimited - 3.f) * 50.f) * -1.f), BaseLocation.Z),
			FVector::OneVector), false);
	
	TitleBarEndRight->AddInstance(FTransform(
		FRotator::ZeroRotator,
		FVector(BaseLocation.X, BaseLocation.Y + ((WidthLimited - 3.f) * 50.f), BaseLocation.Z),
		FVector::OneVector), false); 
}

void ANSamplesDisplayActor::BuildDescription()
{
	// Headless server
	if (DescriptionTextComponent == nullptr) return;
	
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
		GetTitle(),
		FinalString);
}

void ANSamplesDisplayActor::UpdateDescription() const
{
	if (CachedDescription.IsEmpty())
	{
		DescriptionTextComponent->SetVisibility(false);
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
		Origin = TitleTextComponent->GetRelativeLocation();
	}

	const float TitleSpacerOffset = GetTitleSpacerOffset();
	
	FVector WallLocation = FVector(15.f, TextAlignmentOffset(1.f, false) - (DescriptionTextPadding * .5f),
		Origin.Z - TitleSpacerOffset - (DescriptionTextPadding * .5f));
	FVector FloorLocation = FVector(Origin.X - TitleSpacerOffset - (DescriptionTextPadding * .5f), WallLocation.Y, WallLocation.X);

	if (bSeparateTitlePanel)
	{
		WallLocation.Z += (TitleSpacerOffset * 1.5f);
		FloorLocation.X += TitleSpacerOffset;
	}
	
	DescriptionTextComponent->SetRelativeTransform(
		FTransform(
		FRotator(bHeightVSFloorText ? 90.f : 0.f, 0.f, 0.f),
			bHeightVSFloorText ? FloorLocation : WallLocation, 
			FVector::OneVector));
	
	DescriptionTextComponent->SetText(CachedDescription);
	DescriptionTextComponent->SetHorizontalAlignment(TextAlignment);
	DescriptionTextComponent->SetWorldSize(DescriptionScale);
	DescriptionTextComponent->SetTextRenderColor(FNColor::GetColor(DescriptionColor));

	if (bHeightVSFloorText)
	{
		DescriptionTextComponent->SetVerticalAlignment(EVRTA_TextBottom);
	}
	else
	{
		DescriptionTextComponent->SetVerticalAlignment(EVRTA_TextTop);
	}
	DescriptionTextComponent->SetVisibility(true);
}

void ANSamplesDisplayActor::UpdateDisplayColor()
{
	if (DisplayMaterial == nullptr)
	{
		DisplayMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, DisplayMaterialInterface, NAME_None, EMIDCreationFlags::Transient);
		
		// Update the static meshes with the dynamic material
		if (DisplayMaterial != nullptr)
		{
			PanelMain->SetMaterial(0, DisplayMaterial);
			PanelCorner->SetMaterial(0, DisplayMaterial);
			PanelCurve->SetMaterial(0, DisplayMaterial);
			PanelSide->SetMaterial(0, DisplayMaterial);
			PanelCurveEdge->SetMaterial(1, DisplayMaterial);
			Watermark->SetMaterial(0, DisplayMaterial);
		}
	}
	if (DisplayMaterial != nullptr)
	{
		DisplayMaterial->SetVectorParameterValue(FName("Base Color"), FNColor::GetLinearColor(Color));
	}
}

void ANSamplesDisplayActor::UpdateNotice()
{
	// Headless server
	if (NoticeTextComponent == nullptr) return;
	
	NoticeDecalComponent->SetRelativeTransform(FTransform(
			FRotator(-90.f, 0.f, 0.f),
			FVector((Depth * 100.f) + NoticeDepth, 0.f , 0.f),
			FVector::OneVector));
	NoticeTextComponent->SetRelativeTransform(FTransform(
			FRotator(90.f, 0.f, 0.f),
		FVector((Depth * 100.f) + NoticeDepth, 0.f , 0.5f),
	FVector::OneVector)
	);

	if (bNoticeEnabled)
	{
		if (NoticeMaterial == nullptr)
		{
			NoticeMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, NoticeMaterialInterface, NAME_None, EMIDCreationFlags::Transient);
			NoticeDecalComponent->SetMaterial(0, NoticeMaterial);
		}
		if (NoticeMaterial != nullptr)
		{
			NoticeMaterial->SetVectorParameterValue(FName("Base Color"), FNColor::GetLinearColor(NoticeColor));
		}
		
		NoticeDecalComponent->SetActive(true);
		NoticeDecalComponent->DecalSize = FVector(128.f, (50.f * Width), NoticeDepth);
		NoticeDecalComponent->SetVisibility(true);
		NoticeDecalComponent->SetHiddenInGame(false);

		NoticeTextComponent->SetText(NoticeText);
		NoticeTextComponent->SetWorldSize(NoticeScale);
		NoticeTextComponent->SetTextRenderColor(FNColor::GetColor(NoticeColor));
		NoticeTextComponent->SetVisibility(true);
		NoticeTextComponent->SetHiddenInGame(false);
	}
	else
	{
		
		NoticeDecalComponent->DecalSize = FVector::ZeroVector;
		NoticeDecalComponent->SetVisibility(false);
		NoticeDecalComponent->SetActive(false);
		NoticeDecalComponent->SetHiddenInGame(true);

		NoticeTextComponent->SetVisibility(false);
		NoticeTextComponent->SetHiddenInGame(true);
	}
}

void ANSamplesDisplayActor::UpdateSpotlight() const
{
	// Headless server
	if (SpotlightComponent == nullptr) return;
	
	SpotlightComponent->SetRelativeLocationAndRotation(
			FVector(FMath::Max(Depth, 2.5f) * 100.f, 0.f, FMath::Max(Height, 2.5f) * 100.f),
			FRotator(-135.f, 0.f , 0.f));
	
	if (bSpotlightEnabled)
	{
		SpotlightComponent->SetActive(true);
		
		SpotlightComponent->bAffectsWorld = true;
		SpotlightComponent->SetCastShadows(bSpotlightCastShadows);
		SpotlightComponent->SetCastVolumetricShadow(bSpotlightCastVolumetricShadow);
		SpotlightComponent->SetIntensity(SpotlightIntensity);
		SpotlightComponent->SetInnerConeAngle(SpotlightInnerConeAngle);
		SpotlightComponent->SetOuterConeAngle(SpotlightOuterConeAngle);
		SpotlightComponent->SetAttenuationRadius(SpotlightAttenuationRadius);
		SpotlightComponent->SetTemperature(SpotlightTemperature);

		SpotlightComponent->SetVisibility(true);
		SpotlightComponent->SetHiddenInGame(false);
		SpotlightComponent->SetIESTexture(SpotlightLightProfile);
	}
	else
	{
		SpotlightComponent->SetIntensity(0.f);
		SpotlightComponent->SetInnerConeAngle(0.f);
		SpotlightComponent->SetOuterConeAngle(0.f);
		SpotlightComponent->SetAttenuationRadius(0.f);
		SpotlightComponent->SetTemperature(0.f);
		SpotlightComponent->bAffectsWorld = false;
		
		SpotlightComponent->SetVisibility(false);
		SpotlightComponent->SetHiddenInGame(true);
		SpotlightComponent->SetIESTexture(nullptr);
		SpotlightComponent->SetActive(false);
	}
}

void ANSamplesDisplayActor::UpdateTitleText() const
{
	// Headless server
	if (TitleTextComponent == nullptr) return;
	
	// Title Text
	if (!TitleText.IsEmpty())
	{
		TitleTextComponent->SetVisibility(true);
		if (bSeparateTitlePanel)
		{
			
			TitleTextComponent->SetRelativeTransform(TitlePanelTextTransform());
			TitleTextComponent->SetHorizontalAlignment(EHTA_Center);
			TitleTextComponent->SetWorldSize(TitleScale * .5f);
		}
		else
		{
			TitleTextComponent->SetRelativeTransform(TitleTextTransform());
			TitleTextComponent->SetHorizontalAlignment(TextAlignment);
			TitleTextComponent->SetWorldSize(TitleScale);
		}
		TitleTextComponent->SetText(TitleText);
		TitleTextComponent->SetTextRenderColor(FNColor::GetColor(TitleColor));
		TitleTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	}
	else
	{
		TitleTextComponent->SetVisibility(false);
	}

	// Title Spacer
	if (!bSeparateTitlePanel)
	{
		const FVector TitleLocation = TitleTextComponent->GetRelativeLocation();
		FVector LocationOffset = FVector::ZeroVector;

		if (Height < 2.f || (bFloorText && Depth > 2.f))
		{
			LocationOffset.X = -1.f * GetTitleOffset();
		}
		else
		{
			LocationOffset.Z = -1 * GetTitleOffset();
		}
		
		TitleSpacerComponent->SetRelativeTransform(
			FTransform(
				FRotator::ZeroRotator,
				FVector(TitleLocation.X, 0.f, TitleLocation.Z) +
				LocationOffset,
				FVector(0.01f, Width - 1.f, 0.02f)
				));
		TitleSpacerComponent->SetVisibility(true);
	}
	else
	{
		TitleSpacerComponent->SetVisibility(false);
	}
}

void ANSamplesDisplayActor::UpdateCollisions() const
{
	if (bCollisionEnabled)
	{
		PanelMain->SetCollisionProfileName(FName("BlockAll"));
		PanelCorner->SetCollisionProfileName(FName("BlockAll"));
		PanelSide->SetCollisionProfileName(FName("BlockAll"));
		PanelCurve->SetCollisionProfileName(FName("BlockAll"));
		PanelCurveEdge->SetCollisionProfileName(FName("BlockAll"));
		
		TitleBarMain->SetCollisionProfileName(FName("BlockAll"));
		TitleBarEndLeft->SetCollisionProfileName(FName("BlockAll"));
		TitleBarEndRight->SetCollisionProfileName(FName("BlockAll"));
		
		ShadowBoxSide->SetCollisionProfileName(FName("BlockAll"));
		ShadowBoxTop->SetCollisionProfileName(FName("BlockAll"));
		ShadowBoxRound->SetCollisionProfileName(FName("BlockAll"));
		
		Watermark->SetCollisionProfileName(FName("BlockAll"));
	}
	else
	{
		PanelMain->SetCollisionProfileName(FName("NoCollision"));
		PanelCorner->SetCollisionProfileName(FName("NoCollision"));
		PanelSide->SetCollisionProfileName(FName("NoCollision"));
		PanelCurve->SetCollisionProfileName(FName("NoCollision"));
		PanelCurveEdge->SetCollisionProfileName(FName("NoCollision"));
		
		TitleBarMain->SetCollisionProfileName(FName("NoCollision"));
		TitleBarEndLeft->SetCollisionProfileName(FName("NoCollision"));
		TitleBarEndRight->SetCollisionProfileName(FName("NoCollision"));
		
		ShadowBoxSide->SetCollisionProfileName(FName("NoCollision"));
		ShadowBoxTop->SetCollisionProfileName(FName("NoCollision"));
		ShadowBoxRound->SetCollisionProfileName(FName("NoCollision"));

		Watermark->SetCollisionProfileName(FName("NoCollision"));
	}
}

void ANSamplesDisplayActor::UpdateTestComponents()
{
	// Reset our SceneRoot
	RootComponent = SceneRoot;
}

void ANSamplesDisplayActor::UpdateWatermark() const
{
	if (!bWatermarkEnabled)
	{
		Watermark->SetVisibility(false);
		return;
	}

	Watermark->SetRelativeTransform(MainPanelTransform);
	if (Height < 2.f)
	{
		FVector NewLocation = Watermark->GetRelativeLocation();
		NewLocation += FVector(Depth * 50, 0, 9);
		Watermark->SetRelativeLocation(NewLocation);
		FRotator NewRotation = Watermark->GetRelativeRotation();
		NewRotation.Pitch += 180.f;
		Watermark->SetRelativeRotation(NewRotation);
	}
	else
	{
		FVector NewLocation = Watermark->GetRelativeLocation();
		NewLocation += FVector(9, 0, 100 + (Height * 50));
		Watermark->SetRelativeLocation(NewLocation);
	}
	Watermark->SetWorldScale3D(FVector(1, WatermarkScale, WatermarkScale));
	Watermark->SetVisibility(true);
}

void ANSamplesDisplayActor::DefaultInstanceStaticMesh(UInstancedStaticMeshComponent* Instance) const
{
	Instance->SetupAttachment(PartRoot);
	Instance->SetMobility(EComponentMobility::Static);
	Instance->SetCollisionProfileName(FName("NoCollision"));
}

float ANSamplesDisplayActor::GetTitleOffset() const
{
	return TitleScale / 2.f;
}

float ANSamplesDisplayActor::GetTitleSpacerOffset() const
{
	return GetTitleOffset() + 10.f;
}

void ANSamplesDisplayActor::ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform)
{
	if (const FVector Scale = Transform.GetScale3D();
		Scale.X > 0.f && Scale.Y > 0.f && Scale.Z > 0.f)
	{
		Instance->AddInstance(Transform);
	}
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

FTransform ANSamplesDisplayActor::TitlePanelTextTransform() const
{
	return FTransform(
		FRotator(40.f, 0.f, 0.f),
		FVector(((Depth - 1.f) * 100.f) + 88.f, TextAlignmentOffset(3.f, bSeparateTitlePanel), 22.f),
		FVector::OneVector);
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