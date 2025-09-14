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

#define WM_NEWEVENT (WM_APP + 0x0002)

class BalloonWindow : public IBalloonWindow
{
private:
	HWND Handle{};

	BalloonInfo BalloonCfg;
	CharacterInfo CharInfo;

	std::wstring WndClassName{};

	int WindowStyle = WS_POPUP;
	int WindowExStyle = WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT;

	IAgentWindow* Parent = nullptr;

	IBalloonRenderer* BalloonRenderer = nullptr;

	int TipPosition = 0;
	TipQuadrant TipType = TipQuadrant::Top;

	string BalloonText;
	int SpeechPace = 0;

	int WorkingRectMargin = -10;

	std::shared_ptr<IAgentWindow> OwnerAgentWnd;

	AgRect WindowRect;
	AgRect AgentRect;

	std::mutex UpdateMutex{};
	std::queue<AgEvent> AgentUpdates{};

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int InternalSetup(CharacterInfo ci);

	void Redraw();

	void Place(Gdiplus::Rect agRect, Gdiplus::Rect wkRect, TipQuadrant tq);

	void ProcessUserEvents();
	AgEvent PopUserEvent();

	void Show(string t);
	void Hide();
	void AttachToAgent(std::shared_ptr<IAgentWindow> iaw);

	TipQuadrant GetBestTQ(int dist[4]);

public:
	int Setup(CharacterInfo ci) override;

	void UpdateState(AgEvent e) override;

	~BalloonWindow();
};
