#pragma once

#include "../include/AGXWin.h"

#include <windows.h>
#include <cassert>
#include <thread>
#include <future>
#include <inttypes.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define hInstDll ((HINSTANCE)&__ImageBase)

class AgentWindow : public IAgentWindow
{
private:
	uint16_t Width = {};
	uint16_t Height = {};
	size_t PixelBufferSize = {};

	HWND Handle = nullptr;
	HBITMAP FrameBitmap = nullptr;

	static LRESULT CALLBACK IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	std::shared_ptr<uint32_t*> FrameBuffer;

	std::function<void(uint32_t*)> UserDrawFunction = nullptr;

	void MessageLoop() const;

	void SetupBitmap();
public:
	// Herdado por meio de IAgentWindow
	int Setup(uint16_t width, uint16_t height) override;

	uint16_t GetWidth() override;
	uint16_t GetHeight() override;

	bool IsVisible() override;

	void RegisterUserFuntion(std::function<void(uint32_t*)> func) override;

	void SetPosition(uint16_t x, uint16_t y) override;

	void Hide() override;
	void Show() override;

	// Herdado por meio de IAgentWindow
	void StartMessageLoop() override;
};