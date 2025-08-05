// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NUpdateCheckInstanceObject.generated.h"


UENUM(BlueprintType)
enum ENUpdatesChannel
{
	NUC_GithubMain	= 0 UMETA(DisplayName = "Mainline (GitHub)"),
	NUC_GithubDev	= 1 UMETA(DisplayName = "Development (GitHub)"),
	NUC_Custom		= 6 UMETA(DisplayName = "Custom"),
};

UCLASS()
class NEXUSCOREEDITOR_API UNUpdateCheckInstanceObject : public UObject
{
	GENERATED_BODY()

public:
	static void Create();
	
private:
	
	UFUNCTION()
	void Execute();
	
	const FString MainURI = TEXT("https://raw.githubusercontent.com/dotBunny/NEXUS/refs/heads/main/Plugins/Core/Source/NexusCore/Public/NCoreMinimal.h");
	const FString DevURI = TEXT("https://raw.githubusercontent.com/dotBunny/NEXUS/refs/heads/dev/Plugins/Core/Source/NexusCore/Public/NCoreMinimal.h");
	
	FString GetURI() const;
	
};