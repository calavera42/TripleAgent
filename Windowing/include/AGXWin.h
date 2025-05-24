#pragma once

#ifdef WINDOWING_EXPORTS
	#define AGENT_WIN __declspec(dllexport)
#else
	#define AGENT_WIN __declspec(dllimport)
#endif

#include <inttypes.h>
#include <string>
#include <functional>

class IAgentWindow {
public:
	virtual int Setup(uint16_t width, uint16_t height) = 0;

	virtual void StartMessageLoop() = 0;

	virtual uint16_t GetWidth() = 0;
	virtual uint16_t GetHeight() = 0;

	virtual bool IsVisible() = 0;

	virtual void RegisterUserFuntion(std::function<void(uint32_t*)> func) = 0;

	virtual void SetPosition(uint16_t x, uint16_t y) = 0;

	virtual void Hide() = 0;
	virtual void Show() = 0;
};

extern "C" AGENT_WIN IAgentWindow* CreateAgentWindow();
extern "C" AGENT_WIN void DeleteAgentWindow(IAgentWindow* win);