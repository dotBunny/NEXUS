// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NTextRenderWarmupSubsystem.h"

#include "Components/TextRenderComponent.h"
#include "Engine/Font.h"
#include "NUIMinimal.h"
#include "Streaming/LevelStreamingDelegates.h"
#include "UObject/UObjectIterator.h"

void UNTextRenderWarmupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LevelBeginMakingVisibleHandle = FLevelStreamingDelegates::OnLevelBeginMakingVisible.AddUObject(
		this, &UNTextRenderWarmupSubsystem::OnLevelBeginMakingVisible);
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UNTextRenderWarmupSubsystem::OnWorldCleanup);
}

void UNTextRenderWarmupSubsystem::Deinitialize()
{
	FLevelStreamingDelegates::OnLevelBeginMakingVisible.Remove(LevelBeginMakingVisibleHandle);
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
	ReleaseAll();

	Super::Deinitialize();
}

void UNTextRenderWarmupSubsystem::PostInitialize()
{
	Super::PostInitialize();

	// InitWorld runs this after the scene exists and before LoadMap registers the persistent level's components.
	WarmLoadedTextRenders();
}

void UNTextRenderWarmupSubsystem::OnLevelBeginMakingVisible(UWorld* InWorld, const ULevelStreaming* InStreamingLevel,
	ULevel* InLevel)
{
	// Broadcast for every world, once per level, before AddToWorld registers any of the level's components. The level
	// has loaded by now, so its text renders are among the loaded ones.
	if (InWorld == GetWorld())
	{
		WarmLoadedTextRenders();
	}
}

void UNTextRenderWarmupSubsystem::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
{
	// Deinitialize is too late when a cleanup keeps its resources: the world validates its components either way.
	if (InWorld == GetWorld())
	{
		ReleaseAll();
	}
}

void UNTextRenderWarmupSubsystem::WarmLoadedTextRenders()
{
	// The engine's list of live text renders (subclasses included, still-loading ones not), copied up front, so warming
	// one (which makes another) cannot disturb it. Class defaults and Blueprint templates are left out: neither is ever
	// registered. A text render of another world, or of a level loaded but not yet visible, warms its pair early, which
	// costs one component.
	const TObjectRange<UTextRenderComponent> TextRenders(RF_ClassDefaultObject | RF_ArchetypeObject, true,
		EInternalObjectFlags::Garbage);
	for (const UTextRenderComponent* Component : TextRenders)
	{
		WarmPair(Component->TextMaterial, Component->Font);
	}
}

bool UNTextRenderWarmupSubsystem::IsWarm(const UMaterialInterface* Material, const UFont* Font) const
{
	return WarmPairs.Contains({ Material, Font });
}

void UNTextRenderWarmupSubsystem::WarmPair(UMaterialInterface* Material, UFont* Font)
{
	// Only an offline (distance field) font draws through the MID cache; a runtime font never asks it.
	if (Font == nullptr || Font->FontCacheType != EFontCacheType::Offline)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr || World->Scene == nullptr)
	{
		return;
	}

	bool bAlreadyWarm = false;
	WarmPairs.Add({ Material, Font }, &bAlreadyWarm);
	if (bAlreadyWarm)
	{
		return;
	}

	// The proxy keys the cache by the material it resolves from the component's, so handing it the same material
	// (null included) warms the same entry. Empty text builds no geometry, so it draws nothing while it stays visible,
	// which it must: a hidden text render is never added to the scene and so never makes a proxy.
	UTextRenderComponent* Component = NewObject<UTextRenderComponent>(World, NAME_None, RF_Transient);
	Component->SetText(FText::GetEmpty());
	Component->SetTextMaterial(Material);
	Component->SetFont(Font);
	Component->SetCastShadow(false);

	// No context, so this adds the primitive, and builds its proxy and the pair's MIDs, here on the game thread.
	Component->RegisterComponentWithWorld(World);
	WarmComponents.Add(Component);

	UE_LOG(LogNexusUI, Verbose, TEXT("Text render MIDs warmed on the game thread for %s with %s."),
		*GetNameSafe(Material), *GetNameSafe(Font));
}

void UNTextRenderWarmupSubsystem::ReleaseAll()
{
	for (UTextRenderComponent* Component : WarmComponents)
	{
		if (IsValid(Component))
		{
			Component->DestroyComponent();
		}
	}
	WarmComponents.Reset();
	WarmPairs.Reset();
}
