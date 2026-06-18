// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FPropertySection;

/**
 * Registers and manages the "NEXUS" property sections (the grouping tabs shown at the top of the Details panel)
 * for Actors, Actor Components, Scene Components, and Objects.
 *
 * Categories added before Register() runs are queued and flushed during registration, so callers can request
 * sections during module startup without ordering concerns.
 */
class NEXUSCOREEDITOR_API FNPropertySections
{
public:
	/** Creates the NEXUS property sections with the PropertyEditor module and flushes any categories queued beforehand. */
	static void Register();

	/** Releases the cached property sections, clears any queued categories, and resets registration state so a later Register() rebinds cleanly. */
	static void Unregister();

	/** Adds a category to all four NEXUS property sections (Actor, Actor Component, Scene Component, and Object). */
	static void AddCategory(FName Category);
	/** Adds a category to the Actor property section, queueing it if registration has not yet occurred. */
	static void AddActorCategory(FName Category);
	/** Adds a category to the Actor Component property section, queueing it if registration has not yet occurred. */
	static void AddActorComponentCategory(FName Category);
	/** Adds a category to the Scene Component property section, queueing it if registration has not yet occurred. */
	static void AddSceneComponentCategory(FName Category);
	/** Adds a category to the Object property section, queueing it if registration has not yet occurred. */
	static void AddObjectCategory(FName Category);
	
private:
	static TSharedPtr<FPropertySection> ActorPropertySection;
	static TSharedPtr<FPropertySection> ActorComponentPropertySection;
	static TSharedPtr<FPropertySection> SceneComponentPropertySection;
	static TSharedPtr<FPropertySection> ObjectPropertySection;
	
	static const FText DisplayName;
	static const FName Identifier;
	static const FName BaseCategory;
	
	static bool bHasRegistered;
	
	static TArray<FName> PendingActors;
	static TArray<FName> PendingActorComponents;
	static TArray<FName> PendingSceneComponents;
	static TArray<FName> PendingObjectComponents;
};