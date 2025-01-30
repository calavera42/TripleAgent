#include "Agent.h"

Agent::Agent(AgentFile* agf)
{
	AgFile = agf;

	AgentWindow = nullptr;
	Renderer = nullptr;

	Frame = 0;
	Interval = 0;
	LastFrame = 0;
	Shown = true;
	StopRequested = false;
}

void Agent::DoStuff()
{
	AgentThread = std::thread(&Agent::ThreadMain, this);
	AgentThread.detach();
}

bool Agent::AudioInitialized = false;

void Agent::SetupWindow()
{
	LocalizedInfo* loc = AgFile->GetLocalizedInfo(0x416); // pt-BR

	if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) 
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		IMG_Init(SDL_INIT_EVERYTHING);
	}
	
	AgentWindow = SDL_CreateWindow(
		"Agent",
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		AgFile->CharInfo.Width, 
		AgFile->CharInfo.Height, 
		SDL_WINDOW_ALWAYS_ON_TOP | 
		SDL_WINDOW_BORDERLESS
	);

	Renderer = SDL_CreateRenderer(
		AgentWindow,
		-1,
		SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
	);

	// TODO: tonar isso multi plataforma
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(AgentWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	/*SetWindowLong(
		hwnd,
		GWL_STYLE,
		GetWindowLong(hwnd, GWL_STYLE) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
		);*/

	// Janela que não rouba o foco, com suporte para transparência e não sai da tela
	SetWindowLong(
		hwnd,
		GWL_EXSTYLE,
		WS_EX_NOACTIVATE |
		WS_EX_LAYERED |
		WS_EX_TOPMOST
	);

	SetLayeredWindowAttributes(hwnd, 0x00FF00FF, 0xff, 1);
	// ------------------------------

	SDL_SetWindowIcon(AgentWindow, AgFile->AgentTrayIcon);

	if (!AudioInitialized) 
	{
		AudioInitialized = true;

		Mix_ChannelFinished(AudioFinishedCallback);
		Mix_OpenAudio(22050, AUDIO_S8, 1, 1024);
	}

	SDL_SetWindowHitTest(AgentWindow, HitTestCallback, nullptr);

	Balloon.Setup(&AgFile->CharInfo.BalloonInfo);
}

void Agent::WndLoop()
{
	auto p1 = std::chrono::system_clock::now();

	Balloon.Show();
	Balloon.UpdateText(L"Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 Teste balão 123 ");

	while (true) 
	{
		SDL_PumpEvents();

		std::chrono::milliseconds dt = std::chrono::duration_cast<std::chrono::milliseconds>
			(
				(std::chrono::system_clock::now() - p1)
			);

		if ((uint)dt.count() >= Interval * 9)
		{
			p1 = std::chrono::system_clock::now();

			UpdateAnim();

		}

		Balloon.AttachToWindow(AgentWindow);

		Render();
		Balloon.Update();
	}
}

void Agent::UpdateAnim()
{
	if (AnimPaused || !Shown)
		return;

	FrameInfo* currentFrame = GetFrame(Frame);

	if (currentFrame->AudioIndex != -1)
		PlayAudio(currentFrame->AudioIndex);

	AdvanceFrame(currentFrame->Branches);

	if (Frame < CurrentAnimation.Frames.size() - 1 && !StopRequested)
	{
		if (currentFrame->FrameDuration == 0)
			UpdateAnim();
		Interval = currentFrame->FrameDuration;
		return;
	}

	switch (CurrentAnimation.Transition) 
	{
	case TransitionType::ReturnAnimation:
		LoadAnimation(CurrentAnimation.ReturnAnimation);
		break;
	case TransitionType::ExitBranches:
		StopRequested = true;

		LastFrame = Frame;
		Frame = currentFrame->ExitFrameIndex;

		if (Frame == -1) 
		{
			Frame++;
			break;
		}
		if (Frame == -2)
			Frame = LastFrame;
	case TransitionType::None:
		//TODO: avançar fila
		AnimPaused = true;
		break;
	}

	currentFrame = GetFrame(Frame);

	if (currentFrame->FrameDuration == 0)
		UpdateAnim();

	Interval = currentFrame->FrameDuration;
}

void Agent::AdvanceFrame(std::vector<BranchInfo> branches)
{
	if (StopRequested)
		return;

	int rnd = rand() % 101;
	int prob = 0;

	for (auto& brInfo : branches) {
		prob += brInfo.Probability;

		if (rnd <= prob) {
			Frame = brInfo.TargetFrame;
			return;
		}
	}

	Frame++;
}

void Agent::LoadAnimation(string name)
{
	CurrentAnimation = AgFile->ReadAnimation(name);
	Frame = 0;
	LastFrame = -1;

	Interval = 0;

	AnimPaused = 0;
	StopRequested = 0;
}

FrameInfo* Agent::GetFrame(uint index)
{
	return &CurrentAnimation.Frames[index];
}

void Agent::ThreadMain()
{
	SetupWindow();
	LoadAnimation(L"Searching");

	WndLoop();
}

void Agent::Render()
{
	FrameInfo* fi = &CurrentAnimation.Frames[Frame];

	SDL_Rect srcRect = { 0, 0, AgFile->CharInfo.Width, AgFile->CharInfo.Height };
	SDL_Rect targetRect;

	SDL_SetRenderDrawColor(Renderer, 255, 0, 255, 255);
	SDL_RenderClear(Renderer);

	std::vector<SDL_Texture*> textures = {};

	for (int i = (int)fi->Images.size() - 1; i >= 0; i--)
	{
		FrameImage* fImg = &fi->Images[i];

		SDL_Surface* surImg = AgFile->ReadImage(fImg->FrameIndex);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(Renderer, surImg);

		targetRect = { fImg->OffsetX, fImg->OffsetY, srcRect.w, srcRect.h };

		textures.push_back(tex);

		SDL_RenderCopyEx(Renderer, tex, &srcRect, &targetRect, 0, nullptr, SDL_FLIP_VERTICAL);
	}

	for (auto& texture : textures)
		SDL_DestroyTexture(texture);

	SDL_RenderPresent(Renderer);
}

void Agent::ShowWindow()
{
	SDL_ShowWindow(AgentWindow);
	Shown = true;
}

void Agent::HideWindow()
{
	SDL_HideWindow(AgentWindow);
	Shown = false;
}

SDL_HitTestResult Agent::HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data)
{
	return SDL_HITTEST_DRAGGABLE;
}

std::map<uint, AudioInfo> Agent::AudioData = {};
byte Agent::UsedChannels = 0;

void Agent::PlayAudio(uint index)
{
	int availableChannels = Mix_AllocateChannels(-1);

	if (!UsedChannels >= availableChannels)
		return;

	UsedChannels++;

	AudioInfo ai = AgFile->ReadAudio(index);

	SDL_RWops* rw = SDL_RWFromMem(ai.Buffer, ai.Size);

	Mix_Chunk* Song = Mix_LoadWAV_RW(rw, 1);

	int channel = Mix_PlayChannel(-1, Song, 0);
	AudioData.insert({ channel, ai });
}

void Agent::AudioFinishedCallback(int channel)
{
	free(AudioData[channel].Buffer);

	AudioData.erase(channel);
	UsedChannels--;
}