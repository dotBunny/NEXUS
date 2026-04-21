// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "NWidgetTabIdentifiers.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NEditorUtilityWidgetSubsystem.generated.h"

class UEditorUtilityWidget;
class UEditorUtilityWidgetBlueprint;
class UNEditorUtilityWidget;

/**
 * Editor subsystem that stores per-widget state (FNWidgetState key-value bags) and tab identifier
 * mappings across editor sessions via the NexusUserSettings config. Widgets register themselves in
 * NativeConstruct so the subsystem can push cached state back into them before they go live, and
 * unregister on destruct to avoid dangling references.
 */
UCLASS(config = NexusUserSettings)
class NEXUSUIEDITOR_API UNEditorUtilityWidgetSubsystem : public UEditorSubsystem
{
public:
	GENERATED_BODY()
	N_EDITOR_SUBSYSTEM(UNEditorUtilityWidgetSubsystem)

	/**
	 * Store a new FNWidgetState under Identifier; overwrites any existing entry.
	 * @param Identifier Stable widget key (typically UNEditorUtilityWidget::UniqueIdentifier).
	 * @param WidgetState Key-value bag to persist.
	 */
	void AddWidgetState(const FName& Identifier, const FNWidgetState& WidgetState);

	/**
	 * Discard the stored state associated with Identifier.
	 * @param Identifier Widget key previously passed to AddWidgetState.
	 */
	void RemoveWidgetState(const FName& Identifier);

	/**
	 * Replace the stored state for Identifier with WidgetState; logs a warning if the widget is unknown.
	 * @param Identifier Widget key previously passed to AddWidgetState.
	 * @param WidgetState New state bag that supersedes any prior value.
	 */
	void UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState);

	/** @return True when a state bag exists for Identifier. */
	bool HasWidgetState(const FName& Identifier) const;

	/**
	 * Retrieve a reference to the stored state for Identifier.
	 * @param Identifier Widget key previously passed to AddWidgetState.
	 * @return Mutable reference to the stored state; callers must ensure HasWidgetState first.
	 * @note Not bounds-checked; calling with an unknown identifier will assert.
	 */
	FNWidgetState& GetWidgetState(const FName& Identifier);

	/**
	 * Register a live widget instance with the subsystem so state can be pushed into it.
	 * @param Widget Widget instance to track; keyed by its GetUniqueIdentifier().
	 */
	void RegisterWidget(UNEditorUtilityWidget* Widget);

	/**
	 * Drop a previously-registered widget from the live-instance map.
	 * @param Widget Widget instance to remove; safe to call with widgets that were never registered.
	 */
	void UnregisterWidget(const UNEditorUtilityWidget* Widget);

	/**
	 * Look up a live widget instance by its identifier.
	 * @param Identifier Widget key used at registration time.
	 * @return The widget if currently alive and registered, nullptr otherwise.
	 */
	UNEditorUtilityWidget* GetWidget(const FName& Identifier);

	/** @return True when a widget with Identifier is currently registered as live. */
	bool HasWidget(const FName& Identifier);


	/**
	 * Resolve the tab identifier most recently associated with WidgetIdentifier so the widget can
	 * be re-spawned into the same tab on restore.
	 * @param WidgetIdentifier Stable widget key.
	 * @return The associated tab identifier, or NAME_None if none is known.
	 */
	FName GetTabIdentifier(FName WidgetIdentifier);

	/**
	 * Record that a widget is currently hosted under TabIdentifier.
	 * @param WidgetIdentifier Stable widget key.
	 * @param TabIdentifier Tab identifier in the editor tab manager.
	 */
	void SetTabIdentifier(FName WidgetIdentifier, FName TabIdentifier);

	/**
	 * Drop any tab-identifier mapping for WidgetIdentifier (e.g. when the user closes the tab permanently).
	 * @param WidgetIdentifier Stable widget key.
	 */
	void RemoveTabIdentifier(FName WidgetIdentifier);

protected:
	/** Persisted snapshot of all widget state bags; keyed by widget identifier. */
	UPROPERTY(config)
	FNWidgetStateSnapshot WidgetStates;

	/** Persisted widget-to-tab identifier mapping; used to restore tab placement. */
	UPROPERTY(config)
	FNWidgetTabIdentifiers WidgetTabIdentifiers;
private:
	/** Transient map of currently-alive widget instances keyed by identifier. */
	TMap<FName, TObjectPtr<UNEditorUtilityWidget>> KnownWidgets;
};

