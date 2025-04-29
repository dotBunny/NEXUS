// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/DetailsView.h"
#include "NDetailsView.generated.h"

/**
 * A wrapper around the <code>UDetailsView</code> class to expose the internal <code>OnPropertyValueChanged</code> delegate.
 */
UCLASS()
class NEXUSUIEDITOR_API UNDetailsView final : public UDetailsView
{
	GENERATED_BODY()
public:
	/**
	 * Get a reference to the internal <code>OnPropertyValueChanged</code> on the <code>UDetailsView</code>.
	 * @return Delegate holder for the <code>OnPropertyValueChanged</code> event.
	 */
	FOnPropertyValueChanged* GetOnPropertyValueChangedRef()
	{
		return &OnPropertyChanged;
	}
};