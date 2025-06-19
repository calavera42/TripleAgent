#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <future>

#include "../../include/AGXWin.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define hInstDll ((HINSTANCE)&__ImageBase)

class BalloonWindow : public IBalloonWindow
{
private:
	uint32_t Width;
	uint32_t Height;

	HWND Handle;

	BalloonInfo BalloonCfg;
	CharacterInfo CharInfo;

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void InternalSetup(CharacterInfo ci, std::promise<int>& prom);
	AgRect CalcWinSize(AgRect textSize);
	void MessageLoop();

public:
	int Setup(CharacterInfo ci) override;
	void Show(std::wstring text) override;
	void PaceUpdate() override;
};
