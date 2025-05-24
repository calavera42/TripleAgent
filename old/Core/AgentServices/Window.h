#pragma once

#include "../ACSFormat/AgentFile.h"
#include "../Types.h"

#include <SDL3\SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <string>
#include <chrono>
#include <queue>

class AgWindow {
private:
	AgentFile* AgFile;

	bool WindowCreated = false;

	SDL_Window* Window{};
	SDL_Renderer* Renderer{};

	SDL_Texture* AgentTex;

	bool Shown = false;

	void ShowWindow();
	void HideWindow();

	static SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data);

	static bool AudioInitialized;

	static std::map<uint, AudioData> AudioDataMap;
	static byte UsedChannels;

	static void AudioFinishedCallback(int channel);
public:
	void Setup(AgentFile* AgFile);

	// Deve ser chamado no mesmo thread que o SetupWindow()
	void Update();

	void PrepareFrame(FrameInfo* fi);
	void PlayAudio(uint index);
};