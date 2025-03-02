#pragma once

#include "ACSFormat/AgentFile.h"
#include "BalloonWnd.h"
#include "GlobalErrorHandler.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL_thread.h>

#include <chrono>
#include <queue>
#include <assert.h>
#include <thread>

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

enum class AnimationState {
	Paused,
	Progressing,
	Returning,
	Finished
};

enum class RequestType {
	Speak,
	Animate,
	Think,
	Hide,
	Move,
	Point
};

struct Request {
	RequestType Type;
	void* Data;
};

class Agent
{
private:
	bool Shown;

	std::thread AgentThread;

	// AgentWindow
	SDL_Window* Window;
	SDL_Renderer* Renderer;

	void SetupWindow();

	void WndLoop();

	void Render();

	void ShowWindow();
	void HideWindow();

	static SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data);
	// ------------

	// Audio
	static bool AudioInitialized;

	static std::map<uint, AudioInfo> AudioData;
	static byte UsedChannels;

	void PlayAudio(uint index);
	static void AudioFinishedCallback(int channel);
	// ------------

	// Fala 
	BalloonWnd Balloon;
	// ------------

	// Renderização
	SDL_Texture* AgentTex = {};
	SDL_Surface* AgentSur = {};
	// ------------

	// Animação
	AnimationInfo CurrentAnimation;
	int Frame = 0;
	int LastFrame = 0;

	AnimationState AnimState = AnimationState::Progressing;

	uint Interval = 0;

	bool AnimPaused = false;
	bool StopRequested = false;

	void LoadAnimation(string name);

	void UpdateAnim();

	void AdvanceFrame(std::vector<BranchInfo>& branches);
	void PrepareFrame(int index);

	void LoadAnimationFromState(AgentState state);

	FrameInfo* GetFrame(uint index);
	bool CanSpeakOnFrame(uint frame);
	// ------------

	// ThreadMain
	void ThreadMain();
	// ------------

	// Fila
	AgentState CurrentState;
	Request CurrentRequest = {};

	std::queue<Request> RequestQueue;
	void Queue(Request req);

	void AnimationEndLogic();
	// ------------
public:
	Agent(AgentFile* agf);

	AgentFile* AgFile;

	void DoStuff();


};

