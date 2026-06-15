// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NDynamicRef.h"
#include "Types/NActorComponentLifecycle.h"
#include "NDynamicRefComponent.generated.h"

/**
 * A component which registers and unregisters the owning AActor with the UNDynamicRefsSubsystem for future lookup.
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-component/">UNDynamicRefComponent</a>
 */
UCLASS(BlueprintType,Blueprintable, ClassGroup="NEXUS", DisplayName = "NEXUS | DynamicRef",
	meta = (BlueprintSpawnableComponent, Tooltip="Reference an Actor without knowing it."),
	HideCategories=(Tags, Activation, Cooking, AssetUserData, Navigation))
class NEXUSDYNAMICREFS_API UNDynamicRefComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Resolve an ENDynamicRef value to its human-readable display name via UE reflection.
	 * @param DynamicReference The enum value to convert.
	 * @return The display name (e.g. "Objective A").
	 * @note Uses reflection; avoid in tight loops.
	 */
	static FString ToStringSlow(const ENDynamicRef& DynamicReference)
	{
		return StaticEnum<ENDynamicRef>()->GetDisplayNameTextByValue(DynamicReference).ToString();
	}

	UNDynamicRefComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

protected:
	
	/** Lifecycle phase at which this Actor registers/unregisters with the subsystem (BeginPlay vs InitializeComponent). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DynamicRef")
	ENActorComponentLifecycle Lifecycle = ENActorComponentLifecycle::BeginPlay;

	/** The set of ENDynamicRef slot identifiers this Actor will claim (fast array-backed lookup). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DynamicRef")
	TArray<TEnumAsByte<ENDynamicRef>> FastReferences;

	/** The set of named identifiers this Actor will claim (map-backed lookup). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DynamicRef")
	TArray<FName> NamedReferences;
	
	/** The set of gameplay-tag identifiers this Actor will claim (tag-container lookup). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DynamicRef")
	FGameplayTagContainer TagReferences;
	
private:
	/** Register this Actor with UNDynamicRefSubsystem under all configured references. */
	void Register();
	/** Unregister this Actor from UNDynamicRefSubsystem for all configured references. */
	void Unregister();
};
