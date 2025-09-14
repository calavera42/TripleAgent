#pragma once

#include "../../include/AGXWin.h"
#include "AgentRendering.h"

#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>

#include <cassert>
#include <thread>
#include <future>
#include <queue>

#include <inttypes.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define hInstDll ((HINSTANCE)&__ImageBase)

#define WM_NEWEVENT (WM_APP + 0x0001)

class AgentWindow : public IAgentWindow
{
private:
	int WndCount = 0;

	uint16_t Width = {};
	uint16_t Height = {};

	HWND Handle = nullptr;

	AgentRendering AgRender{};

	std::wstring WndClassName{};

	int WindowStyle = WS_POPUP;
	int WindowExStyle = WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED;

	std::shared_ptr<FrameInfo> CurFrame;
	MouthOverlayType CurMouth{};

	std::mutex UpdateMutex{};
	std::queue<AgEvent> AgentUpdates{};
	std::mutex EventsMutex{};
	std::queue<AgEvent> WindowEvents{};

	bool WindowStartDragging = false;

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int InternalSetup(IAgentFile* f, uint16_t langId);
	void PushWindowEvent(AgEvent e);
	void ProcessUserEvent();

	AgEvent PopUserEvent();

public:
	int Setup(IAgentFile* f, uint16_t langid = 0x416) override;

	void UpdateState(AgEvent e) override;
	AgEvent QueryEvent() override;

	bool IsVisible() override;
	AgRect GetRect() override;

	~AgentWindow();
};