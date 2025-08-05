// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.


#pragma once
#include "NDelayedEditorTask.h"
#include "NUpdateCheckDelayedEditorTask.generated.h"


UENUM(BlueprintType)
enum ENUpdatesChannel
{
	NUC_GithubMain	= 0 UMETA(DisplayName = "Mainline (GitHub)"),
	NUC_GithubDev	= 1 UMETA(DisplayName = "Development (GitHub)"),
	NUC_Custom		= 6 UMETA(DisplayName = "Custom"),
};

UCLASS()
class NEXUSCOREEDITOR_API UNUpdateCheckDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	static void Create();
	
private:
	
	UFUNCTION()
	void Execute();
	
	const FString MainQueryURI = TEXT("https://raw.githubusercontent.com/dotBunny/NEXUS/refs/heads/main/Plugins/Core/Source/NexusCore/Public/NCoreMinimal.h");
	const FString MainUpdateURI = TEXT("https://github.com/dotBunny/NEXUS/tree/main");
	const FString DevQueryURI = TEXT("https://raw.githubusercontent.com/dotBunny/NEXUS/refs/heads/dev/Plugins/Core/Source/NexusCore/Public/NCoreMinimal.h");
	const FString DevUpdateURI = TEXT("https://github.com/dotBunny/NEXUS/tree/dev");
	
	FString GetQueryURI() const;
	FString GetUpdateURI() const;
};