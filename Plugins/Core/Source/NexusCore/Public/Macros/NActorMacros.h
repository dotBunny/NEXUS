// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// #SONARQUBE-DISABLE: This file is ignored from duplication checks the duplication is intended

#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_HEADER() \
	private: \
		UPROPERTY() \
		TObjectPtr<class UBillboardComponent> SpriteComponent;
#else
#define N_WORLD_ICON_HEADER()
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_CLEANUP(bDestroyingHierarchy) \
	if (!bDestroyingHierarchy && SpriteComponent != nullptr && !SpriteComponent->IsBeingDestroyed()) \
	{ \
		SpriteComponent->DestroyComponent(); \
		SpriteComponent = nullptr; \
	}
#else
#define N_WORLD_ICON_CLEANUP()
#endif

#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale) \
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite")); \
	if (!IsRunningCommandlet() && (SpriteComponent != nullptr)) \
	{ \
		struct FConstructorStatics \
		{ \
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture; \
			FName ID_Info; \
			FText NAME_Info; \
			FConstructorStatics() \
				: SpriteTexture(TEXT(PackagePath)) \
				, ID_Info(TEXT("Info")) \
				, NAME_Info(NSLOCTEXT("SpriteCategory", "Info", "Info")) \
			{} \
		}; \
		static FConstructorStatics ConstructorStatics; \
		SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get(); \
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Info; \
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Info; \
		SpriteComponent->bIsScreenSizeScaled = true; \
		SpriteComponent->bHiddenInGame = true; \
		SpriteComponent->SetVisibleFlag(true); \
		SpriteComponent->AttachToComponent(AttachPoint, FAttachmentTransformRules::KeepRelativeTransform); \
		SpriteComponent->SetRelativeLocation(FVector::ZeroVector); \
		SpriteComponent->SetRelativeScale3D(FVector(Scale, Scale, Scale)); \
		if(bIsStatic) { \
			SpriteComponent->Mobility = EComponentMobility::Static; \
		} \
		SpriteComponent->SetIsVisualizationComponent(true); \
		SpriteComponent->bReceivesDecals = false; \
	}
#else
#define N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale)
#endif // WITH_EDITORONLY_DATA



#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_IMPLEMENTATION_BRUSH_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale) \
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite")); \
	if (!IsRunningCommandlet() && (SpriteComponent != nullptr)) \
	{ \
		struct FConstructorStatics \
		{ \
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture; \
			FName ID_Info; \
			FText NAME_Info; \
			FConstructorStatics() \
				: SpriteTexture(TEXT(PackagePath)) \
				, ID_Info(TEXT("Info")) \
				, NAME_Info(NSLOCTEXT("SpriteCategory", "Info", "Info")) \
			{} \
		}; \
		static FConstructorStatics ConstructorStatics; \
		SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get(); \
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Info; \
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Info; \
		SpriteComponent->bIsScreenSizeScaled = true; \
		SpriteComponent->bHiddenInGame = true; \
		SpriteComponent->SetVisibleFlag(true); \
		SpriteComponent->AttachToComponent(AttachPoint, FAttachmentTransformRules::KeepRelativeTransform); \
		SpriteComponent->SetRelativeLocation(FVector::ZeroVector); \
		SpriteComponent->SetRelativeScale3D(FVector(Scale, Scale, Scale)); \
		if(bIsStatic) { \
			SpriteComponent->Mobility = EComponentMobility::Static; \
		} \
		SpriteComponent->SetIsVisualizationComponent(true); \
		SpriteComponent->bReceivesDecals = false; \
	}
#else
#define N_WORLD_ICON_IMPLEMENTATION_BRUSH_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale)
#endif // WITH_EDITORONLY_DATA

// #SONARQUBE-ENABLE