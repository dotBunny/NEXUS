// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPrimitiveDrawingUtils.h"
#include "NPrimitiveFont.h"

// #SONARQUBE-DISABLE-CPP_S107 Verbosity necessary, most are defaulted regardless.
void FNPrimitiveDrawingUtils::DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position,
                                         const FRotator& Rotation, const FLinearColor ForegroundColor, const float Scale,
                                         const float LineHeight, const float Thickness,
                                         const bool bInvertLineFeed, const bool bDrawBelowPosition, 
                                         const ESceneDepthPriorityGroup DepthPriorityGroup)
{
	// Ensure our glyphs are created
	if (!FNPrimitiveFont::IsInitialized())
	{
		FNPrimitiveFont::Initialize();
	}

	// Calculate the working scale to multiply our glyph points by
	const float WorkingScale = Scale * 12; // Our pseudo font-size

	// Reserve some room, assuming an average of 5 lines per character
	TArray<TCHAR>& Characters = String.GetCharArray();

	PDI->AddReserveLines(SDPG_World, 6 * Characters.Num(), false, true);
	
	float WorkingLineHeight = ((8 + LineHeight) * WorkingScale) * -1;
	FVector BasePosition = Position;
	if (bDrawBelowPosition)
	{
		BasePosition += FVector(0.f, 0.f, WorkingLineHeight);
	}
	FVector CurrentPosition = BasePosition;
	int LineIndex = 0;
	
	// Make it stack up if inverted
	if (bInvertLineFeed)
	{
		WorkingLineHeight *= -1;
	}
	
	const FVector CharacterOffset = FVector(0.f, 8 * WorkingScale, 0.f);
	const UE::Math::TRotationMatrix<double> RotationMatrix = UE::Math::TRotationMatrix(Rotation);
	const bool bNeedsRotation = Rotation != FRotator::ZeroRotator;
	
	for (const auto Character : Characters)
	{
		switch (const int GlyphIndex = Character)
		{
		case 0: // String terminator, stop now
			return;
		case 9: // Tab (4-spaces)
			CurrentPosition = CurrentPosition + (CharacterOffset * 4);
			break;
		case 10: // New Line
		case 13:
			LineIndex++;
			CurrentPosition = BasePosition + FVector(0.f, 0.f, WorkingLineHeight * LineIndex);
			break;
		case 32: // Space
			CurrentPosition += CharacterOffset;
			break;
		default:

			// Get reference to glyph data
			TArray<FNPrimitiveFontPoint>& Points = FNPrimitiveFont::GetGlyph(GlyphIndex);
			
			// Check our point count, if we don't have any at this point, it is considered a bad character
			int PointCount = Points.Num();
			if (PointCount  == 0)
			{
				Points = FNPrimitiveFont::GetGlyph(0);
				PointCount = FNPrimitiveFont::UndefinedPointCount;
			}

			// Draw our glyph
			for (int i = 0; i < PointCount; i += 2)
			{
				// Scale our points and bring them into 3D
				FVector StartPoint = CurrentPosition + FVector(0.f, Points[i].X * WorkingScale, Points[i].Y * WorkingScale);
				FVector EndPoint = CurrentPosition + FVector(0.f, Points[i + 1].X * WorkingScale, Points[i + 1].Y * WorkingScale);
				
				// Rotate points around base origin w/ rotation
				if (bNeedsRotation)
				{
					StartPoint = BasePosition + RotationMatrix.TransformVector(StartPoint - BasePosition);
					EndPoint = BasePosition + RotationMatrix.TransformVector(EndPoint - BasePosition);
				}
				
				PDI->DrawLine(StartPoint, EndPoint, ForegroundColor, DepthPriorityGroup, Thickness);
			}

			// Offset the position to get ready for the next character
			CurrentPosition += CharacterOffset;
			break;
		}
	}
}
// #SONARQUBE-ENABLE
