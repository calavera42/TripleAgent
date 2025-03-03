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

	std::function<void(void)> AnimationEndLogic;

	AnimationInfo CurrentAnimation;
	int Frame = 0;
	int LastFrame = 0;

	AnimationState AnimState = AnimationState::Progressing;
	AnimationState LastState = AnimationState::Progressing;

	uint Interval = 0;

	bool StopRequested = false;
	void AdvanceFrame(std::vector<BranchInfo>& branches);

	FrameInfo* GetFrame(uint index);
public:
	void Setup(AgentFile* af, std::function<void(void)> animEndNotify);

	AnimationState GetAnimationState();
	FrameInfo* GetCurrentFrame();
	uint GetInterval();

	bool CanSpeak();

	void Update();

	void LoadAnimationFromState(AgentState state);
	void LoadAnimation(string name);

	void Pause();
	void Resume();
};

