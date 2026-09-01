// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "IDetailCustomization.h"

class ANWorldCollisionCacheActor;
class IPropertyUtilities;
class UWorld;

/**
 * Detail-panel customization for ANWorldCollisionCacheActor.
 *
 * The actor holds one property — the pool — and it is not editable, so without this its panel is blank. What is worth
 * having there is not the data but the three things you would want to do while looking at it: see whether the level's
 * bake is current, put the collision visualizer up against it, and re-bake.
 *
 * Deliberately the same actions the World rail offers, routed through the same code — FNWorldAssemblyEditorUtils for
 * the bake, UNWorldAssemblyEditorSubsystem for the visualizer. The difference is reach: this panel is one Outliner
 * click away whether or not the World Assembly edit mode is up, where the rail exists only inside it.
 * @see FNWorldEdModeRail
 */
class FNWorldCollisionCacheActorCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual ~FNWorldCollisionCacheActorCustomization() override;

	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization

private:
	/**
	 * @return The world of the cache actor being customized, or nullptr when the selection is gone or covers more
	 *         than one.
	 * @note Single-select only, and not a limitation worth working around: there is one cache actor per level, so a
	 *       multi-selection means several levels' caches at once, and every action here is level-scoped.
	 */
	UWorld* GetCustomizedWorld() const;

	/** @return A read-only description of what the level's pool holds and whether it still matches the world. */
	FText GetCacheSummary() const;

	/** @return "Remove Visualizer" while one is alive, "Create Visualizer" otherwise. */
	FText GetVisualizerButtonText() const;

	/** @return The tooltip matching whichever action GetVisualizerButtonText is promising. */
	FText GetVisualizerButtonToolTip() const;

	/** Spawn the world-collision visualizer for this level, or destroy it if one is already alive. */
	FReply OnToggleVisualizerClicked();

	/** Select the live visualizer actor, replacing the current selection. */
	FReply OnSelectVisualizerClicked();

	/** @return true while a visualizer is alive to be selected. */
	bool CanSelectVisualizer() const;

	/** Bake every organ in the level into the cache, regardless of whether their fingerprints still match. */
	FReply OnBakeClicked();

	/** @return true when the level can be baked: not in PIE, and the level holds an organ to bake. */
	bool CanBake() const;

	/** Force the panel to re-poll its bound attributes. */
	void RequestRefresh() const;

	/** The cache actor(s) this panel instance is bound to. */
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;

	/** Property utilities for the panel we customized, used to force a refresh. */
	TWeakPtr<IPropertyUtilities> PropertyUtilities;

	/**
	 * Handle for our subscription to UNWorldAssemblyEditorSubsystem::OnCollisionVisualizerChanged.
	 * @note What keeps the toggle's label honest when the visualizer is toggled from somewhere else — the World
	 *       rail's tile drives the same visualizer, and it is reachable while this panel is on screen.
	 */
	FDelegateHandle VisualizerChangedHandle;

	/** Handle for our subscription to FNWorldCollisionBaker::OnBaked, which is what makes the summary re-read. */
	FDelegateHandle BakedHandle;
};
