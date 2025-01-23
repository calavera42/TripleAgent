#pragma once

#include "ACSFormat/Structs.h"

#include <SDL.h>
#include <SDL_syswm.h>

enum class TipQuadrant {
	Top,
	Right,
	Bottom,
	Left
};

class BalloonWnd
{
private:
	SDL_Window* Window = nullptr;
	SDL_Renderer* Renderer = nullptr;

	BalloonInfo* BalloonStyle = nullptr;

	const int CornerDiamenter = 10;
	const int TipDepth = 13;
	const int TipSpacing = 10;
	const int TipMiddle = TipSpacing / 2;

	int TipOffsetInLine = 0;

	void Render(Rect bounds);

	void FillTriangle(SDL_Point v1, SDL_Point v2, SDL_Point v3, RGBQuad color);
	bool PointInTriangle(SDL_Point point, SDL_Point p1, SDL_Point p2, SDL_Point p3);
	float Sign(SDL_Point p1, SDL_Point p2, SDL_Point p3);

	void DrawCorner(SDL_Point pos, int diameter, bool flipX = false, bool flipY = false);
	int Distance(int x, int y, int x1, int y1);

public:
	BalloonWnd() = default;

	TipQuadrant TipQuad = TipQuadrant::Left;

	void Setup(BalloonInfo* bi);
	void Move(int x, int y);
	void Show();
	void Hide();
	void Update();
};

