#pragma once

#include "../request.h"

#include <AGXDpv.h>

class Animation : public Request
{
private:
	IAgentFile* _agentFile;
	AnimationInfo _animation;
	int _currentFrame;

public:
	Animation(int id, string animName, IAgentFile* agentFile)
	{
		_currentFrame = 0;
		_animation = agentFile->GetAnimationInfo(animName);
	}

	UpdateResult Update(IAgentWindow* iaw, IBalloonWindow* ibw) override;
	UpdateResult Stop(IAgentWindow* iaw, IBalloonWindow* ibw) override;

	int GetID() override;
	void Wait() override;
	void OnComplete(void(*callfunc)()) override;
	void OnCancel(void(*callfunc)()) override;
};

