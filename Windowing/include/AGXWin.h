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

#include <agxdpv.h>

enum class EventType : uint8_t
{
	Invalid,

	WindowMouseLDown,
	WindowMouseLUp,
	WindowDragStart,
	WindowMouseRDown,
	WindowMouseRUp,
	WindowDragEnd,

	BalloonWindowHidden,

	AgentBalloonShow,
	AgentBalloonHide,
	AgentBalloonAttach,
	AgentBalloonPace,

	AgentFrameChange,
	AgentMouthChange,
	AgentMoveWindow,
	AgentVisibleChange
};

struct Event {
	EventType Type{};
	std::variant<
		std::shared_ptr<FrameInfo>, 
		MouthOverlayType, 
		AgPoint, 
		bool, 
		string, 
		int, 
		AgRect> Data{};
};

class IAgentWindow {
public:
	virtual int Setup(IAgentFile* af, uint16_t langid = 0x416) = 0;
	virtual AgPoint GetSize() = 0;
	virtual AgPoint GetPos() = 0;
	virtual bool IsVisible() = 0;

	virtual void UpdateState(Event info) = 0;
	virtual Event QueryEvent() = 0;
};

class IBalloonWindow {
public:
	// TODO: dar a opção de modificar o tamanho da fonte 
	// (já q a info passada pelo balloon info é inútil)
	virtual int Setup(CharacterInfo ci) = 0;

	virtual void UpdateState(Event e) = 0;
	virtual Event QueryEvent() = 0;
};

extern "C" AGENT_WIN IAgentWindow* CreateAgentWindow();
extern "C" AGENT_WIN void DeleteAgentWindow(IAgentWindow* win);

extern "C" AGENT_WIN IBalloonWindow* CreateBalloonWindow();
extern "C" AGENT_WIN void DeleteBalloonWindow(IBalloonWindow* win);