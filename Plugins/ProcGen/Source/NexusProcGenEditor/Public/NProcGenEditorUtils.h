// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"

class UNCell;

class NEXUSPROCGENEDITOR_API FNProcGenEditorUtils
{
public:
	static bool IsNCellActorPresentInCurrentWorld();
	static ANCellActor* GetNCellActorFromCurrentWorld();
	static bool IsNCellActorSelected();
	
	static void SaveNCell(UWorld* World, ANCellActor* CellActor = nullptr);
	static bool UpdateNCell(UNCell* Cell, ANCellActor* CellActor);

	FORCEINLINE static bool EffectsGeneratedData(const AActor* ContextActor)
	{
		if (ContextActor == nullptr) return false;
		if (ContextActor->FindComponentByClass<UNCellRootComponent>() != nullptr || ContextActor->FindComponentByClass<UNCellJunctionComponent>() != nullptr) return true;
		
		return false;
	}
	FORCEINLINE static bool EffectsGeneratedData(const UActorComponent* ContextActorComponent)
	{
		if (ContextActorComponent == nullptr) return false;
		return ContextActorComponent->IsA<UNCellRootComponent>() || ContextActorComponent->IsA<UNCellJunctionComponent>();
	}
};