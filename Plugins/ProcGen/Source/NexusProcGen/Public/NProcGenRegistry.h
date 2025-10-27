// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class UNOrganComponent;
class UNCellJunctionComponent;
class UNCellRootComponent;

class NEXUSPROCGEN_API FNProcGenRegistry
{
public:

	FORCEINLINE static TArray<UNCellRootComponent*>& GetCellRootComponents() { return CellRoots; }
	FORCEINLINE static TArray<UNCellJunctionComponent*>& GetCellJunctionComponents() { return CellJunctions; }
	FORCEINLINE static TArray<UNOrganComponent*>& GetOrganComponents() { return Organs; }

	static TArray<UNCellJunctionComponent*> GetCellJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted = true);
	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* Level);
	static UNCellRootComponent* GetCellRootComponentFromLevel(const ULevel* Level);
	
	static bool HasRootComponents();
	static bool HasJunctionComponents();
	static bool HasOrganComponents();
	static bool HasOrganComponentsInWorld(const UWorld* World);
	
	static bool RegisterCellRootComponent(UNCellRootComponent* Component);
	static bool RegisterCellJunctionComponent(UNCellJunctionComponent* Component);
	static bool RegisterOrganComponent(UNOrganComponent* Organ);
	
	static bool UnregisterCellRootComponent(UNCellRootComponent* Component);
	static bool UnregisterCellJunctionComponent(UNCellJunctionComponent* Component);
	static bool UnregisterOrganComponent(UNOrganComponent* Organ);

private:
	static TArray<UNCellRootComponent*> CellRoots;
	static TArray<UNCellJunctionComponent*> CellJunctions;
	static TArray<UNOrganComponent*> Organs;
};