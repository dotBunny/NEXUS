// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

/**
 * Detail-panel customization for UNOrganComponent. Surfaces Generate/Cancel/Clear action buttons
 * that drive the editor-side ProcGen operation, with visibility gated on current operation state.
 */
class FNOrganComponentCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization

private:
	/** Kick off a new editor-side ProcGen operation for the selected organ component(s). */
	FReply OnGenerateClicked(const TArray<TWeakObjectPtr<UObject>> Objects);

	/** Cancel the currently-running operation associated with the selected organ component(s). */
	FReply OnCancelClicked(TArray<TWeakObjectPtr<UObject>> Object);

	/** Remove previously-spawned generated proxies for the selected organ component(s). */
	FReply OnClearClicked(TArray<TWeakObjectPtr<UObject>> Object);

	/** @return Visible when no operation is active and generation is possible. */
	EVisibility GenerateButtonVisible() const;

	/** @return Visible only while an operation is running for the selected component(s). */
	EVisibility CancelButtonVisible() const;

	/** @return Visible when previously-generated proxies exist that could be cleared. */
	EVisibility ClearButtonVisible() const;
};