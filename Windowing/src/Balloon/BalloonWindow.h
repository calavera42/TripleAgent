#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <chrono>
#include <queue>
#include <future>
#include <tuple>

#include "../../include/AGXWin.h"
#include "BalloonRendering/SpeechBalloonRenderer.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define hInstDll ((HINSTANCE)&__ImageBase)

class BalloonWindow : public IBalloonWindow
{
private:
	HWND Handle;

	BalloonInfo BalloonCfg;
	CharacterInfo CharInfo;

	int WindowStyle;
	int WindowExStyle;

	IAgentWindow* Parent;

	IBalloonRenderer* BalloonRenderer;

	int TipPosition = 0;
	TipQuadrant TipType = TipQuadrant::Top;
	string BalloonText;

	AgRect WindowRect;
	AgRect AgentRect;

	std::mutex WindowEventMutex;
	std::queue<Event> WindowEvents;

	std::atomic_bool UpdateAdded;
	std::mutex UserUpdateMutex;
	std::queue<Event> UserEvents;

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void InternalSetup(CharacterInfo ci, std::promise<int>& prom);
	void MessageLoop();

	void ProcessUserEvent();

	void Place(TipQuadrant tq, Gdiplus::Rect agRect, Gdiplus::Rect wkRect);

public:
	int Setup(CharacterInfo ci) override;

	void UpdateState(Event e) override;
	Event QueryEvent() override;

	~BalloonWindow();
};
