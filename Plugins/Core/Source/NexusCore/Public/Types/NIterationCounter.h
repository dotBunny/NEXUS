// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNIterationCounter
{
	int Iteration = 0;
	TArray<int32> Counter = TArray({0});
	
	void Increment()
	{
		Counter[Iteration] = Counter[Iteration] + 1;
	}
	void Decrement()
	{
		Counter[Iteration] = Counter[Iteration] - 1;
	}
	void NextIteration()
	{
		Counter.Add(0);
		Iteration++;
	}
};
