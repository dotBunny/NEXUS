// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "NTextRenderComponent.generated.h"


/**
 * A component that builds a network-synchronized UTextRenderComponent between clients.
 * @see <a href="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/">UNTextRenderComponent</a>
 */
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

	/**
	 * Set the value of the text component from an FName via RPC.
	 * @remark Client setter requires bAllowRPC to be true.
	 * @param NewValue The desired text value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set From Name (Server)", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/#set-from-name"))
	void Server_SetFromName(const FName& NewValue);

	/**
	 * Set the value of the text component from an FString via RPC.
	 * @remark Client setter requires bAllowRPC to be true.
	 * @param NewValue The desired text value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set From String (Server)", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/#set-from-string"))
	void Server_SetFromString(const FString& NewValue);

	/**
	 * Set the value of the text component from an FText via RPC.
	 * @remark Client setter requires bAllowRPC to be true.
	 * @param NewValue The desired text value.
	 */	
	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set From Text (Server)", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/#set-from-text"))
	void Server_SetFromText(const FText& NewValue);

	/**
	 * Set the value of the text component from an FName.
	 * @param NewValue The desired text value.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName="Set From Name", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/#set-from-name"))
	void SetFromName(const FName& NewValue);

	/**
	 * Set the value of the text component from an FString.
	 * @param NewValue The desired text value.
	 */		
	UFUNCTION(BlueprintCallable, DisplayName="Set From String", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/#set-from-string"))
	void SetFromString(const FString& NewValue);

	/**
	 * Set the value of the text component from an FText.
	 * @param NewValue The desired text value.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName="Set From Text",  Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/text-render-component/#set-from-text"))
	void SetFromText(const FText& NewValue);

	/**
	 * Allow Clients to use RPCs to change text.
	 */
	UPROPERTY(BlueprintReadWrite)
	bool bAllowRPC = false;

	/**
	 * Delegate fired when the text is changed via any of the provided methods.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnTextChanged OnTextChanged;	
};