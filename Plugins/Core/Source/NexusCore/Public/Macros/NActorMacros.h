// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// #SONARQUBE-DISABLE This file is ignored from duplication checks the duplication is intended

/**
 * Declares an editor-only UBillboardComponent property named SpriteComponent inside a UCLASS.
 *
 * Use in the class declaration (adds a private UPROPERTY member). Pair with the matching
 * N_WORLD_ICON_IMPLEMENTATION_* macro in the constructor and N_WORLD_ICON_CLEANUP in OnDestroy
 * to give the actor/component a world-space icon that is only present in the editor.
 */
#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_HEADER() \
	private: \
		UPROPERTY() \
		TObjectPtr<class UBillboardComponent> SpriteComponent;
#else // !WITH_EDITORONLY_DATA
#define N_WORLD_ICON_HEADER()
#endif // WITH_EDITORONLY_DATA

/**
 * Destroys the SpriteComponent created by N_WORLD_ICON_HEADER/N_WORLD_ICON_IMPLEMENTATION_*.
 *
 * Call from the owning actor's OnDestroy / component's destruction path. The macro no-ops when
 * the outer object is already tearing down its hierarchy so it is safe to call unconditionally.
 *
 * @param bDestroyingHierarchy Mirrors the AActor destruction flag — when true the cleanup is skipped.
 */
#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_CLEANUP(bDestroyingHierarchy) \
	if (!bDestroyingHierarchy && IsValid(SpriteComponent) && !SpriteComponent->IsBeingDestroyed()) \
	{ \
		SpriteComponent->DestroyComponent(); \
		SpriteComponent = nullptr; \
	}
#else // !WITH_EDITORONLY_DATA
#define N_WORLD_ICON_CLEANUP()
#endif // WITH_EDITORONLY_DATA

/**
 * Creates and configures the editor-only SpriteComponent when the owning actor exposes a USceneComponent attach point.
 *
 * @param PackagePath String path to the UTexture2D used as the sprite.
 * @param AttachPoint USceneComponent* the sprite will be attached to.
 * @param bIsStatic When true, sets the sprite mobility to Static.
 * @param Scale Uniform scale applied to the sprite's relative transform.
 */
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
#else // !WITH_EDITORONLY_DATA
#define N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale)
#endif // WITH_EDITORONLY_DATA



/**
 * Variant of N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT for owners whose attach point is a UBrushComponent.
 *
 * @param PackagePath String path to the UTexture2D used as the sprite.
 * @param AttachPoint UBrushComponent* the sprite will be attached to.
 * @param bIsStatic When true, sets the sprite mobility to Static.
 * @param Scale Uniform scale applied to the sprite's relative transform.
 */
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
#else // !WITH_EDITORONLY_DATA
#define N_WORLD_ICON_IMPLEMENTATION_BRUSH_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale)
#endif // WITH_EDITORONLY_DATA

// #SONARQUBE-ENABLE