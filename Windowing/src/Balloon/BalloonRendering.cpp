#include "BalloonRendering.h"

using namespace Gdiplus;

void BalloonRendering::Setup(BalloonInfo bi) 
{

}

void BalloonRendering::Paint(Graphics* g, std::shared_ptr<FrameInfo> curFrame, MouthOverlayType overlay) 
{
	GraphicsPath gp{};

	Rect arcs[] = {
		{ Bounds.Left, Bounds.Top, CornerDiameter, CornerDiameter },
		{ Bounds.Right - CornerDiameter, Bounds.Top, CornerDiameter, CornerDiameter },
		{ Bounds.Right - CornerDiameter, Bounds.Bottom - CornerDiameter, CornerDiameter, CornerDiameter },
		{ Bounds.Left, Bounds.Bottom - CornerDiameter, CornerDiameter, CornerDiameter },
	};
	Point tipPoints[3];

	switch (TipPosition) 
	{
		case TipPos::Top:
		{
			tipPoints[0] = { Bounds.Left + CornerDiameter + TipOffsetInLine, Bounds.Top + TipDepth };
			tipPoints[1] = { Bounds.Left + CornerDiameter + TipOffsetInLine + HalfTipW, Bounds.Top };
			tipPoints[2] = { Bounds.Left + CornerDiameter + TipOffsetInLine + TipWidth, Bounds.Top + TipDepth };
			break;
		}
		case TipPos::Bottom:
		{
			tipPoints[0] = {Bounds.Left + CornerDiameter + TipOffsetInLine + TipWidth, Bounds.Top,}
			break;
		}
	}
}

BalloonRendering::~BalloonRendering()
{

}
