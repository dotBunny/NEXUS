﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNFixersEditorBulkOperations
{
public:

	static void Register();
	static void Unregister();

	static void FillMenu(UToolMenu* Menu, bool bIsContextMenu);
};
