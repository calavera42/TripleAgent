#include "AgentRendering.h"

using namespace Gdiplus;

void AgentRendering::Setup(IAgentFile* file)
{
	GdiplusStartupInput gdiStart;
	ULONG_PTR gdiToken;
	GdiplusStartup(&gdiToken, &gdiStart, nullptr);

	AgFile = file;
	CharacterInfo charInfo = file->GetCharacterInfo();

	AgentPalette = CreatePalette(charInfo.ColorTable);
}

void AgentRendering::Paint(Gdiplus::Graphics* g, std::shared_ptr<FrameInfo> curFrame, MouthOverlayType overlay)
{
	bool drawOverlay = !curFrame->Overlays.empty() && overlay != MouthOverlayType::Closed;
	OverlayInfo oi;

	if (drawOverlay)
		oi = curFrame->Overlays[(int)overlay];

	// note que o agente é desenhado de trás para frente
	for (int i = (int)(curFrame->Images.size() - 1); i >= 0; i--)
	{
		if (drawOverlay && i == 0 && oi.ReplaceTopImage)
			continue; // não desenhar essa camada se o overlay serve de substituto

		FrameImage fi = curFrame->Images[i];

		DrawFrame(g, fi.ImageIndex, fi.OffsetX, fi.OffsetY);
	}

	if (drawOverlay)
		DrawFrame(g, oi.ImageIndex, oi.OffsetX, oi.OffsetY);
}

AgentRendering::~AgentRendering()
{
	free(AgentPalette);
}

void AgentRendering::DrawFrame(Gdiplus::Graphics* g, uint frameIndex, int offsetX, int offsetY)
{
	ImageAttributes imgAttr = {};
	CharacterInfo ci = AgFile->GetCharacterInfo();
	RGBQuad rq = ci.ColorTable[ci.TransparencyIndex];

	imgAttr.SetColorKey(
		Color(rq.Red, rq.Green, rq.Blue),
		Color(rq.Red, rq.Green, rq.Blue)
	);

	ImageData im = AgFile->ReadImageData(frameIndex);
	auto bm = GetFrameBitmap(im);

	Rect destRect = {
		offsetX, offsetY,
		im.Width, im.Height
	};

	g->DrawImage(bm.get(), destRect, 0, 0, im.Width, im.Height, Unit::UnitPixel, &imgAttr);
}

Gdiplus::ColorPalette* AgentRendering::CreatePalette(std::vector<RGBQuad> colorPalette)
{
	size_t structSize = sizeof(ColorPalette) + (colorPalette.size() * sizeof(uint32_t));
	ColorPalette* cp = (ColorPalette*)malloc(structSize);

	if (!cp)
		throw std::runtime_error("Falha ao alocar paleta do personagem, memória insuficiente.");

	memset(cp, 0, structSize);

	cp->Flags = 0;
	cp->Count = static_cast<uint>(colorPalette.size());

	for (size_t i = 0; i < cp->Count; i++)
	{
		RGBQuad& rq = colorPalette[i];

		cp->Entries[i] =
			(255 << 24) |
			(rq.Red << 16) |
			(rq.Green << 8) |
			rq.Blue;
	}

	return cp;
}

std::unique_ptr<Gdiplus::Bitmap> AgentRendering::GetFrameBitmap(ImageData& id) const
{
	auto b = std::make_unique<Bitmap>(
		(INT)id.Width,
		(INT)id.Height,
		-(INT)id.Width, // é 8bpp e está de cabeça para baixo (vide msdn: DIB)
		PixelFormat8bppIndexed,
		(byte*)id.Data.get() + (id.Size - id.Width)
	);

	if (b->GetLastStatus() != Status::Ok)
		throw std::runtime_error("AgentRendering::BitmapFromImageData : Falha ao criar bitmap. Gdistatus: " + b->GetLastStatus());

	b->SetPalette(AgentPalette);

	return b;
}