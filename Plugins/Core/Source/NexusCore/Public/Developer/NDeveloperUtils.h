// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/Object.h"

/**
 * Native helpers exposed for developer tooling and diagnostics.
 *
 * The members of this class are intentionally cheap and read-only so that they can be safely
 * invoked from tests, overlays, and developer UIs without perturbing the state they are inspecting.
 */
class FNDeveloperUtils
{
public:
	/**
	 * Indicates whether the current build was compiled with the IS_DEMO_BUILD=1 preprocessor define.
	 * @return true when the build is flagged as a demo build, false otherwise.
	 * @remark Requires a BuildTarget definition of IS_DEMO_BUILD=1 to return true.
	 */
	FORCEINLINE static bool IsDemoBuild()
	{
#ifdef IS_DEMO_BUILD
#if IS_DEMO_BUILD == 1
		return true;
#else // IS_DEMO_BUILD != 1
		return false;
#endif // IS_DEMO_BUILD == 1
#else // !IS_DEMO_BUILD
		return false;
#endif // IS_DEMO_BUILD
	}

	/**
	 * Current number of live UObjects, computed from GUObjectArray's allocated count minus free slots.
	 * @return The approximate number of live UObjects in the global object array.
	 */
	FORCEINLINE static int32 GetCurrentObjectCount() { return GUObjectArray.GetObjectArrayNumMinusAvailable(); }

	/**
	 * Scans the global UObject array and collects every object currently on the engine root set.
	 * @return An array of every rooted UObject at the time of the call.
	 * @note Potentially expensive on large projects; use sparingly and from tools/diagnostics only.
	 */
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

	/**
	 * Writes every rooted UObject's name to LogNexusCore, prefixed with the total count.
	 * @note Intended for on-demand leak diagnosis; not suitable for continuous logging.
	 */
	static void DumpRootSetToLog()
	{
		TArray<UObject*> RootedObjects = GetRootSetObjects();
		UE_LOG(LogNexusCore, Log, TEXT("[ROOTSET] %i"), RootedObjects.Num());
		for (UObject* Object : RootedObjects)
		{
			UE_LOG(LogNexusCore, Log, TEXT("%s"), *Object->GetName());
		}
	}

	/**
	 * Builds a human-readable watermark string that identifies the current build.
	 * @return A string in the form "<Version>-<Config><Target> (<Date>)" with '+' replaced by '/'.
	 */
	static FString GetBuildWatermark()
	{
		static const FString BuildWatermark = FString::Printf(TEXT("%s-%s%s (%s)"),
			FApp::GetBuildVersion(),
			LexToString(FApp::GetBuildConfiguration()),
			LexToString(FApp::GetBuildTargetType()),
			*FApp::GetBuildDate()).Replace(TEXT("+"), TEXT("/"));
		return BuildWatermark;
	}

	/**
	 * Indicates whether the engine reports real build information (a non-default changelist).
	 * @return false when running an unsubmitted local build ("UE5-CL-0"), true otherwise.
	 */
	static bool HasBuildInfo()
	{
		return !FString(FApp::GetBuildVersion()).Equals(FString(TEXT("UE5-CL-0")));
	}
};
