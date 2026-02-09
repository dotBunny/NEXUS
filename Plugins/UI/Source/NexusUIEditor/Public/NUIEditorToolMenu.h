// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNUIEditorToolMenu
{
public:
	static void Register();
	static void Unregister();
	
	static void CreateCollisionQueryWindow();
	static bool HasCollisionQueryWindow();
};