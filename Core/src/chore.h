#pragma once

#include <inttypes.h>
#include <agxwin.h>
#include <future>

#include "../include/trplagnt.h"
#include "agent.h"

enum Type : uint8_t
{
	Speak,
	Move,
	Gesture,
	Animate
};

class Chore
{
public:
	virtual Type GetType() = 0;

	virtual int Update(Agent* current, IAgentWindow* iaw, IBalloonWindow* ibw) = 0;
	virtual int Return(Agent* current, IAgentWindow* iaw, IBalloonWindow* ibw) = 0;
};
