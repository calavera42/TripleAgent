#include "AnimationProvider.h"

FrameInfo* AnimationProvider::GetFrame(uint index)
{
	return &CurrentAnimation.Frames[index];
}

void AnimationProvider::SetFrame(uint index)
{
	Frame = index;
	CurrentFrame = &CurrentAnimation.Frames[index];
}

bool AnimationProvider::CanSpeak()
{
	return GetFrame(Frame)->Overlays.size() > 0;
}

uint AnimationProvider::GetInterval() const
{
	return Interval;
}

void AnimationProvider::Setup(AgentFile* af, std::function<void(void)> animEndNotify)
{
	AgFile = af;
	AnimationEndLogic = animEndNotify;
}

AnimationState AnimationProvider::GetAnimationState()
{
	return AnimState;
}

FrameInfo* AnimationProvider::GetCurrentFrame()
{
	return &CurrentAnimation.Frames[Frame];
}

void AnimationProvider::LoadAnimation(string name)
{
	CurrentAnimation = AgFile->ReadAnimation(name);
	Frame = 0;
	LastFrame = -1;

	Interval = 0;

	StopRequested = false;
	AnimState = AnimationState::Progressing;
}

void AnimationProvider::Pause()
{
	LastState = AnimState;
	AnimState = AnimationState::Paused;
}

void AnimationProvider::Resume()
{
	AnimState = LastState;
}

void AnimationProvider::Update()
{
	FrameInfo* currentFrame = GetFrame(Frame);

	switch (AnimState) {
	case AnimationState::Progressing:
	{
		AdvanceFrame(currentFrame->Branches);
		currentFrame = GetFrame(Frame);

		Interval = currentFrame->FrameDuration;

		bool isLastFrame = Frame == CurrentAnimation.Frames.size() - 1;
		bool isBranchingFrame = currentFrame->FrameDuration == 0;

		if (isLastFrame)
		{
			if (isBranchingFrame)
			{
				AdvanceFrame(currentFrame->Branches);

				if (Frame < CurrentAnimation.Frames.size() - 1)
				{
					Interval = GetFrame(Frame)->FrameDuration;
					LastFrame = Frame;
					break;
				}

				Frame = LastFrame;
			}

			AnimationEndLogic();
		}
		else
			if (isBranchingFrame)
				AdvanceFrame(currentFrame->Branches);
		LastFrame = Frame;
	}
	break;
	case AnimationState::Returning:
		int exitFrame = currentFrame->ExitFrameIndex;

		if (exitFrame == -2
			|| exitFrame == 0
			|| (Frame == CurrentAnimation.Frames.size() - 1 && exitFrame == -1)
			)
		{
			AnimState = AnimationState::Finished;
			AnimationEndLogic();
			break;
		}

		if (exitFrame == -1)
			exitFrame = Frame + 1;

		Frame = exitFrame;
		Interval = GetFrame(Frame)->FrameDuration;
		break;
	}
}

void AnimationProvider::AdvanceFrame(std::vector<BranchInfo>& branches)
{
	if (StopRequested)
		return;

	int rnd = rand() % 100 + 1;
	int prob = 0;

	for (auto& brInfo : branches) {
		prob += brInfo.Probability;

		if (rnd < prob) {
			Frame = brInfo.TargetFrame;
			return;
		}
	}

	Frame++;
}

void AnimationProvider::LoadAnimationFromState(AgentState state)
{
	std::vector<string> stateLookup = {
	L"unknown",
	L"Showing",
	L"Hiding",
	L"GesturingDown",
	L"GesturingUp",
	L"GesturingLeft",
	L"GesturingRight",
	L"Listening",
	L"Hearing",
	L"IdlingLevel1",
	L"IdlingLevel2",
	L"IdlingLevel3",
	L"MovingDown",
	L"MovingUp",
	L"MovingLeft",
	L"MovingRight",
	L"Speaking"
	};

	StateInfo* stateInfo = AgFile->ReadState(stateLookup[(int)state]);

	if (stateInfo == nullptr)
		return;

	string animName = stateInfo->Animations[rand() % (stateInfo->Animations.size() - 1)];

	LoadAnimation(animName);
}