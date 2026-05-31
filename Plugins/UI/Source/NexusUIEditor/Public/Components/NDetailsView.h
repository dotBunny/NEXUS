// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/DetailsView.h"
#include "NDetailsView.generated.h"

/**
 * A wrapper around the UDetailsView class to expose the internal OnPropertyValueChanged delegate.
 * The delegate is protected on the base class so subclassing is the only way to surface it to
 * native callers that need to react to property edits in an embedded details panel.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/components/details-view/">UNDetailsView</a>
 */
UCLASS()
class NEXUSUIEDITOR_API UNDetailsView final : public UDetailsView
{
	GENERATED_BODY()
public:
	/**
	 * Get a reference to the internal OnPropertyValueChanged on the UDetailsView.
	 * @return Delegate holder for the OnPropertyValueChanged event.
	 */
	FOnPropertyValueChanged* GetOnPropertyValueChangedRef()
	{
		return &OnPropertyChanged;
	}
};