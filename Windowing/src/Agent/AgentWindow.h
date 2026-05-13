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
	int _wndCount = 0;

	uint16_t Width = {};
	uint16_t Height = {};

	HWND Handle = nullptr;

	AgentRendering AgRender{};

	std::wstring WndClassName{};

	int WindowStyle = WS_POPUP;
	int WindowExStyle = WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED;


public:
	int Setup(std::shared_ptr<IAgentFile> af, std::function<void(AgEvent)> callback, LangId langid = LangId::pt_BR) override;

	void UpdateState(AgEvent e) override;

	bool IsVisible() override;
	AgRect GetRect() override;

	~AgentWindow();
};