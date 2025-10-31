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

#define AGX_WND_CREATION_FAIL 1
#define AGX_WND_CREATION_SUCCESS 1

enum class AgEventType : uint8_t
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

struct AgEvent;
class IAgentWindow;

class IAgentWindow {
public:
	IAgentWindow() {};

	IAgentWindow(const IAgentWindow&) = delete;
	void operator=(const IAgentWindow&) = delete;

	virtual int Setup(IAgentFile* af, uint16_t langid = 0x416) = 0;

	virtual void UpdateState(AgEvent e) = 0;
	virtual AgEvent QueryEvent() = 0;

	virtual AgRect GetRect() = 0;
	virtual bool IsVisible() = 0;
};

class IBalloonWindow {
public:
	IBalloonWindow() {};

	IBalloonWindow(const IBalloonWindow&) = delete;
	void operator=(const IBalloonWindow&) = delete;

	virtual int Setup(CharacterInfo ci) = 0;

	virtual void UpdateState(AgEvent e) = 0;
};

typedef std::shared_ptr<FrameInfo> FramePointer;
typedef std::shared_ptr<IAgentWindow> WindowPointer;

struct AgEvent {
	AgEventType Type{};
	std::variant<
		FramePointer,
		MouthOverlayType,
		AgPoint,
		bool,
		string,
		int,
		AgRect,
		WindowPointer
	> Data{};
};

extern "C" AGENT_WIN IAgentWindow* CreateAgentWindow();
extern "C" AGENT_WIN IBalloonWindow* CreateBalloonWindow();

extern "C" AGENT_WIN void DestroyAgentWindow(IAgentWindow* ptr);
extern "C" AGENT_WIN void DestroyBalloonWindow(IBalloonWindow* ptr);

extern "C" AGENT_WIN void ProcessMessages();