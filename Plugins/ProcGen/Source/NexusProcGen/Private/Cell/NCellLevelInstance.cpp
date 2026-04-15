// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellLevelInstance.h"

#include "NProcGenUtils.h"

ANCellLevelInstance::ANCellLevelInstance()
{
	// We could setup a global setting to disable replication, some folks might want both sides generating?
	bReplicates = true;
}

void ANCellLevelInstance::OnLevelInstanceLoaded()
{
	ANCellActor* CellActor = FNProcGenUtils::GetCellActorFromLevel(GetLoadedLevel());
	
	CellActor->InitializeFromProxy();
	
	Super::OnLevelInstanceLoaded();
}