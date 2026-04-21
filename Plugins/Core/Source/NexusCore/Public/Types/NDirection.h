// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * The six axis-aligned directions in Unreal's left-handed coordinate system.
 */
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

/**
 * Lookup helpers that map ENDirection values to their canonical FVector.
 */
class FNDirection
{
public:
	/**
	 * Returns the engine's canonical unit vector for Direction.
	 * @param Direction The axis-aligned direction to look up.
	 * @return The matching unit vector, or the zero vector if Direction is unrecognized.
	 */
	static const FVector& GetVector(const ENDirection& Direction)
	{
		switch (Direction) 
		{
			using enum ENDirection;
		case Up:
			return FVector::UpVector;

		case Down:
			return FVector::DownVector;

		case Forward:
			return FVector::ForwardVector;
		
		case Backward:
			return FVector::BackwardVector;
		
		case Right:
			return FVector::RightVector;
		
		case Left:
			return FVector::LeftVector;
		}
		return FVector::ZeroVector;
	}
};
