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

void AgentRendering::Paint(Gdiplus::Graphics* g, FrameInfo* curFrame, MouthOverlayType overlay)
{
	ImageAttributes imgAttr;

	// TODO: renderizar overlays

	imgAttr.SetColorKey(
		Color(0, 255, 255),
		Color(0, 255, 255)
	);

	g->Clear(Color(255, 0, 255));

	for (int i = curFrame->Images.size() - 1; i >= 0; i--)
	{
		FrameImage fi = curFrame->Images[i];
		ImageData im = AgFile->ReadImageData(fi.FrameIndex);

		auto bm = BitmapFromImageData(&im);

		Rect destRect = {
			fi.OffsetX, fi.OffsetY,
			im.Width, im.Height
		};

		g->DrawImage(bm.get(), destRect, 0, 0, im.Width, im.Height, Unit::UnitPixel, &imgAttr);

		delete bm.release();
		AgFile->FreeImageData(im);
	}
}

Gdiplus::ColorPalette* AgentRendering::CreatePalette(std::vector<RGBQuad> colorPalette)
{
	int structSize = sizeof(ColorPalette) + (colorPalette.size() * sizeof(uint32_t));
	ColorPalette* cp = (ColorPalette*)malloc(structSize);

	if (!cp)
		throw std::runtime_error("AgentRendering::CreatePalette : Falha ao alocar paleta do personagem, memória insuficiente.");

	memset(cp, 0, structSize);

	cp->Flags = 0;
	cp->Count = colorPalette.size();

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

std::unique_ptr<Gdiplus::Bitmap> AgentRendering::BitmapFromImageData(ImageData* id)
{
	auto b = std::make_unique<Bitmap>(
		(INT)id->Width,
		(INT)id->Height,
		-(INT)id->Width, // é 8bpp e está de cabeça para baixo (vide msdn: DIB)
		PixelFormat8bppIndexed,
		(byte*)id->Buffer + (id->Size - id->Width)
	);

	if (b->GetLastStatus() != Status::Ok)
		throw std::runtime_error("AgentRendering::BitmapFromImageData : Falha ao criar bitmap. Gdistatus: " + b->GetLastStatus());

	b->SetPalette(AgentPalette);

	return b;
}