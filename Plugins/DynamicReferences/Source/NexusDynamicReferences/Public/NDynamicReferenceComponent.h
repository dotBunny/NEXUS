// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicReference.h"
#include "Types/NActorComponentLifecycle.h"
#include "NDynamicReferenceComponent.generated.h"

UCLASS(BlueprintType,Blueprintable, ClassGroup="NEXUS", DisplayName="Dynamic Reference",
	meta = (BlueprintSpawnableComponent, Tooltop="Reference an Actor without knowing it."),
	HideCategories=(Tags, Activation, Cooking, AssetUserData, Navigation))
class NEXUSDYNAMICREFERENCES_API UNDynamicReferenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	static FString ToStringSlow(const ENDynamicReference& DynamicReference)
	{
		return StaticEnum<ENDynamicReference>()->GetDisplayNameTextByValue(DynamicReference).ToString();
	}

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

protected:
	/**
	 * What phase of the components lifecycle should the references be linked.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	TEnumAsByte<ENActorComponentLifecycleStart> LinkPhase = ENActorComponentLifecycleStart::BeginPlay;

	/**
	 * What phase of the components lifecycle should the references be broken.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	TEnumAsByte<ENActorComponentLifecycleEnd> BreakPhase = ENActorComponentLifecycleEnd::EndPlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	TArray<TEnumAsByte<ENDynamicReference>> References;
	
private:
	void Register();
	void Unregister();
};
