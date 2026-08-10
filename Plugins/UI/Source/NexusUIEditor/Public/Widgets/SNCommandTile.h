// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Styling/AppStyle.h"
#include "Styling/ToolBarStyle.h"
#include "Widgets/SCompoundWidget.h"

class FUICommandInfo;
class FUICommandList;

/**
 * One command drawn as a palette tile: its icon above a label that wraps and centers inside a fixed tile width.
 *
 * A stand-in for the toolbar button a palette group would otherwise be built from, because that button cannot show a
 * label of more than about a word. SToolBarButtonBlock builds the label as a plain STextBlock with no wrapping, and
 * SMultiBoxWidget then hands the block a cell it caps at 50 by 43, so PaletteToolBar's Ellipsis overflow policy eats
 * the rest. Nothing in FToolBarStyle turns wrapping on for that path, and the content cannot be replaced from a
 * builder either: FButtonArgs carries no custom widget, and FToolBarButtonBlock::SetCustomWidget is only reachable
 * through UToolMenus.
 *
 * @note Only the drawing is ours. Execution, enablement, check state and visibility all resolve through the command
 *       list the tile is given, so a command behaves here exactly as it does in a toolbar or a menu — including
 *       collapsing itself when its FIsActionButtonVisible predicate says it has nothing to offer.
 * @remark Sized rather than uniform. Tiles do not negotiate a common size with each other, so a group of them wants a
 *         panel that lays out fixed-size children — an SWrapBox with UseAllottedSize is what the World Assembly edit
 *         mode's rail puts them in, and is the layout the defaults below are tuned for.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/widgets/command-tile/">SNCommandTile</a>
 */
class NEXUSUIEDITOR_API SNCommandTile final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNCommandTile)
		: _Style(&FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("PaletteToolBar"))
		, _TileWidth(64.0f)
		, _ReservedLabelLines(2)
		{}

		/** The command the tile draws, and acts on when clicked. */
		SLATE_ARGUMENT(TSharedPtr<const FUICommandInfo>, Command)

		/** The list the command's action, enablement, check state and visibility are resolved against. */
		SLATE_ARGUMENT(TSharedPtr<const FUICommandList>, CommandList)

		/** Toolbar style the tile borrows its button, label and paddings from, so it matches the toolbars around it. */
		SLATE_STYLE_ARGUMENT(FToolBarStyle, Style)

		/**
		 * Width the tile takes.
		 * @note A hard bound — it is what the label wraps against, and every tile in a group has to agree on it for the
		 *       group to read as a grid.
		 */
		SLATE_ARGUMENT(float, TileWidth)

		/**
		 * Lines of label height the tile keeps room for.
		 * @note Two by default, so one wrapped label in a group does not leave itself standing taller than the tiles
		 *       beside it. Room only: a label needing another line still gets it, and takes its tile's height with it.
		 *       Pass one for a group whose labels are known to be short, which gives back a line of empty tile.
		 */
		SLATE_ARGUMENT(int32, ReservedLabelLines)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/**
	 * Build the icon-over-label content, which is the whole of what this widget adds to a stock toolbar button.
	 *
	 * @param InCommand The command supplying the icon and the label.
	 * @param InStyle Toolbar style the icon and label paddings come from.
	 * @param InTileWidth Width of the tile, which is what the label wraps against.
	 * @param InReservedLabelLines Lines of label height to keep room for.
	 * @return The tile's content, ready to drop into whichever button shape wraps it.
	 */
	static TSharedRef<SWidget> CreateTileContent(const TSharedRef<const FUICommandInfo>& InCommand, const FToolBarStyle& InStyle,
		float InTileWidth, int32 InReservedLabelLines);
};
