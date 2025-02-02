#pragma once

#include "ACSFormat/AgentFile.h"
#include "BalloonWnd.h"

#include <thread>
#include <chrono>
#include <queue>

enum class States
{
	Showing = 1,
	Hiding = 2,
	GesturingDown = 3,
	GesturingUp = 4,
	GesturingLeft = 5,
	GesturingRight = 6,
	Listening = 7,
	Hearing = 8,
	IdlingLevel1 = 9,
	IdlingLevel2 = 10,
	IdlingLevel3 = 11,
	MovingDown = 12,
	MovingUp = 13,
	MovingLeft = 14,
	MovingRight = 15,
	Speaking = 16,

	None = 0
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
	/*
	 * Essa implementação do sistema de audio permite que mais de uma
	 * instância da classe Agent possa ser feita sem eventuais problemas.
	 */
	static bool AudioInitialized;

	static std::map<uint, AudioInfo> AudioData;
	static byte UsedChannels;

	void PlayAudio(uint index);
	static void AudioFinishedCallback(int channel);
	// ------------

	// Fala 
	BalloonWnd Balloon;
	// ------------

	// Animação
	AnimationInfo CurrentAnimation;
	uint Frame = 0;
	uint LastFrame = 0;

	std::vector<SDL_Surface*> FrameLayers = {};
	std::vector<SDL_Surface*> FrameOverlays = {};

	uint Interval = 0;

	bool AnimPaused = false;
	bool StopRequested = false;

	void LoadAnimation(string name);

	void UpdateAnim();

	void AdvanceFrame(std::vector<BranchInfo> branches);
	void PrepareFrame(int index);

	void LoadAnimationFromState(States state);

	FrameInfo* GetFrame(uint index);
	bool CanSpeakOnFrame(uint frame);
	// ------------

	// ThreadMain
	void ThreadMain();
	// ------------

	// Fila
	States CurrentState;
	Request CurrentRequest = {};

	std::queue<Request> RequestQueue;
	void Queue(Request req);

	void QueueLogic();
	// ------------
public:
	Agent(AgentFile* agf);

	AgentFile* AgFile;

	void DoStuff();


};

