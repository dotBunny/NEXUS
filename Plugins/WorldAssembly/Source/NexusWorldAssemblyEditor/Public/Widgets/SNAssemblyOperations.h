// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/SCompoundWidget.h"

enum class ENWorldAssemblyOperationState : uint8;
class SVerticalBox;
class UNAssemblyOperation;
struct FNStatusChannelUpdate;

/**
 * Declares this widget as invalidation-aware, which SCompoundWidget is not by default.
 *
 * FCategoryDrivenContentBuilderBase wraps the whole mode panel in an SInvalidationPanel with dynamic invalidation on,
 * and such a panel only caches when every descendant opts in. The claim holds here: this widget declares no
 * SlateAttributes of its own and overrides neither OnPaint nor Tick, its per-frame bindings all sit on stock widgets
 * whose registered attributes invalidate correctly, and Rebuild mutates its column through TPanelChildren, which
 * invalidates on both the clear and the re-add. Anything added here that paints or caches state directly has to
 * re-earn it.
 *
 * @note Load-bearing. The mode panel's other non-opting-in widget was FToolkitSections' details view, which
 *       FNWorldAssemblyEdModeToolkit no longer registers — so this widget is now the only thing standing between that
 *       panel and a cached one. Anything added here that paints or caches state directly has to re-earn the claim.
 */
template <>
struct TWidgetTypeTraits<class SNAssemblyOperations>
{
	static constexpr bool SupportsInvalidation() { return true; }
};

/**
 * Live progress for every World Assembly operation the registry currently knows about.
 *
 * The Slate counterpart to UNWorldAssemblyDeveloperOverlay: one block per operation showing its name, task counts,
 * combined progress and status message, with a bar per open status channel beneath. Subscribes to the same two
 * registry broadcasts the overlay does, so it needs no tick of its own.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API SNAssemblyOperations : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNAssemblyOperations) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SNAssemblyOperations() override;

private:
	/**
	 * What one status channel's bar draws from.
	 * @note A copy rather than the FNStatusChannelUpdate it came from: that carries FStrings, and these are read by
	 *       text bindings every frame, so the conversion is done once on arrival instead.
	 */
	struct FNChannelSnapshot
	{
		/** Left-aligned label captured when the channel was opened. */
		FText Label;

		/** Most recent status message published to the channel. */
		FText Message;

		/** Most recent completion percent in the 0..1 range. */
		float Percent = 0.0f;
	};

	/**
	 * The latest drained state of one operation's status channels.
	 * @note Kept here rather than read back off the operation: OnOperationChannelsChanged delivers deltas and the
	 *       operation exposes no channel list, so the only complete picture is the one accumulated from them.
	 */
	struct FNOperationChannels
	{
		/** Channel ids in first-seen order, so a bar keeps its position as later updates arrive. */
		TArray<int32> Order;

		/** Latest state per channel id. */
		TMap<int32, FNChannelSnapshot> States;
	};

	/** Registry callback: an operation registered or unregistered, so the set of blocks has changed. */
	void OnOperationStateChanged(UNAssemblyOperation* Operation, ENWorldAssemblyOperationState NewState);

	/** Registry callback: fold channel deltas into ChannelStates, rebuilding only when a new channel appears. */
	void OnOperationChannelsChanged(UNAssemblyOperation* Operation, const TArray<FNStatusChannelUpdate>& Changes);

	/** Rebuild every block from the registry's current operations. */
	void Rebuild();

	/** @return One operation's block: its heading row, combined progress bar, message, and channel bars. */
	TSharedRef<SWidget> CreateOperationBlock(UNAssemblyOperation* Operation) const;

	/** @return One status channel's bar, reading its state out of ChannelStates every frame. */
	TSharedRef<SWidget> CreateChannelBar(int32 Ticket, int32 ChannelId) const;

	/**
	 * @return true if the operation belongs in the list.
	 * @note Filters the edit mode's own preview operation, which UNWorldAssemblyEdMode keeps registered for as long as
	 *       the mode is open and which would otherwise sit here as a permanently idle block. Same exclusion the
	 *       developer overlay makes.
	 */
	static bool ShouldShowOperation(const UNAssemblyOperation* Operation);

	/** The column of operation blocks, replaced wholesale by Rebuild. */
	TSharedPtr<SVerticalBox> OperationsBox;

	/** Accumulated channel state, keyed by operation ticket. Dropped with the operation when it unregisters. */
	TMap<int32, FNOperationChannels> ChannelStates;

	/** Handle for the registry state-change subscription. */
	FDelegateHandle OperationStateChangedHandle;

	/** Handle for the registry channels-changed subscription. */
	FDelegateHandle OperationChannelsChangedHandle;
};
