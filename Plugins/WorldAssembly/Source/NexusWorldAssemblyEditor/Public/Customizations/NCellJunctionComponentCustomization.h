// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Types/SlateEnums.h"

class AActor;
class SWidget;
class UNCellJunctionComponent;
struct FPropertyChangedEvent;
template<typename OptionType> class SComboBox;

/**
 * Detail-panel customization for UNCellJunctionComponent. Surfaces a "Filler Visualizer" panel: a dropdown of the
 * junction's authored Fillers plus Fill/Clear buttons. Fill spawns the selected entry's Actor as a transient
 * preview at the junction's transform combined with the entry's Offset; changing the dropdown while a preview is
 * live replaces it, and the preview is removed on Clear or when the junction leaves the editor selection.
 *
 * @note Editor-only and single-junction only: the controls are suppressed when 0 or more than 1 junction is selected.
 */
class FNCellJunctionComponentCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual ~FNCellJunctionComponentCustomization() override;

	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization

private:
	/** @return Friendly label for the filler entry at FillerIndex ("(None)" when its Actor is unset), e.g. "BP My Filler". */
	FText GetFillerLabel(int32 FillerIndex) const;

	/** @return Friendly label for the currently selected filler entry, or empty when there is no selection. */
	FText GetSelectedFillerLabel() const;

	/** Builds the dropdown row widget for a single filler option. */
	TSharedRef<SWidget> OnGenerateFillerWidget(TSharedPtr<int32> InOption) const;

	/** Stores the new dropdown selection and, when a preview is already live, replaces it with the new entry. */
	void OnFillerSelectionChanged(TSharedPtr<int32> NewSelection, ESelectInfo::Type SelectInfo);

	/** Spawns (replacing any existing) the transient preview actor for the selected filler. */
	FReply OnFillClicked();

	/** Removes the transient preview actor, if any. */
	FReply OnClearClicked();

	/** @return true when a junction with a non-null selected filler is available to spawn. */
	bool IsFillEnabled() const;

	/** @return true when a live preview actor exists to clear. */
	bool IsClearEnabled() const;

	/** Spawns the transient preview actor for the filler at FillerIndex, placed at the junction transform + entry Offset. */
	void SpawnPreview(int32 FillerIndex);

	/** Destroys the transient preview actor, if one exists. */
	void DestroyPreview();

	/**
	 * Computes the world placement for a preview at the junction with the given authored Offset, mirroring
	 * UNCellJunctionComponent::Fill(): the location offset is authored in the junction's frame (rotated by the
	 * junction's orientation before adding) and the rotation offset spins the preview in place at that spot.
	 */
	void ComputePreviewPlacement(const FTransform& Offset, FVector& OutLocation, FQuat& OutRotation) const;

	/** Re-places (or, when the entry's Actor changed, respawns) the live preview after the Fillers data is edited. */
	void OnFillersPropertyChanged();

	/**
	 * Respawns the live preview after a committed edit to the junction's fill depth (FillDepthMode / OverrideFillDepth)
	 * so it re-reads the depth through the filler's OnInitializedFromJunction. Interactive changes (slider drags) are
	 * ignored; the trailing commit refreshes once. Applies to the authored fillers and the "(Default)" option alike.
	 * @param PropertyChangedEvent The change notification, inspected for its EPropertyChangeType.
	 */
	void OnFillDepthPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * Rebuilds FillerOptions from the junction's current Fillers (plus the trailing default option), preserves the
	 * current selection by index, and refreshes the live dropdown so it reflects added/removed entries when reopened.
	 */
	void RebuildFillerOptions();

	/** Selection-changed backstop: removes the preview once the junction's owner leaves the editor selection. */
	void OnEditorSelectionChanged(UObject* NewSelection);

	/** The single junction being customized; unset when 0 or more than 1 junction is selected. */
	TWeakObjectPtr<UNCellJunctionComponent> Junction;

	/** Combo source — one shared int per index into Junction->Fillers. */
	TArray<TSharedPtr<int32>> FillerOptions;

	/** Current dropdown selection (an entry of FillerOptions), or null when there are no fillers. */
	TSharedPtr<int32> SelectedOption;

	/** The filler dropdown widget, retained so its options can be refreshed when the junction's Fillers list changes. */
	TSharedPtr<SComboBox<TSharedPtr<int32>>> FillerComboBox;

	/** Live transient preview actor, or invalid when none is spawned. */
	TWeakObjectPtr<AActor> PreviewActor;

	/** The preview actor's intrinsic scale captured at spawn (before the entry Offset scale), so offset edits re-scale from a stable base. */
	FVector PreviewBaseScale = FVector::OneVector;

	/** Handle for the USelection::SelectionChangedEvent backstop subscription. */
	FDelegateHandle SelectionChangedHandle;
};
