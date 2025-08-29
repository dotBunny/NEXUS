// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "NTextRenderComponent.generated.h"



UCLASS(meta = (BlueprintSpawnableComponent))
class NEXUSMULTIPLAYER_API UNTextRenderComponent final : public UTextRenderComponent
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTextChanged, FString, NewText);
	
	explicit UNTextRenderComponent(const FObjectInitializer& Initializer);

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_TextValue)
	FString CachedValue;

	UFUNCTION()
	void OnRep_TextValue();
	
	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set From Name (Server)", Category = "NEXUS|Multiplayer")
	void Server_SetFromName(const FName& NewValue);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set From String (Server)", Category = "NEXUS|Multiplayer")
	void Server_SetFromString(const FString& NewValue);

	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set From Text (Server)", Category = "NEXUS|Multiplayer")
	void Server_SetFromText(const FText& NewValue);
	
	UFUNCTION(BlueprintCallable, DisplayName="Set From Name", Category = "NEXUS|Multiplayer")
	void SetFromName(const FName& NewValue);

	UFUNCTION(BlueprintCallable, DisplayName="Set From String", Category = "NEXUS|Multiplayer")
	void SetFromString(const FString& NewValue);

	UFUNCTION(BlueprintCallable, DisplayName="Set From Text",  Category = "NEXUS|Multiplayer")
	void SetFromText(const FText& NewValue);

	/**
	 * Allow Clients to use RPCs to change text.
	 */
	UPROPERTY(BlueprintReadWrite)
	bool bAllowRPC = false;
	
	UPROPERTY(BlueprintAssignable)
	FOnTextChanged OnTextChanged;	
};