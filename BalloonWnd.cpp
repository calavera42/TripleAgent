#include "BalloonWnd.h"

void BalloonWnd::Setup()
{
	Window = SDL_CreateWindow(
		"bloon",
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		180, 
		80, 
		SDL_WINDOW_BORDERLESS | 
		SDL_WINDOW_ALWAYS_ON_TOP
	);

	Renderer = SDL_CreateRenderer(
		Window, 
		-1, 
		SDL_RENDERER_PRESENTVSYNC | 
		SDL_RENDERER_ACCELERATED
	);

	// TODO: fazer a janela transparente
}

void BalloonWnd::Render(Rect bounds)
{
	SDL_Point arcPositions[4] = {
		{bounds.x, bounds.y},
		{bounds.right() - CornerDiamenter, bounds.y},
		{bounds.right() - CornerDiamenter, bounds.bottom() - CornerDiamenter},
		{bounds.x, bounds.bottom() - CornerDiamenter}
	};

	SDL_Point tipPoints[3] = {};

	SDL_Point offsetsLookup[8] = {
		{ 0, TipDepth}, { 0, -TipDepth },
		{ TipDepth, 0}, { -TipDepth, 0 },
		{ 0, TipDepth }, { 0, -TipDepth },
		{ TipDepth, 0}, { -TipDepth, 0 },
	};

	SDL_Rect balloonRect = { bounds.x, bounds.y, bounds.w, bounds.h };


}

void BalloonWnd::DrawCorner(SDL_Point pos, int diameter, bool flipX, bool flipY)
{
	int startX = pos.x;
	int startY = pos.y;

	int xDec = (flipX ? diameter + 1 : 0);
	int yDec = (flipY ? diameter + 1 : 0);

	for (int x = 0; x < diameter; x++)
		for (int y = 0; y < diameter; y++)
		{
			int distance = Distance(x, y, diameter - xDec, diameter - yDec);
			RGBQuad color = { 255, 0, 255 };

			if (distance == diameter)
				color = BalloonStyle.BorderColor;
			else if (distance < diameter)
				color = BalloonStyle.BackgroundColor;

			SDL_SetRenderDrawColor(Renderer, color.Red, color.Green, color.Blue, 255);
			SDL_RenderDrawPoint(Renderer, x + startX, y + startY);
		}
}

int BalloonWnd::Distance(int x, int y, int x1, int y1)
{
	int a = x1 - x;
	int b = y1 - y;

	return (int)roundf(sqrtf(a * a + b * b));
}
