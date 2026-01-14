// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Organ/NOrganVolume.h"

class UNCell;

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENProcGenSelectionFlags : uint8
{
	None = 0,
	CellActor = 1 << 0,
	OrganVolume = 1 << 1
};
ENUM_CLASS_FLAGS(ENProcGenSelectionFlags)

class NEXUSPROCGENEDITOR_API FNProcGenEditorUtils
{
public:
	static bool IsCellActorPresentInCurrentWorld();
	static ANCellActor* GetCellActorFromCurrentWorld();
	static bool IsCellActorSelected();
	static TArray<ANCellActor*> GetSelectedCellActors();
	static bool IsOrganVolumeSelected();
	static bool IsOrganComponentPresentInCurrentWorld();
	static TArray<ANOrganVolume*> GetSelectedOrganVolumes();
	static TArray<UNOrganComponent*> GetSelectedOrganComponents();
	static ENProcGenSelectionFlags GetSelectionFlags();
	
	static void SaveCell(UWorld* World, ANCellActor* CellActor = nullptr);
	static bool UpdateCell(UNCell* Cell, ANCellActor* CellActor);

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