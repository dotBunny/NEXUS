// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NColor.h"

#include "NStyleLibrary.generated.h"

/**
 * Blueprint-visible style constants for NEXUS-branded warning/error/info UI treatments.
 * Returns ENColor values rather than FLinearColor so downstream widgets can look up the exact
 * palette entry via FNColor at the last moment.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Style Library")
class UNStyleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** @return The canonical warning accent color used by NEXUS UI treatments. */
	UFUNCTION(BlueprintPure, DisplayName="Get Warning Color", Category = "NEXUS|User Interface|Style")
	static ENColor GetWarningColor()
	{
		return ENColor::NC_Orange;
	}

	/** @return The foreground (text/icon) color paired with the warning accent. */
	UFUNCTION(BlueprintPure, DisplayName="Get Warning Color (Foreground)", Category = "NEXUS|User Interface|Style")
	static ENColor GetWarningForegroundColor()
	{
		return ENColor::NC_White;
	}
	/** @return The background/container color paired with the warning accent. */
	UFUNCTION(BlueprintPure, DisplayName="Get Warning Color (Background)", Category = "NEXUS|User Interface|Style")
	static ENColor GetWarningBackgroundColor()
	{
		return ENColor::NC_Orange;
	}



	/** @return The canonical error accent color used by NEXUS UI treatments. */
	UFUNCTION(BlueprintPure, DisplayName="Get Error Color", Category = "NEXUS|User Interface|Style")
	static ENColor GetErrorColor()
	{
		return ENColor::NC_Red;
	}
	/** @return The foreground (text/icon) color paired with the error accent. */
	UFUNCTION(BlueprintPure, DisplayName="Get Error Color (Foreground)", Category = "NEXUS|User Interface|Style")
	static ENColor GetErrorForegroundColor()
	{
		return ENColor::NC_White;
	}
	/** @return The background/container color paired with the error accent. */
	UFUNCTION(BlueprintPure, DisplayName="Get Error Color (Background)", Category = "NEXUS|User Interface|Style")
	static ENColor GetErrorBackgroundColor()
	{
		return ENColor::NC_Red;
	}

	/** @return The canonical informational accent color used by NEXUS UI treatments. */
	UFUNCTION(BlueprintPure, DisplayName="Get Info Color", Category = "NEXUS|User Interface|Style")
	static ENColor GetInfoColor()
	{
		return ENColor::NC_BlueDark;
	}

	/** @return The foreground (text/icon) color paired with the informational accent. */
	UFUNCTION(BlueprintPure, DisplayName="Get Info Color (Foreground)", Category = "NEXUS|User Interface|Style")
	static ENColor GetInfoForegroundColor()
	{
		return ENColor::NC_White;
	}

	/** @return The background/container color paired with the informational accent. */
	UFUNCTION(BlueprintPure, DisplayName="Get Info Color (Background)", Category = "NEXUS|User Interface|Style")
	static ENColor GetInfoBackgroundColor()
	{
		return ENColor::NC_BlueDark;
	}
};