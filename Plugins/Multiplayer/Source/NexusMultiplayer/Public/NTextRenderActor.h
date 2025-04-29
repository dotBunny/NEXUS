// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TextRenderActor.h"
#include "GameFramework/Actor.h"
#include "NTextRenderActor.generated.h"



UCLASS()
class NEXUSMULTIPLAYER_API ANTextRenderActor final : public ATextRenderActor
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTextChanged, FString, NewText);
	
	GENERATED_BODY()

	explicit ANTextRenderActor(const FObjectInitializer& Initializer);
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Replicated", ReplicatedUsing=OnRep_TextValue)
	FString CachedValue;

	UFUNCTION()
	void OnRep_TextValue();
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "NEXUS")
	void SetFromName(const FName& NewValue);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "NEXUS")
	void SetFromString(const FString& NewValue);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "NEXUS")
	void SetFromText(const FText& NewValue);

	UPROPERTY(BlueprintAssignable)
	FOnTextChanged OnTextChanged;
};
