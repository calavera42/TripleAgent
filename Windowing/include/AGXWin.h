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

	virtual int Setup(std::shared_ptr<IAgentFile> af, std::function<void(AgEvent)> callback, LangId langid = LangId::pt_BR) = 0;
	virtual void UpdateState(AgEvent e) = 0;

	virtual AgRect GetRect() = 0;
	virtual bool IsVisible() = 0;
};

class IBalloonWindow {
public:
	IBalloonWindow() {};

	IBalloonWindow(const IBalloonWindow&) = delete;
	void operator=(const IBalloonWindow&) = delete;

	virtual int Setup(CharacterInfo ci, std::function<void(AgEvent)> callback) = 0;
	virtual void UpdateState(AgEvent e) = 0;
};

typedef std::shared_ptr<IAgentWindow> WindowPointer;

struct AgEvent {
	AgEventType Type{};
	std::variant<
		FrameInfo,
		MouthOverlayType,
		AgPoint,
		bool,
		std::string,
		int,
		AgRect,
		WindowPointer
	> Data{};
};

AGENT_WIN std::shared_ptr<IAgentWindow> CreateAgentWindow();
AGENT_WIN std::shared_ptr<IBalloonWindow> CreateBalloonWindow();