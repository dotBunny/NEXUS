// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCellActorFactory.h"
#include "Cell/NCell.h"
#include "Cell/NCellProxy.h"

UNCellActorFactory::UNCellActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("NexusProcGenEditor", "NCellFactoryDisplayName", "NCell Proxy");
	NewActorClass = ANCellProxy::StaticClass();
}

bool UNCellActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.IsInstanceOf(UNCell::StaticClass()))
	{
		return true;
	}
	return false;
}

void UNCellActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	ANCellProxy* Proxy = Cast<ANCellProxy>(NewActor);
	Proxy->InitializeFromNCell(Cast<UNCell>(Asset));
}

FString UNCellActorFactory::GetDefaultActorLabel(UObject* InAsset) const
{
	return InAsset->GetName().Mid(0, InAsset->GetName().Len() - 5) + TEXT("0");
}
