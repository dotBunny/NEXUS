// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NBoxPicker.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_BOX_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector( \
		FloatValue(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X), \
		FloatValue(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y), \
		FloatValue(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z))
#define N_PICKER_BOX_LOCATION(FloatValue, BooleanValue) \
	Params.Origin + FVector( \
		BooleanValue ? FloatValue(-MinimumExtent.X, -MaximumExtent.X) : FloatValue(MinimumExtent.X, MaximumExtent.X), \
		BooleanValue ? FloatValue(-MinimumExtent.Y, -MaximumExtent.Y) : FloatValue(MinimumExtent.Y, MaximumExtent.Y), \
		BooleanValue ? FloatValue(-MinimumExtent.Z, -MaximumExtent.Z) : FloatValue(MinimumExtent.Z, MaximumExtent.Z))

#if ENABLE_VISUAL_LOG
#define N_PICKER_BOX_VLOG(HasMinimumDimensions) \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MinimumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		if(Params.MaximumDimensions.IsValid != 0) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MaximumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else
#define N_PICKER_BOX_VLOG(HasMinimumDimensions)
#endif

void FNBoxPicker::Next(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
{
	
	const int OutLocationsStartIndex = OutLocations.Num();
	const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
	
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(FNRandom::Deterministic.FloatRange);
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection	
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION_SIMPLE(FNRandom::Deterministic.FloatRange));
			}
		}
	}
	else
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(FNRandom::Deterministic.FloatRange, FNRandom::Deterministic.Bool());
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection	
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION(FNRandom::Deterministic.FloatRange, FNRandom::Deterministic.Bool()));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(bSimpleMode)
	
}

void FNBoxPicker::Random(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
{
	const int OutLocationsStartIndex = OutLocations.Num();
	const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
		
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(FNRandom::NonDeterministic.FRandRange);
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection	
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION_SIMPLE(FNRandom::NonDeterministic.FRandRange));
			}
		}
	}
	else
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
			
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(FNRandom::NonDeterministic.FRandRange, FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f);
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection	
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION(FNRandom::NonDeterministic.FRandRange, FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f));
			}
		}
	}
		
	N_PICKER_BOX_VLOG(bSimpleMode)		
}

void FNBoxPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNBoxPickerParams& Params)
{
	const int OutLocationsStartIndex = OutLocations.Num();
	const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
	const FRandomStream RandomStream(Seed);
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
		
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RandomStream.FRandRange);
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection	
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION_SIMPLE(RandomStream.FRandRange));
			}
		}
	}
	else
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
			
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RandomStream.FRandRange, RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f);
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection	
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION(RandomStream.FRandRange, RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f));
			}
		}
	}
		
	N_PICKER_BOX_VLOG(bSimpleMode)
	Seed = RandomStream.GetCurrentSeed();
}