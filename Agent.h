#pragma once

#include "ACSFormat/AgentFile.h"
#include "BalloonWnd.h"

#include <thread>
#include <chrono>
#include <queue>

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

	uint Interval = 0;

	bool AnimPaused = false;
	bool StopRequested = false;

	void UpdateAnim();
	void AdvanceFrame(std::vector<BranchInfo> branches);
	void LoadAnimation(string name);

	FrameInfo* GetFrame(uint index);
	// ------------

	// ThreadMain
	void ThreadMain();
	// ------------

	// Fila
	std::queue<Request> RequestQueue;

	void QueueLogic();
	// ------------
public:
	Agent(AgentFile* agf);

	AgentFile* AgFile;

	void DoStuff();
};

