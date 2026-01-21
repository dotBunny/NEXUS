// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "HttpFwd.h"
#include "NDelayedEditorTask.h"
#include "NUpdateCheckDelayedEditorTask.generated.h"

UENUM(BlueprintType)
enum class ENUpdatesChannel : uint8
{
	GithubRelease	= 0 UMETA(DisplayName = "Release (GitHub)"),
	GithubMain		= 1 UMETA(DisplayName = "Main (GitHub)"),
	Custom			= 6 UMETA(DisplayName = "Custom"),
};

UCLASS()
class NEXUSCOREEDITOR_API UNUpdateCheckDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	static void Create();
	
private:
	
	static void OnUpdateQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bProcessedSuccessfully);
	
	UFUNCTION()
	void Execute();
	
	const FString ReleaseQueryURI = TEXT("https://raw.githubusercontent.com/dotBunny/NEXUS/refs/heads/release/Plugins/Core/Source/NexusCore/Public/NCoreMinimal.h");
	const FString ReleaseUpdateURI = TEXT("https://github.com/dotBunny/NEXUS/tree/release");
	const FString MainQueryURI = TEXT("https://raw.githubusercontent.com/dotBunny/NEXUS/refs/heads/main/Plugins/Core/Source/NexusCore/Public/NCoreMinimal.h");
	const FString MainUpdateURI = TEXT("https://github.com/dotBunny/NEXUS/tree/main");
	
	FString GetQueryURI() const;
	FString GetUpdateURI() const;
};