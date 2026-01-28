// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NColor.h"

#include "NStyleLibrary.generated.h"

UCLASS(ClassGroup = "NEXUS", DisplayName = "Style Library")
class UNStyleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, DisplayName="Get Warning Color", Category = "NEXUS|User Interface|Style")
	static ENColor GetWarningColor()
	{
		return ENColor::NC_Orange;
	}
	
	UFUNCTION(BlueprintPure, DisplayName="Get Warning Color (Foreground)", Category = "NEXUS|User Interface|Style")
	static ENColor GetWarningForegroundColor()
	{
		return ENColor::NC_White;
	}
	UFUNCTION(BlueprintPure, DisplayName="Get Warning Color (Background)", Category = "NEXUS|User Interface|Style")
	static ENColor GetWarningBackgroundColor()
	{
		return ENColor::NC_Orange;
	}
	
	
	
	UFUNCTION(BlueprintPure, DisplayName="Get Error Color", Category = "NEXUS|User Interface|Style")
	static ENColor GetErrorColor()
	{
		return ENColor::NC_Red;
	}
	UFUNCTION(BlueprintPure, DisplayName="Get Error Color (Foreground)", Category = "NEXUS|User Interface|Style")
	static ENColor GetErrorForegroundColor()
	{
		return ENColor::NC_White;
	}
	UFUNCTION(BlueprintPure, DisplayName="Get Error Color (Background)", Category = "NEXUS|User Interface|Style")
	static ENColor GetErrorBackgroundColor()
	{
		return ENColor::NC_Red;
	}
	
	UFUNCTION(BlueprintPure, DisplayName="Get Info Color", Category = "NEXUS|User Interface|Style")
	static ENColor GetInfoColor()
	{
		return ENColor::NC_BlueDark;
	}
	
	UFUNCTION(BlueprintPure, DisplayName="Get Info Color (Foreground)", Category = "NEXUS|User Interface|Style")
	static ENColor GetInfoForegroundColor()
	{
		return ENColor::NC_White;
	}
	
	UFUNCTION(BlueprintPure, DisplayName="Get Info Color (Background)", Category = "NEXUS|User Interface|Style")
	static ENColor GetInfoBackgroundColor()
	{
		return ENColor::NC_BlueDark;
	}
};