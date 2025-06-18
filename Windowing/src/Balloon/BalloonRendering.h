#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <stdexcept>

#include "../../include/AGXWin.h"
#include "../../../DataProvider/include/AGXDpv.h"

enum class TipPos
{
	Top,
	Left,
	Bottom,
	Right
};

class BalloonRendering
{
	const uint32_t CornerDiameter = 3;
	const uint32_t TipDepth = 12;
	const uint32_t TipWidth = 10;
	const uint32_t HalfTipW = TipWidth / 2;

	int TipOffsetInLine = 0;

	/*|
	*  \_______________    _____ ...
	*                  \  / ] TipDepth
	*   +-------------+ \/  ]
	*   TipOffsetInLine
	*                  +--+
	*					TipWidth
	*/

	AgRect Bounds;
	TipPos TipPosition;

public:
	void Setup(BalloonInfo bi);
	void Paint(Gdiplus::Graphics* g, std::shared_ptr<FrameInfo> curFrame, MouthOverlayType overlay);
	~BalloonRendering();
};

