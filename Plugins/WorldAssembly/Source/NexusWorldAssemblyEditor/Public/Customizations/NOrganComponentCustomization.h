// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class IPropertyUtilities;
class UNAssemblyOperation;
enum class ENWorldAssemblyOperationState : uint8;

/**
 * Detail-panel customization for UNOrganComponent. Surfaces Generate/Cancel/Clear action buttons
 * that drive the editor-side World Assembly operation, with visibility gated on current operation state.
 */
class FNOrganComponentCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual ~FNOrganComponentCustomization() override;


	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization

private:
	/** Kick off a new editor-side World Assembly operation for the selected organ component(s). */
	FReply OnGenerateClicked(const TArray<TWeakObjectPtr<UObject>> Objects);

	/** Cancel the currently-running operation(s) associated with the selected organ component(s). */
	FReply OnCancelClicked();

	/** Remove previously-spawned generated proxies for the selected organ component(s). */
	FReply OnClearClicked(TArray<TWeakObjectPtr<UObject>> Object);

	/** @return Visible only while an operation is running for the selected component(s). */
	EVisibility CancelButtonVisible() const;

	/** @return Visible when the selected organ component(s)' last operation still has proxies that could be cleared. */
	EVisibility ClearButtonVisible() const;
	
	EVisibility GenerateButtonVisible() const;

	/**
	 * @return true when the panel's organ component(s) can be generated: not in PIE, no operation
	 * currently running, and at least one valid organ component is being customized.
	 *
	 * Gates on the customized objects rather than the live editor selection: regenerating silently
	 * clears the actor selection (ClearGeneratedProxies -> GEditor->SelectNone), which would otherwise
	 * leave the button stuck disabled even though this panel is still bound to a valid component.
	 */
	bool CanGenerate() const;

	/** @return Running operations whose generation context contains any of the customized organ component(s). */
	TArray<UNAssemblyOperation*> GetActiveOperations() const;

	/** Re-evaluates the cached enabled-state of the action buttons when an operation registers/unregisters. */
	void HandleOperationStateChanged(UNAssemblyOperation* Operation, ENWorldAssemblyOperationState NewState);

	/** The organ component object(s) this panel instance is bound to; drives the action buttons. */
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;

	/** Property utilities for the panel we customized, used to force a refresh. */
	TWeakPtr<IPropertyUtilities> PropertyUtilities;

	/** Handle for our subscription to FNWorldAssemblyRegistry::OnOperationStateChanged. */
	FDelegateHandle OperationStateChangedHandle;
};