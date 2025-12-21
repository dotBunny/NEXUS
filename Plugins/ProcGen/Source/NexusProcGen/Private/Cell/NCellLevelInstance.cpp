// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellLevelInstance.h"
#include "NProcGenUtils.h"

void ANCellLevelInstance::OnLevelInstanceLoaded()
{
	ANCellActor* CellActor = FNProcGenUtils::GetCellActorFromLevel(GetLoadedLevel());
	CellActor->InitializeFromProxy();
	
	Super::OnLevelInstanceLoaded();
}
