#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <memory>
#include <stdexcept>

#include <agxdpv.h>

class AgentRendering
{
private:
	IAgentFile* AgFile = nullptr;
	CharacterInfo CharInfo;
	Gdiplus::ColorPalette* AgentPalette = nullptr;

	void DrawFrame(Gdiplus::Graphics* g, uint frameIndex, int offsetX, int offsetY);

	Gdiplus::ColorPalette* CreatePalette(std::vector<RGBQuad> colorPalette);
	std::unique_ptr<Gdiplus::Bitmap> GetFrameBitmap(ImageData& im) const;
public:
	void Setup(IAgentFile* file);

	void Paint(HWND hwnd, std::shared_ptr<FrameInfo> curFrame, MouthOverlayType overlay);

	~AgentRendering();
};

