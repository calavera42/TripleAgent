#pragma once

#include <inttypes.h>
#include <agxwin.h>

#include "../include/trplagnt.h"

enum Type : uint8_t
{
	Speak,
	Move,
	Gesture,
	Animate
};

enum UpdateResult : uint8_t
{
	Continue,
	Done,

	ActionPause
};

class Request : public IRequest
{
	virtual UpdateResult Update(IAgentWindow* iaw, IBalloonWindow* ibw) = 0;
	virtual UpdateResult Stop(IAgentWindow* iaw, IBalloonWindow* ibw) = 0;

	int GetID() override = 0;
	void Wait() override = 0;

	void OnComplete(void(*callfunc)()) override = 0;
	void OnCancel(void(*callfunc)()) override = 0;
};
