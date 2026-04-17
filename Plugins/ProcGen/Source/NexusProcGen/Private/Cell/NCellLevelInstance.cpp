// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellLevelInstance.h"

#include "NProcGenSettings.h"
#include "NProcGenUtils.h"

ANCellLevelInstance::ANCellLevelInstance()
{
	bReplicates = true;
	if (UNProcGenSettings::Get()->NetworkingMode == ENProcGenNetworkMode::AlwaysRelevantLevelInstances)
	{
		bAlwaysRelevant = true;
	}
}

void ANCellLevelInstance::OnLevelInstanceLoaded()
{
	ANCellActor* CellActor = FNProcGenUtils::GetCellActorFromLevel(GetLoadedLevel());
	CellActor->InitializeFromProxy(this);
	Super::OnLevelInstanceLoaded();
}