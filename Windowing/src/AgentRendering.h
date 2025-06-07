#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <stdexcept>

#include "../../DataProvider/include/agxdpv.h"

class AgentRendering
{
private:
	IAgentFile* AgFile;
	Gdiplus::ColorPalette* AgentPalette = nullptr;

	Gdiplus::ColorPalette* CreatePalette(std::vector<RGBQuad> colorPalette);
	std::unique_ptr<Gdiplus::Bitmap> BitmapFromImageData(ImageData* id);
public:
	void Setup(IAgentFile* file);

	void Paint(Gdiplus::Graphics* g, FrameInfo* curFrame, MouthOverlayType overlay);
};

