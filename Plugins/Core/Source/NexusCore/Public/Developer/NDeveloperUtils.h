// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/Object.h"

class FNDeveloperUtils
{
public:
	FORCEINLINE static int32 GetCurrentObjectCount() { return GUObjectArray.GetObjectArrayNumMinusAvailable(); }
	
	static TArray<UObject*> GetRootSetObjects()
	{
		TArray<UObject*> RootedObjects;
		for (int32 i = 0; i < GUObjectArray.GetObjectArrayNum(); ++i)
		{
			FUObjectItem* ObjectItem = GUObjectArray.IndexToObject(i);
			if (ObjectItem->IsRootSet())
			{
#if UE_VERSION_OLDER_THAN(5, 6, 0) // .Object gets deprecated in 5.6
				UObject* Object = static_cast<UObject*>(ObjectItem.Object);
#else // !UE_VERSION_OLDER_THAN(5, 6, 0)
				UObject* Object = static_cast<UObject*>(ObjectItem->GetObject());
#endif // UE_VERSION_OLDER_THAN(5, 6, 0)
				if (Object != nullptr)
				{
					RootedObjects.Add(Object);
				}
			}
		}
		return MoveTemp(RootedObjects);
	}
	static void DumpRootSetToLog()
	{
		TArray<UObject*> RootedObjects = GetRootSetObjects();
		UE_LOG(LogNexusCore, Log, TEXT("[ROOTSET] %i"), RootedObjects.Num());
		for (UObject* Object : RootedObjects)
		{
			UE_LOG(LogNexusCore, Log, TEXT("%s"), *Object->GetName());
		}
	}
	
	
	static FString GetBuildWatermark()
	{
		static const FString BuildWatermark = FString::Printf(TEXT("%s-%s%s (%s)"),
			FApp::GetBuildVersion(),
			LexToString(FApp::GetBuildConfiguration()),
			LexToString(FApp::GetBuildTargetType()),
			*FApp::GetBuildDate()).Replace(TEXT("+"), TEXT("/"));
		return BuildWatermark;
	}

	static bool HasBuildInfo()
	{
		return !FString(FApp::GetBuildVersion()).Equals(FString(TEXT("UE5-CL-0")));
	}
};
