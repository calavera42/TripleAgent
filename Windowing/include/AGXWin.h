#pragma once

#ifdef WINDOWING_EXPORTS
	#define AGENT_WIN __declspec(dllexport)
#else
	#define AGENT_WIN __declspec(dllimport)
#endif

#include <inttypes.h>
#include <string>
#include <functional>
#include <variant>

#include "../../DataProvider/include/agxdpv.h"

enum class EventType : uint8_t
{
	Invalid,

	WindowMouseLDown,
	WindowMouseLUp,
	WindowDragStart,
	WindowMouseRDown,
	WindowMouseRUp,
	WindowDragEnd,

	AgentFrameChange,
	AgentMouthChange,
	AgentMoveWindow,
	AgentVisibleChange
};

struct AgPoint {
	int32_t X;
	int32_t Y;
};

struct Event {
	EventType Type{};
	std::variant<std::shared_ptr<FrameInfo>, MouthOverlayType, AgPoint, bool> Data{};
};

class IAgentWindow {
public:
	virtual int Setup(IAgentFile* af, uint16_t langid = 0x416) = 0;
	virtual uint16_t GetWidth() = 0;
	virtual uint16_t GetHeight() = 0;
	virtual bool IsVisible() = 0;
	virtual void UpdateState(Event info) = 0;
	virtual Event QueryInfo() = 0;
};

class IBalloonWindow {
public:
	virtual int Setup(CharacterInfo ci) = 0;
	virtual void Show(std::wstring text) = 0;
	virtual void PaceUpdate() = 0;
};

extern "C" AGENT_WIN IAgentWindow* CreateAgentWindow();
extern "C" AGENT_WIN void DeleteAgentWindow(IAgentWindow* win);

extern "C" AGENT_WIN IBalloonWindow* CreateBalloonWindow();
extern "C" AGENT_WIN void DeleteBalloonWindow(IBalloonWindow* win);