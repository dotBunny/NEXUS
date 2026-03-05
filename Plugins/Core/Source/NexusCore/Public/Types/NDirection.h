// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum class ENDirection : uint8
{
	Up = 0,
	Down = 1,
	Forward = 2,
	Backward = 3,
	Right = 4,
	Left = 5
};

class FNDirection
{
public:
	static const FVector& GetVector(const ENDirection& Direction)
	{
		switch (Direction) 
		{
		case ENDirection::Up:
			return FVector::UpVector;

		case ENDirection::Down:
			return FVector::DownVector;

		case ENDirection::Forward:
			return FVector::ForwardVector;
		
		case ENDirection::Backward:
			return FVector::BackwardVector;
		
		case ENDirection::Right:
			return FVector::RightVector;
		
		case ENDirection::Left:
			return FVector::LeftVector;
		}
		return FVector::ZeroVector;
	}
};
