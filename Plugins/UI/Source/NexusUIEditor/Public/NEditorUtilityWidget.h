// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "EditorUtilityWidget.h"
#include "NEditorUtilityWidget.generated.h"

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization
 * and appearance: a custom tab icon, deferred post-construct work via UAsyncEditorDelay, and
 * integration with UNEditorUtilityWidgetSubsystem so widget state survives editor sessions when
 * bIsPersistent / bHasPermanentState are set.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS(BlueprintType)
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:

	/**
	 * Spawn the editor utility widget asset found at ObjectPath inside its own dock tab.
	 * @param ObjectPath Object path to the UEditorUtilityWidgetBlueprint asset to spawn.
	 * @param Identifier Optional tab identifier override; defaults to NAME_None meaning the subsystem chooses one.
	 * @return The instantiated UEditorUtilityWidget hosted by the spawned tab, or nullptr on failure.
	 */
	static UEditorUtilityWidget* SpawnTab(const FString& ObjectPath, FName Identifier = NAME_None);

	/** @return True when the widget opts in to cross-session state persistence via the widget subsystem. */
	UFUNCTION(BlueprintCallable)
	bool IsPersistent() const
	{
		return bIsPersistent;
	};

	/** @return The widget's stable identifier used as the key when storing/restoring persistent state. */
	UFUNCTION(BlueprintCallable)
	FName GetUniqueIdentifier() const
	{
		return UniqueIdentifier;
	};

	/** @return The tab identifier the widget was most recently hosted under, or NAME_None if not tabbed. */
	UFUNCTION(BlueprintCallable)
	FName GetTabIdentifier() const
	{
		return CachedTabIdentifier;
	};

protected:

	//~UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//End UUserWidget

	/** Runs one frame after NativeConstruct so the widget is fully laid out before state is re-applied. */
	UFUNCTION()
	virtual void DelayedConstructTask();

	/** Toggle persistent state tracking; when true the subsystem stores this widget's FNWidgetState across sessions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State")
	bool bIsPersistent = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State",
		meta=(Tooltip="Should the widget attempt to remove itself when the tab is closed from any cached state in the UNEditorUtilityWidgetSubsystem."))
	bool bHasPermanentState = false;

	/** Stable key used by the subsystem to match this widget back to its stored state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State")
	FName UniqueIdentifier;

	/** Slate style set that owns the tab icon brush (e.g. FNUIEditorStyle::GetStyleSetName()). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	FName TabIconStyle;

	/** Brush name inside TabIconStyle to use as the dock tab icon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	FName TabIconName;

	/** When true the tab is removed from the workspace menu on close so it can't be re-spawned accidentally. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	bool bRemoveWorkspaceItem = true;

	/** Accessing this has to happen on the following frame after constructing the widget. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Info")
	FVector2D UnitScale = FVector2D::One();

	/** Holds the in-flight UAsyncEditorDelay used to schedule DelayedConstructTask. */
	UPROPERTY()
	TObjectPtr<UAsyncEditorDelay> DelayedTask;

private:
	/** Bound to the host SDockTab's on-closed callback; forwards to the subsystem to clean up state if bHasPermanentState is false. */
	void OnTabClosed(TSharedRef<SDockTab> Tab) const;

	/** Handle retained so OnTabClosed can be unregistered from the dock tab on destruction. */
	SDockTab::FOnTabClosedCallback OnTabClosedCallback;

	/** Composed icon built from TabIconStyle + TabIconName on construct; applied to the hosting tab. */
	FSlateIcon TabIcon;

	/** Resolved tab identifier captured at construct time and returned by GetTabIdentifier. */
	FName CachedTabIdentifier;

};