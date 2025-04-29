// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesDisplayActor.h"

#include "NCoreMinimal.h"
#include "Components/DecalComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

ANSamplesDisplayActor::ANSamplesDisplayActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	RootComponent = SceneRoot;
	
	// Create Materials
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialGrey(TEXT("/NexusMaterialLibrary/Debug/MI_NDebug_Grey"));
	if (MaterialGrey.Succeeded())
	{
		DisplayMaterialInterface = MaterialGrey.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialGreyDark(TEXT("/NexusMaterialLibrary/Debug/MI_NDebug_GreyDark"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialWhite(TEXT("/NexusMaterialLibrary/Debug/MI_NDebug_White"));

	// Create Components
	NoticeDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	NoticeDecalComponent->bAutoActivate = false;
	NoticeDecalComponent->SetMobility(EComponentMobility::Static);
	NoticeDecalComponent->SetupAttachment(RootComponent);
	NoticeDecalComponent->DecalSize = FVector::ZeroVector;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalMaterial(TEXT("/NexusSharedSamples/M_NSamples_Notice"));
	if (DecalMaterial.Succeeded())
	{
		NoticeMaterialInterface = DecalMaterial.Object;
	}
	NoticeTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Notice"));
	NoticeTextComponent->SetupAttachment(RootComponent);
	NoticeTextComponent->SetWorldSize(24.f);
	NoticeTextComponent->HorizontalAlignment = EHTA_Center;
	NoticeTextComponent->VerticalAlignment = EVRTA_TextCenter;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TextMaterial(TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque"));
	static ConstructorHelpers::FObjectFinder<UFont> TextFont(TEXT("/Engine/EngineFonts/RobotoDistanceField"));

	
	
	TitleTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Title"));
	TitleTextComponent->SetupAttachment(RootComponent);
	
	DescriptionTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Description"));
	DescriptionTextComponent->SetupAttachment(RootComponent);
	if (TextFont.Succeeded())
	{
		TitleTextComponent->SetFont(TextFont.Object);
		DescriptionTextComponent->SetFont(TextFont.Object);
		NoticeTextComponent->SetFont(TextFont.Object);
	}
	if (TextMaterial.Succeeded())
	{
		TitleTextComponent->SetMaterial(0, TextMaterial.Object);
		DescriptionTextComponent->SetMaterial(0, TextMaterial.Object);
		NoticeTextComponent->SetMaterial(0, TextMaterial.Object);
	}
	
	SpotlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	SpotlightComponent->bAutoActivate = false;
	SpotlightComponent->SetMobility(EComponentMobility::Type::Stationary);
	SpotlightComponent->SetupAttachment(RootComponent);
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
	TitleSpacerComponent->SetupAttachment(RootComponent);
	TitleSpacerComponent->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EngineCubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (EngineCubeMesh.Succeeded())
	{
		TitleSpacerComponent->SetStaticMesh(EngineCubeMesh.Object);
		TitleSpacerComponent->SetMaterial(0, MaterialWhite.Object); // divider color
	}
	
	// Create Display
	ISM_CurveEdge = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_CurveEdge"));
	DefaultInstanceStaticMesh(ISM_CurveEdge);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EdgeCurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_EdgeCurve"));
	if (EdgeCurveMesh.Succeeded())
	{
		ISM_CurveEdge->SetStaticMesh(EdgeCurveMesh.Object);
		ISM_CurveEdge->SetMaterial(0, MaterialGreyDark.Object);
	}
	
	ISM_Side = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_Side"));
	DefaultInstanceStaticMesh(ISM_Side);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SideMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Side"));
	if (SideMesh.Succeeded())
	{
		ISM_Side->SetStaticMesh(SideMesh.Object);
		ISM_Side->SetMaterial(1, MaterialGreyDark.Object);
	}

	ISM_Curve = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_Curve"));
	DefaultInstanceStaticMesh(ISM_Curve);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CurveMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Curve"));
	if (CurveMesh.Succeeded())
	{
		ISM_Curve->SetStaticMesh(CurveMesh.Object);
	}

	ISM_Corner = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_Corner"));
	DefaultInstanceStaticMesh(ISM_Corner);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CornerMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Corner"));
	if (CornerMesh.Succeeded())
	{
		ISM_Corner->SetStaticMesh(CornerMesh.Object);
		ISM_Corner->SetMaterial(1, MaterialGreyDark.Object);
	}

	ISM_Main = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_Main"));
	DefaultInstanceStaticMesh(ISM_Main);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MainMesh(TEXT("/NexusSharedSamples/SM_NSamples_Display_Main"));
	if (MainMesh.Succeeded())
	{
		ISM_Main->SetStaticMesh(MainMesh.Object);
	}

	// Create Tag Bar
	ISM_TitleBarMain = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_TitleBarMain"));
	DefaultInstanceStaticMesh(ISM_TitleBarMain);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarMainMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_Middle"));
	if (TitleBarMainMesh.Succeeded())
	{
		ISM_TitleBarMain->SetStaticMesh(TitleBarMainMesh.Object);
		ISM_TitleBarMain->SetMaterial(0, MaterialGreyDark.Object);
	}

	ISM_TitleBarEnd = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_TitleBarEnd"));
	DefaultInstanceStaticMesh(ISM_TitleBarEnd);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_R"));
	if (TitleBarEndMesh.Succeeded())
	{
		ISM_TitleBarEnd->SetStaticMesh(TitleBarEndMesh.Object);
		ISM_TitleBarEnd->SetMaterial(0, MaterialGreyDark.Object);
	}

	ISM_TitleBarEndR = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_TitleBarEndR"));
	DefaultInstanceStaticMesh(ISM_TitleBarEndR);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TitleBarEndRMesh(TEXT("/NexusSharedSamples/SM_NSamples_TagBar_L"));
	if (TitleBarEndRMesh.Succeeded())
	{
		ISM_TitleBarEndR->SetStaticMesh(TitleBarEndRMesh.Object);
		ISM_TitleBarEndR->SetMaterial(0, MaterialGreyDark.Object);
	}

	// Create ShadowBox
	ISM_ShadowStraight = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_ShadowStraight"));
	DefaultInstanceStaticMesh(ISM_ShadowStraight);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side_Ribbed"));
	if (ShadowStraightMesh.Succeeded())
	{
		ISM_ShadowStraight->SetStaticMesh(ShadowStraightMesh.Object);
		ISM_ShadowStraight->SetMaterial(0, MaterialWhite.Object);
		ISM_ShadowStraight->SetMaterial(1, MaterialWhite.Object);
	}

	ISM_ShadowRound = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_ShadowRound"));
	DefaultInstanceStaticMesh(ISM_ShadowRound);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowRoundMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_RoundEdge"));
	if (ShadowRoundMesh.Succeeded())
	{
		ISM_ShadowRound->SetStaticMesh(ShadowRoundMesh.Object);
		ISM_ShadowRound->SetMaterial(0, MaterialWhite.Object);
		ISM_ShadowRound->SetMaterial(1, MaterialWhite.Object);
	}

	ISM_ShadowStraightTop = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM_ShadowStraightTop"));
	DefaultInstanceStaticMesh(ISM_ShadowStraightTop);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowStraightTopMesh(TEXT("/NexusSharedSamples/SM_NSamples_ShadowBox_Side"));
	if (ShadowStraightTopMesh.Succeeded())
	{
		ISM_ShadowStraightTop->SetStaticMesh(ShadowStraightTopMesh.Object);
		ISM_ShadowStraightTop->SetMaterial(0, MaterialWhite.Object);
		ISM_ShadowStraightTop->SetMaterial(1, MaterialWhite.Object);
	}
}

void ANSamplesDisplayActor::OnConstruction(const FTransform& Transform)
{
	Rebuild();
	Super::OnConstruction(Transform);
}

void ANSamplesDisplayActor::Rebuild()
{
	// Clear previous instances
	ISM_Main->ClearInstances();
	ISM_Corner->ClearInstances();
	ISM_Side->ClearInstances();
	ISM_Curve->ClearInstances();
	ISM_CurveEdge->ClearInstances();

	ISM_TitleBarMain->ClearInstances();
	ISM_TitleBarEnd->ClearInstances();
	ISM_TitleBarEndR->ClearInstances();

	ISM_ShadowStraight->ClearInstances();
	ISM_ShadowStraightTop->ClearInstances();
	ISM_ShadowRound->ClearInstances();

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
	UpdateDescription();
}

UActorComponent* ANSamplesDisplayActor::GetComponentInstance(TSubclassOf<UActorComponent> ComponentClass)
{
	return this->GetComponentByClass(ComponentClass);
}
FColor ANSamplesDisplayActor::GetColor(const ENSamplesDisplayColor Color)
{
	return GetLinearColor(Color).ToFColor(true);
}
FLinearColor ANSamplesDisplayActor::GetLinearColor(const ENSamplesDisplayColor Color)
{
	constexpr FLinearColor BlueDark = FLinearColor(0.0352941176470588f, 0.0509803921568627f, 0.1450980392156863f, 1.0f);
	constexpr FLinearColor BlueMid = FLinearColor(0.1058823529411765f, 0.2745098039215686f, 0.6156862745098039f, 1.0f);
	constexpr FLinearColor BlueLight = FLinearColor(0.1019607843137255f, 0.8f, 0.9333333333333333f, 1.0f);
	constexpr FLinearColor Pink = FLinearColor(0.8196078431372549f, 0.3529411764705882f, 0.8941176470588235f, 1.0f);
	constexpr FLinearColor GreyLight = FLinearColor(0.123264f, 0.123264f, 0.123264f, 1.0f);
	constexpr FLinearColor GreyDark = FLinearColor(0.029514f, 0.029514f, 0.029514f, 1.0f);
	constexpr FLinearColor Yellow = FLinearColor(1.f, 1.f, 0.0f, 1.0f);
	constexpr FLinearColor Orange = FLinearColor(1.f, 0.5f, 0.f, 1.0f);
	
	switch (Color)
	{
	case SDC_Black:
		return FLinearColor::Black;
	case SDC_White:
		return FLinearColor::White;
	case SDC_BlueLight:
		return BlueLight;
	case SDC_BlueMid:
		return BlueMid;
	case SDC_BlueDark:
		return BlueDark;
	case SDC_Red:
		return FLinearColor::Red;
	case SDC_Green:
		return FLinearColor::Green;
	case SDC_GreyLight:
		return GreyLight;
	case SDC_GreyDark:
		return GreyDark;
	case SDC_Yellow:
		return Yellow;
	case SDC_Orange:
		return Orange;
	case SDC_Pink:
		break;
	}
	return Pink;
}

void ANSamplesDisplayActor::CreateDisplayInstances()
{
	// Ensure width
	if (Width < 2.f) Width = 2.f;
	
	if (Height >= 2.f && Depth >= 2.f)
	{
		const float WidthY = (Width - 2.f) * 0.5f;
		ISM_Curve->AddInstance(
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
		ISM_CurveEdge->AddInstances(InstanceTransforms, false, false, true);
	}
	
	// Panels
	if (Depth >= 2.f)
	{
		if (Height >= 2.f)
		{
			// Traditional Display
			CreateScalablePanelInstances(FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, -100.f), FVector::OneVector), Height);
			CreateScalablePanelInstances(FTransform(FRotator(90.f, 180.f, 0.f), FVector(0.f, 0.f, -100.f), FVector::OneVector), Depth);
		}
		else
		{
			
			// Floor
			CreateScalablePanelInstances(FTransform(FRotator(-90.f, 0.f, 0.f), FVector::ZeroVector, FVector::OneVector), 1.f, true);
			CreateScalablePanelInstances(FTransform(FRotator(90.f, 180.f, 0.f), FVector(0.f, 0.f, -100), FVector::OneVector), Depth);
		}
	}
	else
	{

		if (Height < 2.f)
		{
			Height = 2.f;
		}
		
		CreateScalablePanelInstances(FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, ((Height * 0.5f)) - 102.f), FVector::OneVector), Height);
		CreateScalablePanelInstances(FTransform(FRotator(180.f, 180.f, 0.f), FVector(0.f, 0.f, 100.f), FVector::OneVector), 2.f, false);
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
		ISM_Main->AddInstance(
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
	
	ISM_Corner->AddInstances(InstanceTransforms, false, false, true);

	// Sides
	ScaleSafeInstance(ISM_Side, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator(0.f,0.f, 90.f),
			FVector(0.f, ScaledLocationY, ScaledLocationZ),
			FVector(1.f, PrimaryScaleZ, 1.f)), BaseTransform));
	
	ScaleSafeInstance(ISM_Side, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator(0.f,0.f, -90.f),
			FVector(0.f, ScaledLocationY * -1.f, ScaledLocationZ),
			FVector(1.f, PrimaryScaleZ, 1.f)), BaseTransform));
	
	ScaleSafeInstance(ISM_Side, UKismetMathLibrary::MakeRelativeTransform(
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
	
	ISM_ShadowStraight->AddInstances(InstanceTransforms, false, false, true);
	InstanceTransforms.Empty();
	
	ISM_ShadowStraightTop->AddInstance(FTransform(
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
	
	ISM_ShadowRound->AddInstances(InstanceTransforms, false, false, true);
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
	
	ISM_TitleBarMain->AddInstance(BaseTransform, false);
	const FVector BaseLocation = BaseTransform.GetLocation();
	
	ISM_TitleBarEnd->AddInstance(FTransform(
			FRotator::ZeroRotator,
			FVector(BaseLocation.X, BaseLocation.Y + (((WidthLimited - 3.f) * 50.f) * -1.f), BaseLocation.Z),
			FVector::OneVector), false);
	
	ISM_TitleBarEndR->AddInstance(FTransform(
		FRotator::ZeroRotator,
		FVector(BaseLocation.X, BaseLocation.Y + ((WidthLimited - 3.f) * 50.f), BaseLocation.Z),
		FVector::OneVector), false); 
}

void ANSamplesDisplayActor::UpdateDescription()
{
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
		// We dont want break on the last
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

	// Set Position
	if (!FinalString.IsEmpty())
	{
		const bool bHeightVSFloorText = (Height < 2.f || (bFloorText && Depth > 2.f));

		FVector Origin = FVector::ZeroVector;
		if (bSeparateTitlePanel)
		{
			Origin = TitleTextTransform().GetLocation();
		}
		else
		{
			Origin = TitleTextComponent->GetRelativeLocation();
		}

		const float TitleSpacerOffset = GetTitleSpacerOffset();
		
		FVector WallLocation = FVector(15.f, TextAlignmentOffset(1.f, false),
			Origin.Z - TitleSpacerOffset);
		FVector FloorLocation = FVector(Origin.X - TitleSpacerOffset, WallLocation.Y, WallLocation.X);

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
		
		DescriptionTextComponent->SetText(FText::FromString(FinalString));
		DescriptionTextComponent->SetHorizontalAlignment(TextAlignment);
		DescriptionTextComponent->SetWorldSize(DescriptionScale);
		DescriptionTextComponent->SetTextRenderColor(GetColor(DescriptionColor));

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
	else
	{
		DescriptionTextComponent->SetVisibility(false);
	}
}

void ANSamplesDisplayActor::UpdateDisplayColor()
{
	if (DisplayMaterial == nullptr)
	{
		DisplayMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, DisplayMaterialInterface, NAME_None, EMIDCreationFlags::Transient);
		
		// Update the static meshes with the dynamic material
		if (DisplayMaterial != nullptr)
		{
			ISM_Main->SetMaterial(0, DisplayMaterial);
			ISM_Corner->SetMaterial(0, DisplayMaterial);
			ISM_Curve->SetMaterial(0, DisplayMaterial);
			ISM_Side->SetMaterial(0, DisplayMaterial);
			ISM_CurveEdge->SetMaterial(1, DisplayMaterial);
		}
	}
	if (DisplayMaterial != nullptr)
	{
		DisplayMaterial->SetVectorParameterValue(FName("Base Color"), GetLinearColor(Color));
	}
}
void ANSamplesDisplayActor::UpdateNotice()
{
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
			NoticeMaterial->SetVectorParameterValue(FName("Base Color"), GetLinearColor(NoticeColor));
		}
		
		NoticeDecalComponent->SetActive(true);
		NoticeDecalComponent->DecalSize = FVector(128.f, (50.f * Width), NoticeDepth);
		NoticeDecalComponent->SetVisibility(true);
		NoticeDecalComponent->SetHiddenInGame(false);

		NoticeTextComponent->SetText(NoticeText);
		NoticeTextComponent->SetWorldSize(NoticeScale);
		NoticeTextComponent->SetTextRenderColor(GetColor(NoticeColor));
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
		TitleTextComponent->SetTextRenderColor(GetColor(TitleColor));
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


void ANSamplesDisplayActor::DefaultInstanceStaticMesh(UInstancedStaticMeshComponent* Instance) const
{
	Instance->SetupAttachment(RootComponent);
	Instance->SetMobility(EComponentMobility::Static);
	Instance->bDisableCollision = true;
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