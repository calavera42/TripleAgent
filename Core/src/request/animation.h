#pragma once

#include "../chore.h"

#include <AGXDpv.h>

#define REQUEST_DONE -1

class Animation : public Chore
{
private:
	AnimationInfo _animation;
	int _id;

	int _lastValidFrame;

	int _currentFrame;
	int _animationLength;

	FramePointer GetFrame(int frame);
	void DoFrameProceed(std::vector<BranchInfo>& branches);

public:
	Animation(int id, std::wstring animName, IAgentFile* agentFile);

	int Update(Agent* current, IAgentWindow* iaw, IBalloonWindow* ibw) override;
	int Return(Agent* current, IAgentWindow* iaw, IBalloonWindow* ibw) override;

	Type GetType() override;
};

