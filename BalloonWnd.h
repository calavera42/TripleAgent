#pragma once

#include "ACSFormat/Structs.h"

#include <SDL3/SDL.h>
#include <SDL_ttf.h>
#include <locale>
#include <codecvt>

enum class TipQuadrant {
	Top,
	Right,
	Bottom,
	Left,
};

enum class WindowQuadrant {
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

class BalloonWnd
{
private:
	bool Created = false;

	SDL_Window* Window = nullptr;
	SDL_Renderer* Renderer = nullptr;

	BalloonInfo* BalloonStyle = nullptr;

	TTF_Font* Font = nullptr;
	string BalloonText = L"";

	SDL_Surface* TextSurface = nullptr;
	SDL_Texture* TextTexture = nullptr;

	const int CornerDiameter = 12;
	const int TipDepth = 16;
	const int TipSpacing = 10;
	const int TipMiddle = TipSpacing / 2;

	int FontSizePt = 0;

	int TipOffsetInLine = 0;

	Rect GetBounds();

	void RenderBalloonShape(Rect bounds);

	void PrepareText(string text, int posX, int posY, RGBQuad color);

	void FillTriangle(SDL_Point v1, SDL_Point v2, SDL_Point v3, RGBQuad color);
	bool IsPointInTriangle(SDL_Point point, SDL_Point p1, SDL_Point p2, SDL_Point p3);
	float Sign(SDL_Point p1, SDL_Point p2, SDL_Point p3);

	void DrawCorner(SDL_Point pos, int diameter, bool flipX = false, bool flipY = false);
	int Distance(int x, int y, int x1, int y1);

public:
	BalloonWnd() = default;

	bool Shown = false;

	TipQuadrant TipQuad = TipQuadrant::Left;

	int Setup(BalloonInfo* bi);

	void AttachToWindow(SDL_Window * agentWindow);
	void UpdateText(string text);

	void Show();
	void Hide();

	void Render();
};

