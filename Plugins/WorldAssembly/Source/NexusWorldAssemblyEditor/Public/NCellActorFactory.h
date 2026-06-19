// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ActorFactories/ActorFactory.h"
#include "NCellActorFactory.generated.h"

/**
 * Actor factory for ANCellProxy — spawns a cell proxy from a UNCell asset drag-drop and initializes
 * it from the dropped cell via ANCellProxy::InitializeFromNCell.
 */
UCLASS()
class UNCellActorFactory : public UActorFactory
{
	GENERATED_BODY()

public:
	explicit UNCellActorFactory(const FObjectInitializer& ObjectInitializer);

	//~UActorFactory
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//End UActorFactory

protected:
	//~UActorFactory
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual FString GetDefaultActorLabel(UObject* InAsset) const override;
	//End UActorFactory
};
