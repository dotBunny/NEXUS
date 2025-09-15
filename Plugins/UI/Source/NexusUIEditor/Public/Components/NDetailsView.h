// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/DetailsView.h"
#include "NDetailsView.generated.h"

/**
 * A wrapper around the UDetailsView class to expose the internal OnPropertyValueChanged delegate.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/details-view/">UNDetailsView</a> 
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