// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCellActorFactory.h"
#include "Cell/NCell.h"
#include "Cell/NCellProxy.h"

UNCellActorFactory::UNCellActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("NexusWorldAssemblyEditor", "NCellFactoryDisplayName", "NCell Proxy");
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
	if (ANCellProxy* Proxy = Cast<ANCellProxy>(NewActor))
	{
		// InitializeFromNCell null-checks the cell internally, so a non-UNCell Asset is handled gracefully.
		Proxy->InitializeFromNCell(Cast<UNCell>(Asset));
	}
}

FString UNCellActorFactory::GetDefaultActorLabel(UObject* InAsset) const
{
	// Cell assets follow the <Name>_NCell convention; strip the suffix and let the engine append the
	// numeric disambiguator via SetActorLabelUnique (this label must not include one itself).
	FString Label = InAsset->GetName();
	Label.RemoveFromEnd(TEXT("_NCell"));
	return Label;
}
