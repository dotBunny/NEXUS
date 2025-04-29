// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferenceSubsystem.h"

void UNDynamicReferenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ReferenceMap.AddZeroed(NDR_Max);
	Super::Initialize(Collection);
}

void UNDynamicReferenceSubsystem::Deinitialize()
{
	ReferenceMap.Empty();
	Super::Deinitialize();
}

void UNDynamicReferenceSubsystem::RegisterReference(ENDynamicReference InType, AActor* InActor)
{
	ReferenceMap[InType].AddUnique(InActor);
}

void UNDynamicReferenceSubsystem::UnregisterReference(ENDynamicReference InType, AActor* InActor)
{
	ReferenceMap[InType].Remove(InActor);
}
