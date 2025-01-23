#pragma once

#include "ACSFormat/Structs.h"

#include <SDL.h>

enum class TipQuadrant {
	Top,
	Right,
	Bottom,
	Left
};

class BalloonWnd
{
public:
	SDL_Window* Window;
	SDL_Renderer* Renderer;

	BalloonInfo BalloonStyle;

	SDL_Point TipPosition;

	const int CornerDiamenter = 10;
	const int TipDepth = 13;
	const int TipSpacing = 10;
	const int TipMiddle = TipSpacing / 2;

	void Setup();
	void Render(Rect bounds);

	void DrawCorner(SDL_Point pos, int diameter, bool flipX = false, bool flipY = false);
	int Distance(int x, int y, int x1, int y1);
};

