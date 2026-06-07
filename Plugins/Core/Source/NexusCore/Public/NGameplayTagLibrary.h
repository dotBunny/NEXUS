// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NGameplayTagLibrary.generated.h"

/**
 * Blueprint function library that exposes FGameplayTagContainer queries as branch-able execution nodes.
 *
 * Each helper wraps a standard container query and uses ExpandBoolAsExecs so the boolean result drives
 * true/false execution pins directly, removing the need for a separate Branch node in Blueprint graphs.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | GameplayTag Library")
class UNGameplayTagLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** Branch on whether TagContainer holds Tag using an exact match (ignores tag hierarchy). 
	 * @param TagContainer The container to query. 
	 * @param Tag The tag to look for. 
	 * @return true if the container holds exactly Tag. */
	UFUNCTION(BlueprintCallable, DisplayName="Has Exact Tag ?", Category = "NEXUS|GameplayTags", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool HasExactTagExec(const FGameplayTagContainer& TagContainer, const FGameplayTag Tag)
	{
		return TagContainer.HasTagExact(Tag);
	};

	/** Branch on whether TagContainer holds Tag, honoring tag hierarchy (a container tag that is a child of Tag also matches). 
	 * @param TagContainer The container to query. 
	 * @param Tag The tag to look for. 
	 * @return true if the container holds Tag or a descendant of it. */
	UFUNCTION(BlueprintCallable, DisplayName="Has Tag ?", Category = "NEXUS|GameplayTags", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool HasTagExec(const FGameplayTagContainer& TagContainer, const FGameplayTag Tag)
	{
		return TagContainer.HasTag(Tag);
	};
	
	/** Branch on whether TagContainer holds any of Tags using an exact match (ignores tag hierarchy).
	 * @param TagContainer The container to query.
	 * @param Tags The tags to look for.
	 * @return true if the container holds at least one tag in Tags exactly. */
	UFUNCTION(BlueprintCallable, DisplayName="Has Any Exact Tags ?", Category = "NEXUS|GameplayTags", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool HasAnyTagExactExec(const FGameplayTagContainer& TagContainer, const FGameplayTagContainer Tags)
	{
		return TagContainer.HasAnyExact(Tags);
	};

	/** Branch on whether TagContainer holds any of Tags, honoring tag hierarchy (a container tag that is a child of a queried tag also matches).
	 * @param TagContainer The container to query.
	 * @param Tags The tags to look for.
	 * @return true if the container holds at least one tag in Tags or a descendant of it. */
	UFUNCTION(BlueprintCallable, DisplayName="Has Any Tags ?", Category = "NEXUS|GameplayTags", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool HasAnyTagExec(const FGameplayTagContainer& TagContainer, const FGameplayTagContainer Tags)
	{
		return TagContainer.HasAny(Tags);
	};
};
