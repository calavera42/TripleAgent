#include "BalloonWnd.h"

void BalloonWnd::Setup(BalloonInfo* bi)
{
	Window = SDL_CreateWindow(
		"bloon",
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		30,
		30,
		SDL_WINDOW_BORDERLESS | 
		SDL_WINDOW_ALWAYS_ON_TOP |
		SDL_WINDOW_HIDDEN
	);

	Renderer = SDL_CreateRenderer(
		Window, 
		-1, 
		SDL_RENDERER_PRESENTVSYNC | 
		SDL_RENDERER_ACCELERATED
	);

	BalloonStyle = bi;

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(Window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	// FIXME: tornar multiplataforma
	// Janela que não rouba o foco, com suporte para transparência e não sai da tela
	SetWindowLong(
		hwnd,
		GWL_EXSTYLE,
		WS_EX_NOACTIVATE |
		WS_EX_LAYERED |
		WS_EX_TOPMOST
	);

	SetLayeredWindowAttributes(hwnd, 0x00FF00FF, 0xff, 1);

	TTF_Init();

	// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta#parameters
	int fontSizePt = (-BalloonStyle->FontHeight * 72) / GetDeviceCaps(GetWindowDC(hwnd), LOGPIXELSY);

	Font = TTF_OpenFont("c:/windows/fonts/arial.ttf", std::abs(BalloonStyle->FontHeight));
	FontSizePt = fontSizePt;
}

void BalloonWnd::AttachToWindow(SDL_Window* agentWindow)
{
	Rect agRect;
	Rect dsRect;

	SDL_GetWindowPosition(agentWindow, &agRect.x, &agRect.y);
	SDL_GetWindowSizeInPixels(agentWindow, &agRect.w, &agRect.h);

	int agDisplay = SDL_GetWindowDisplayIndex(agentWindow);

	SDL_GetDisplayBounds(agDisplay, (SDL_Rect*)&dsRect);

	agRect.x -= dsRect.x;
	agRect.y -= dsRect.y;

	int halfH = agRect.x / (dsRect.w / 2);
	int halfV = agRect.y / (dsRect.h / 2);

	TipQuad = (TipQuadrant)((halfV << 1) | halfH);
 }

void BalloonWnd::UpdateText(string text)
{
	BalloonText = text;

	int tW, tH;

	TTF_SizeUNICODE(Font, (ushort*)text.c_str(), &tW, &tH);

	Rect windowBounds = GetBounds();

	SDL_SetWindowSize(Window, windowBounds.w + tW + CornerDiameter, windowBounds.h + tH + CornerDiameter);
}

void BalloonWnd::Show()
{
	SDL_ShowWindow(Window);
}

void BalloonWnd::Hide()
{
	SDL_HideWindow(Window);
}

void BalloonWnd::Update()
{
	SDL_SetRenderDrawColor(Renderer, 255, 0, 255, 255);
	SDL_RenderClear(Renderer);

	Rect bounds = GetBounds();
	bounds.x = 0;
	bounds.y = 0;

	Render(bounds);

	RenderWrappedText(BalloonText, CornerDiameter, CornerDiameter, BalloonStyle->ForegroundColor);

	SDL_RenderPresent(Renderer);
}

Rect BalloonWnd::GetBounds()
{
	int x, y, w, h;
	SDL_GetWindowPosition(Window, &x, &y);
	SDL_GetWindowSize(Window, &w, &h);

	return { x, y, w, h };
}

void BalloonWnd::Render(Rect bounds)
{
	SDL_Point arcPositions[4] = {
		{bounds.x, bounds.y},
		{bounds.right() - CornerDiameter, bounds.y},
		{bounds.right() - CornerDiameter, bounds.bottom() - CornerDiameter},
		{bounds.x, bounds.bottom() - CornerDiameter}
	};

	SDL_Point tipPoints[3] = {};

	const static SDL_Point offsetsLookup[8] = {
		{ 0, TipDepth }, { 0, -TipDepth },
		{ 0, 0 }, { -TipDepth, 0 },
		{ 0, 0 }, { 0, -TipDepth },
		{ TipDepth, 0 }, { -TipDepth, 0 }
	};

	SDL_Rect balloonRect = { bounds.x, bounds.y, bounds.w, bounds.h };

	switch (TipQuad) {
	case TipQuadrant::Top:
		tipPoints[0] = { bounds.x + TipOffsetInLine + CornerDiameter, bounds.y + TipDepth };
		tipPoints[1] = { bounds.x + TipOffsetInLine + CornerDiameter + TipMiddle, bounds.y };
		tipPoints[2] = { bounds.x + TipOffsetInLine + CornerDiameter + TipSpacing, bounds.y + TipDepth };

		arcPositions[0].y += TipDepth;
		arcPositions[1].y += TipDepth;
		break;
	case TipQuadrant::Right:
		tipPoints[0] = { bounds.right() - TipDepth - 1, bounds.y + TipOffsetInLine + CornerDiameter };
		tipPoints[1] = { bounds.right(), bounds.y + TipOffsetInLine + CornerDiameter + TipMiddle };
		tipPoints[2] = { bounds.right() - TipDepth - 1, bounds.y + TipOffsetInLine + CornerDiameter + TipSpacing };

		arcPositions[1].x -= TipDepth;
		arcPositions[2].x -= TipDepth;
		break;
	case TipQuadrant::Bottom:
		tipPoints[0] = { bounds.x + TipOffsetInLine + CornerDiameter, bounds.bottom() - TipDepth - 1 };
		tipPoints[1] = { bounds.x + TipOffsetInLine + CornerDiameter + TipMiddle, bounds.bottom() };
		tipPoints[2] = { bounds.x + TipOffsetInLine + CornerDiameter + TipSpacing, bounds.bottom() - TipDepth - 1 };

		arcPositions[2].y -= TipDepth;
		arcPositions[3].y -= TipDepth;
		break;
	case TipQuadrant::Left:
		tipPoints[0] = { bounds.x + TipDepth, bounds.y + TipOffsetInLine + CornerDiameter };
		tipPoints[1] = { bounds.x, bounds.y + TipOffsetInLine + CornerDiameter + TipMiddle };
		tipPoints[2] = { bounds.x + TipDepth, bounds.y + TipOffsetInLine + CornerDiameter + TipSpacing };

		arcPositions[0].x += TipDepth;
		arcPositions[3].x += TipDepth;
		break;
	}

	SDL_SetRenderDrawColor(
		Renderer, 
		BalloonStyle->BackgroundColor.Red, 
		BalloonStyle->BackgroundColor.Green, 
		BalloonStyle->BackgroundColor.Blue, 
		255
	);

	int offset = (int)TipQuad * 2;

	balloonRect.x += offsetsLookup[offset].x;
	balloonRect.y += offsetsLookup[offset].y;
	balloonRect.w += offsetsLookup[offset + 1].x;
	balloonRect.h += offsetsLookup[offset + 1].y;

	SDL_RenderFillRect(Renderer, &balloonRect);

	SDL_SetRenderDrawColor(
		Renderer,
		BalloonStyle->BorderColor.Red,
		BalloonStyle->BorderColor.Green,
		BalloonStyle->BorderColor.Blue,
		255
	);

	SDL_RenderDrawRect(Renderer, &balloonRect);

	SDL_SetRenderDrawColor(
		Renderer,
		BalloonStyle->BackgroundColor.Red,
		BalloonStyle->BackgroundColor.Green,
		BalloonStyle->BackgroundColor.Blue,
		255
	);

	FillTriangle(tipPoints[0], tipPoints[1], tipPoints[2], BalloonStyle->BackgroundColor);

	SDL_SetRenderDrawColor(
		Renderer, 
		BalloonStyle->BorderColor.Red, 
		BalloonStyle->BorderColor.Green, 
		BalloonStyle->BorderColor.Blue, 
		255
	);

	SDL_RenderDrawLine(Renderer, tipPoints[0].x, tipPoints[0].y, tipPoints[1].x, tipPoints[1].y);
	SDL_RenderDrawLine(Renderer, tipPoints[2].x, tipPoints[2].y, tipPoints[1].x, tipPoints[1].y);

	DrawCorner(arcPositions[0], CornerDiameter, false, false); // top left
	DrawCorner(arcPositions[1], CornerDiameter, true, false); // top right
	DrawCorner(arcPositions[2], CornerDiameter, true, true); // bottom right
	DrawCorner(arcPositions[3], CornerDiameter, false, true); // bottom left
}


/*
* TODO: melhorar esse algoritmo.
*
* - dar suporte a outros estilos de texto (negrito, itálico etc);
* - consertar algumas coisas da renderização de texto, ex: se for um bloco
* de texto sem espaços o balão não mostra nada.
*/
void BalloonWnd::RenderWrappedText(string t, int posX, int posY, RGBQuad color)
{
	int lastSpace = 0;

	int y = 0;

	int width = 0;
	int height = 0;

	string rawText = t;

	bool vertical = (int)TipQuad % 2 == 0;

	for (int i = 0; i < rawText.length(); i++) 
	{
		wchar_t curChar = rawText[i];
		bool lineBreak = curChar == L'\n';
		string ln = L"";

		if (curChar == L' ' && i != 0)
			lastSpace = i;

		if (i <= BalloonStyle->CharsPerLine && !lineBreak)
			continue;

		int breakPoint = (lineBreak) ? i : lastSpace;

		ln = rawText.substr(0, breakPoint);
		rawText = rawText.substr(breakPoint + 1, rawText.length() - breakPoint + 1);

		int texWidth, texHeight;

		TTF_SizeUNICODE(Font, (ushort*)ln.c_str(), &texWidth, &texHeight);

		width = std::max({ texWidth, width });
		height += texHeight;

		RenderText(
			ln,
			(TipQuad == TipQuadrant::Left ? TipDepth : 0) + posX,
			(TipQuad == TipQuadrant::Top ? TipDepth : 0) + posY + y * (texHeight),
			color
		);

		i = 0;
		y++;
	}

	SDL_SetWindowSize(
		Window,
		CornerDiameter * 2 + (vertical ? 0 : TipDepth) + width,
		CornerDiameter * 2 + (vertical ? TipDepth : 0) + height
	);
}

Rect BalloonWnd::RenderText(string text, int x, int y, RGBQuad color)
{
	SDL_Surface* srf = TTF_RenderUNICODE_Blended(
		Font,
		(ushort*)text.c_str(),
		{
			color.Red,
			color.Green,
			color.Blue,
			255
		}
	);

	SDL_Texture* tex = SDL_CreateTextureFromSurface(Renderer, srf);

	SDL_Rect targetRect = {
		x,
		y,
		srf->w,
		srf->h
	};

	SDL_RenderCopy(Renderer, tex, nullptr, &targetRect);

	SDL_FreeSurface(srf);
	SDL_DestroyTexture(tex);

	return { 0, 0, srf->w, srf->h };
}

void BalloonWnd::FillTriangle(SDL_Point v1, SDL_Point v2, SDL_Point v3, RGBQuad color)
{
	int minX = std::min({ v1.x, v2.x, v3.x }) - 1;
	int maxX = std::max({ v1.x, v2.x, v3.x }) + 1;

	int minY = std::min({ v1.y, v2.y, v3.y }) - 1;
	int maxY = std::max({ v1.y, v2.y, v3.y }) + 1;
	SDL_SetRenderDrawColor(Renderer, color.Red, color.Green, color.Blue, 255);

	for (int x = minX; x < maxX; x++)
		for (int y = minY; y < maxY; y++)
			if (PointInTriangle({ x, y }, v1, v2, v3)) 
			{
				SDL_RenderDrawPoint(Renderer, x, y);
			}
}

bool BalloonWnd::PointInTriangle(SDL_Point point, SDL_Point p1, SDL_Point p2, SDL_Point p3)
{
	float d1, d2, d3;
	bool hasNeg, hasPos;

	d1 = Sign(point, p1, p2);
	d2 = Sign(point, p2, p3);
	d3 = Sign(point, p3, p1);

	hasNeg = d1 < 0 || d2 < 0 || d3 < 0;
	hasPos = d1 > 0 || d2 > 0 || d3 > 0;

	return !(hasNeg && hasPos);
}

float BalloonWnd::Sign(SDL_Point p1, SDL_Point p2, SDL_Point p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
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
				color = BalloonStyle->BorderColor;
			else if (distance < diameter)
				color = BalloonStyle->BackgroundColor;

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
