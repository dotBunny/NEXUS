// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class UNCellJunctionComponent;
class UNCellRootComponent;

class NEXUSPROCGEN_API FNCellRegistry
{
public:

	FORCEINLINE static TArray<UNCellRootComponent*>& GetRootComponents() { return Roots; }
	FORCEINLINE static TArray<UNCellJunctionComponent*>& GetJunctionComponents() { return Junctions; }

	static TArray<UNCellJunctionComponent*> GetJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted = true);
	static UNCellRootComponent* GetRootComponentFromLevel(const ULevel* Level);
	
	static bool HasRootComponents();
	static bool HasJunctionComponents();
	
	static bool RegisterRootComponent(UNCellRootComponent* Component);
	static bool RegisterJunctionComponent(UNCellJunctionComponent* Component);
	
	static bool UnregisterRootComponent(UNCellRootComponent* Component);
	static bool UnregisterJunctionComponent(UNCellJunctionComponent* Component);

private:
	static TArray<UNCellRootComponent*> Roots;
	static TArray<UNCellJunctionComponent*> Junctions;
};