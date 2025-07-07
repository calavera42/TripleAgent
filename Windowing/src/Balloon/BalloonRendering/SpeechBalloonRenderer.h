#pragma once

#include "IBalloonRenderer.h"

constexpr int SBTipDepth = 20;
constexpr int SBTipSpacing = 14;
constexpr int SBTipMiddle = SBTipSpacing / 2;
constexpr int SBCornerDiameter = 23;

constexpr int SBCornerSpacing = 12;

class SpeechBalloonRenderer : public IBalloonRenderer
{
private:
	// CornerSpacing
	/*|+-+Blah blah
	 *|
	 * \_______________    _____ ...
	 *                 \  / ] PointDepth
	 *  +-------------+ \/  ]
	 *  PointOffsetInLine
	 *                 +--+ } PointSpacing
	 */

	BalloonInfo Style{};
	CharacterFlags CharFlags{};

	ULONG_PTR GdiToken{};

	Gdiplus::SizeF MaxSize{};

	Gdiplus::FontFamily* AgFntFamily{};
	Gdiplus::Font* AgFont{};

	Gdiplus::SolidBrush* TextBrush{};
	Gdiplus::SolidBrush* BackBrush{};
	Gdiplus::Pen* BorderPen{};

private:
	void DrawShape(Gdiplus::Graphics* g, Gdiplus::Rect bodyBounds, int tipOffsetInLine, TipQuadrant tq);

	void Place(TipQuadrant tq, Gdiplus::Rect agRect, Gdiplus::Rect wkRect);

	Gdiplus::SizeF GetTextSize(string text, Gdiplus::SizeF maxSize) const;
	float GetAverageCharWidth(const Gdiplus::Font* f) const;


public:
	void Setup(CharacterInfo& ci) override;
	void Paint(HWND hwnd, BalloonRenderInfo bri) override;

	Gdiplus::Rect GetSize(TipQuadrant tq, string Text) override;

	~SpeechBalloonRenderer();
};

