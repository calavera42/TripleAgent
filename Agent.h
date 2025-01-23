#pragma once

#include "ACSFormat/AgentFile.h"

#include <thread>
#include <chrono>

class Agent
{
private:
	bool Shown;

	std::thread AgentThread;

	// AgentWindow
	SDL_Window* AgentWindow;
	SDL_Renderer* Renderer;

	void SetupWindow();

	void WndLoop();

	void Render();

	void ShowWindow();
	void HideWindow();
	// ------------

	// Audio
	/*
	 * A implementação do sistema de áudio pode parecer
	 * desnecessáriamente complicada à primeira vista, porém,
	 * o callback do SDL_Mixer (Mixer_ChannelFinished) é estático.
	 * 
	 * Logo, essa implementação permite que mais de uma instância
	 * da classe Agent possa ser feita sem eventuais problemas.
	 */
	static bool AudioInitialized;

	static std::map<uint, AudioInfo> AudioData;
	static byte UsedChannels;

	void PlayAudio(uint index);
	static void AudioFinishedCallback(int channel);
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

	// ------------
public:
	Agent(AgentFile* agf);

	AgentFile* AgFile;

	void DoStuff();
};

