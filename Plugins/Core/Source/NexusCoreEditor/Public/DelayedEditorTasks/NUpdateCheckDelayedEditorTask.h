// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "HttpFwd.h"
#include "NDelayedEditorTask.h"
#include "NUpdateCheckDelayedEditorTask.generated.h"

/**
 * Source channel used by the NEXUS framework update-check task.
 */
UENUM(BlueprintType)
enum class ENUpdatesChannel : uint8
{
	/** Latest stable release tag on GitHub. */
	GithubRelease	= 0 UMETA(DisplayName = "Release (GitHub)"),
	/** Bleeding-edge main branch on GitHub. */
	GithubMain		= 1 UMETA(DisplayName = "Main (GitHub)"),
	/** User-provided custom URIs (see UNEditorSettings::UpdatesCustom*URI). */
	Custom			= 6 UMETA(DisplayName = "Custom"),
};

/**
 * Delayed editor task that checks the configured channel for a newer NEXUS release.
 *
 * Fires an HTTP request to read the canonical NCoreMinimal.h from the chosen channel, compares
 * the embedded version number to the current build, and, when newer, surfaces a notification.
 */
UCLASS()
class NEXUSCOREEDITOR_API UNUpdateCheckDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	/** Schedules the update-check task according to the user's configured frequency. */
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