#pragma once

#include <agxdpv.h>

#include <windows.h>
#include <gdiplus.h>


// A ORDEM IMPORTA!!!
enum class TipQuadrant : int {
	Top = 0,
	Right = 1,
	Bottom = 2,
	Left = 3,
};

struct BalloonRenderInfo {
	TipQuadrant TipQuad;
	int TipOffsetInLine;
	string Text;
	bool AutoPace;
	int SpeechProgress;
};

class IBalloonRenderer
{
public:
	virtual void Setup(CharacterInfo& ci) = 0;
	virtual void Paint(HWND hwnd, BalloonRenderInfo bri) = 0;

	virtual Gdiplus::Rect GetSize(TipQuadrant tq, string Text) = 0;
};