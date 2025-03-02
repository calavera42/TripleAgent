#pragma once

#include "../ACSFormat/AgentFile.h"

enum class AnimationState {
	Paused,
	Progressing,
	Returning,
	Finished
};

enum class AgentState
{
	None,
	Showing,
	Hiding,
	GesturingDown,
	GesturingUp,
	GesturingLeft,
	GesturingRight,
	Listening,
	Hearing,
	IdlingLevel1,
	IdlingLevel2,
	IdlingLevel3,
	MovingDown,
	MovingUp,
	MovingLeft,
	MovingRight,
	Speaking
};

class AnimationProvider
{
private:
	AgentFile* AgFile;

	std::function<void()> AnimationEndLogic;

	AnimationInfo CurrentAnimation;
	int Frame = 0;
	int LastFrame = 0;

	AnimationState AnimState = AnimationState::Progressing;

	uint Interval = 0;

	bool StopRequested = false;

	void LoadAnimation(string name);

	void UpdateAnim();

	void AdvanceFrame(std::vector<BranchInfo>& branches);

	void LoadAnimationFromState(AgentState state);

	FrameInfo* GetFrame(uint index);
public:
	void Setup(AgentFile* af, std::function<void()> animEndNotify);

	AnimationState GetAnimationState();
	FrameInfo* GetCurrentFrame();
	bool CanSpeak();

};

