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

class AgentWindow : public IAgentWindow
{
private:
	uint16_t Width = {};
	uint16_t Height = {};

	HWND Handle = nullptr;

	AgentRendering AgRender{};

	std::shared_ptr<FrameInfo> CurFrame;
	MouthOverlayType CurMouth;

	std::queue<Event> AgentUpdateQueue{};
	std::mutex AgentQueueMutex{};

	std::queue<Event> WindowEventsQueue{};
	std::mutex WindowEventsMutex{};

	bool WindowStartDragging = false;

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void MessageLoop();
	void InternalSetup(IAgentFile* f, uint16_t langId, std::promise<int>& prom);
	Event GetAgentUpdate();
	void PushWindowEvent(Event e);
	void ProcessAgentUpdate();

public:
	int Setup(IAgentFile* f, uint16_t langid = 0x416) override;
	bool IsVisible() override;
	void UpdateState(Event info) override;
	Event QueryEvent() override;
	AgPoint GetSize() override;
	AgPoint GetPos() override;
};