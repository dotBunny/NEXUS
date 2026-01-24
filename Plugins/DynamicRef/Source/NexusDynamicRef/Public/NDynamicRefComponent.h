// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicRef.h"
#include "Types/NActorComponentLifecycle.h"
#include "NDynamicRefComponent.generated.h"

/**
 * A component which registers and unregisters the owning AActor with the UNDynamicRefSubsystem for future lookup.
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ptr-component/">UNDynamicRefComponent</a>
 */
UCLASS(BlueprintType,Blueprintable, ClassGroup="NEXUS", DisplayName="DynamicRef Component",
	meta = (BlueprintSpawnableComponent, Tooltop="Reference an Actor without knowing it."),
	HideCategories=(Tags, Activation, Cooking, AssetUserData, Navigation))
class NEXUSDYNAMICREF_API UNDynamicRefComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	static FString ToStringSlow(const ENDynamicRef& DynamicReference)
	{
		return StaticEnum<ENDynamicRef>()->GetDisplayNameTextByValue(DynamicReference).ToString();
	}

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

protected:
	/**
	 * What phase of the component's lifecycle should the references be linked?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	ENActorComponentLifecycleStart LinkPhase = ENActorComponentLifecycleStart::BeginPlay;

	/**
	 * What phase of the component's lifecycle should the references be broken?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	ENActorComponentLifecycleEnd BreakPhase = ENActorComponentLifecycleEnd::EndPlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	TArray<TEnumAsByte<ENDynamicRef>> FastReferences;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dynamic Reference")
	TArray<FName> NamedReferences;
	
private:
	void Register();
	void Unregister();
};
