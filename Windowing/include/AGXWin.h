#pragma once

#ifdef WINDOWING_EXPORTS
	#define AGENT_WIN __declspec(dllexport)
#else
	#define AGENT_WIN __declspec(dllimport)
#endif

#include <inttypes.h>
#include <string>
#include <functional>

#include "../../DataProvider/include/agxdpv.h"

enum class UpdateType {
	None,
	FrameChange,
	MouthChange,
	MoveWindow,
	VisibleChange
};

struct Point {
	uint32_t X;
	uint32_t Y;
};

struct UpdateInfo {
	UpdateType Type;
	union {
		FrameInfo* Frame;
		MouthOverlayType Mouth;
		Point TargetPos;
		bool WindowVisible;
	};
};

class IAgentWindow {
public:
	virtual int Setup(IAgentFile* af, uint16_t langid = 0x416) = 0;

	virtual uint16_t GetWidth() = 0;
	virtual uint16_t GetHeight() = 0;

	virtual bool IsVisible() = 0;

	virtual void UpdateState(UpdateInfo info) = 0;

	virtual void Do(IAgentFile* f) = 0;
};

class IBaloonWindow {
public:
	virtual void Setup(BalloonInfo bi) = 0;

	virtual void Show(std::wstring text) = 0;

	virtual void PaceUpdate() = 0;
};

extern "C" AGENT_WIN IAgentWindow* CreateAgentWindow();
extern "C" AGENT_WIN void DeleteAgentWindow(IAgentWindow* win);