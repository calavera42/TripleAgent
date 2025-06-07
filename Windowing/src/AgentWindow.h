#pragma once

#include "../include/AGXWin.h"
#include "AgentRendering.h"

#include <windows.h>
#include <cassert>
#include <thread>
#include <future>
#include <inttypes.h>
#include <gdiplus.h>
#include <queue>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define hInstDll ((HINSTANCE)&__ImageBase)

class AgentWindow : public IAgentWindow
{
private:
	uint16_t Width = {};
	uint16_t Height = {};

	AgentRendering AgRender{};

	HWND Handle = nullptr;
	FrameInfo* CurFrame = nullptr;

	std::queue<UpdateInfo> UpdateQueue{};
	std::mutex QueueMutex{};

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void MessageLoop();
	void InternalSetup(IAgentFile* f, uint16_t langId, std::promise<int>& prom);

	UpdateInfo GetUpdate();
	void ProcessUpdate();
public:
	int Setup(IAgentFile* f, uint16_t langid = 0x416) override;

	uint16_t GetWidth() override;
	uint16_t GetHeight() override;

	bool IsVisible() override;

	void UpdateState(UpdateInfo info) override;

	void Do(IAgentFile* f) override;
};