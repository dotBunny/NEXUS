// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "UObject/ConstructorHelpers.h"

// #SONARQUBE-DISABLE This file is ignored from duplication checks the duplication is intended

/**
 * Declares an editor-only UBillboardComponent property named SpriteComponent inside a UCLASS.
 *
 * Use in the class declaration (adds a private UPROPERTY member). Pair with the matching
 * N_WORLD_ICON_* macro in the constructor and N_WORLD_ICON_CLEANUP in OnDestroy
 * to give the actor a world-space icon that is only present in the editor.
 *
 * @warning For AActor only. A UActorComponent wants N_WORLD_ICON_COMPONENT_HEADER and the register-time
 *          macros — see the warning on N_WORLD_ICON_SCENE_COMPONENT for why.
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
 * Declares the editor-only SpriteComponent property for a UActorComponent that builds its icon at register time.
 *
 * Transient, unlike the actor form: the icon is rebuilt on every register and must never reach a package. Pair
 * with N_WORLD_ICON_ON_REGISTER in the component's OnRegister and N_WORLD_ICON_CLEANUP in its
 * OnComponentDestroyed.
 */
#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_COMPONENT_HEADER() \
	private: \
		UPROPERTY(Transient) \
		TObjectPtr<class UBillboardComponent> SpriteComponent;
#else // !WITH_EDITORONLY_DATA
#define N_WORLD_ICON_COMPONENT_HEADER()
#endif // WITH_EDITORONLY_DATA

/**
 * Destroys the SpriteComponent created by N_WORLD_ICON_HEADER/N_WORLD_ICON_*.
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
#define N_WORLD_ICON_CLEANUP(bDestroyingHierarchy)
#endif // WITH_EDITORONLY_DATA

/**
 * Creates and configures the editor-only SpriteComponent when the owning actor exposes a USceneComponent attach point.
 *
 * @warning AActor constructors only. From a UActorComponent constructor this builds a default subobject of a
 *          *component*, which is serialized with its outer — so when that component is an SCS node on a Blueprint,
 *          the sprite's AttachParent is copied off the component template and points at the template. Nothing
 *          remaps it to the instance (AttachParent is not an instanced reference; the SCS sets it on instances
 *          directly, which is why ordinary components never hit this), so registering it trips the "Template
 *          Mismatch during attachment" ensure in USceneComponent::AttachToComponent on every load. Use
 *          N_WORLD_ICON_ON_REGISTER instead, which never serializes the icon at all.
 *
 * @param PackagePath String path to the UTexture2D used as the sprite.
 * @param AttachPoint USceneComponent* the sprite will be attached to.
 * @param bIsStatic When true, sets the sprite mobility to Static.
 * @param Scale Uniform scale applied to the sprite's relative transform.
 */
#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_SCENE_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale) \
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
#define N_WORLD_ICON_SCENE_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale)
#endif // WITH_EDITORONLY_DATA



/**
 * Variant of N_WORLD_ICON_SCENE_COMPONENT for owners whose attach point is a UBrushComponent.
 *
 * @param PackagePath String path to the UTexture2D used as the sprite.
 * @param AttachPoint UBrushComponent* the sprite will be attached to.
 * @param bIsStatic When true, sets the sprite mobility to Static.
 * @param Scale Uniform scale applied to the sprite's relative transform.
 */
#if WITH_EDITORONLY_DATA
#define N_WORLD_ICON_BRUSH_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale) \
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
#define N_WORLD_ICON_BRUSH_COMPONENT(PackagePath, AttachPoint, bIsStatic, Scale)
#endif // WITH_EDITORONLY_DATA



/**
 * Builds, configures and registers the editor-only SpriteComponent for a UActorComponent, attaching it to the
 * component itself.
 *
 * Call from the component's OnRegister, before Super::OnRegister — the icon registers in turn and wants its parent
 * to already know its place in the hierarchy. Pair with N_WORLD_ICON_COMPONENT_HEADER and N_WORLD_ICON_CLEANUP.
 *
 * The icon is built here rather than in the constructor, and outered to the owning actor rather than to the
 * component, so that it is never a default subobject of a component and never serialized — which is what keeps the
 * template/instance mismatch described on N_WORLD_ICON_SCENE_COMPONENT from arising at all. It is the pattern the
 * engine uses for USceneCaptureComponent's ProxyMeshComponent, for the same reason.
 *
 * The icon is always Movable, with no bIsStatic counterpart to the constructor macros. A Movable child attaches to
 * a parent of any mobility, while a Static one under a Movable parent is refused outright — and NEXUS marker
 * components force themselves Static, so the icon must not follow suit.
 *
 * Requires <Components/BillboardComponent.h>, <Engine/CollisionProfile.h>, <Engine/Texture2D.h>, <Engine/World.h>
 * and <GameFramework/Actor.h> at the call site.
 *
 * @param PackagePath String path to the UTexture2D used as the sprite.
 * @param Scale Uniform world scale applied to the icon, independent of the component's own scale.
 */
#if WITH_EDITOR
#define N_WORLD_ICON_ON_REGISTER(PackagePath, Scale) \
	{ \
		/* A rebuild around us — RerunConstructionScripts, an undone paste — can leave this pointing at something */ \
		/* already on its way out. Treat that as absent, or the guard below skips the rebuild and the icon is lost. */ \
		if (!IsValid(SpriteComponent)) \
		{ \
			SpriteComponent = nullptr; \
		} \
		/* No owner means a class default object or a component template: nothing to draw into, and exactly the */ \
		/* objects the icon must not be built onto. IsListedInSceneOutliner drops the rest — an actor hidden from */ \
		/* the outliner is not one being authored. */ \
		AActor* NWorldIconOwner = GetOwner(); \
		if (SpriteComponent == nullptr && NWorldIconOwner != nullptr && !IsRunningCommandlet() \
			&& NWorldIconOwner->IsListedInSceneOutliner()) \
		{ \
			SpriteComponent = NewObject<UBillboardComponent>(NWorldIconOwner, NAME_None, \
				RF_Transactional | RF_TextExportTransient); \
			SpriteComponent->Sprite = LoadObject<UTexture2D>(nullptr, TEXT(PackagePath)); \
			SpriteComponent->SpriteInfo.Category = TEXT("Info"); \
			SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("SpriteCategory", "Info", "Info"); \
			SpriteComponent->bIsScreenSizeScaled = true; \
			SpriteComponent->bHiddenInGame = true; \
			SpriteComponent->bReceivesDecals = false; \
			SpriteComponent->SetIsVisualizationComponent(true); \
			SpriteComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName); \
			/* Marker components carry an authored scale that their readers forward downstream, so the icon has to */ \
			/* sit outside it or a 10x marker gets a 10x icon. bIsScreenSizeScaled only holds the drawn size */ \
			/* constant against camera distance; it does not undo the parent's scale. */ \
			SpriteComponent->SetUsingAbsoluteScale(true); \
			SpriteComponent->SetRelativeScale3D(FVector(Scale)); \
			SpriteComponent->SetupAttachment(this); \
			SpriteComponent->CreationMethod = CreationMethod; \
			SpriteComponent->RegisterComponentWithWorld(GetWorld()); \
		} \
	}
#else // !WITH_EDITOR
#define N_WORLD_ICON_ON_REGISTER(PackagePath, Scale)
#endif // WITH_EDITOR

// #SONARQUBE-ENABLE