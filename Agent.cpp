#include "Agent.h"

Agent::Agent(AgentFile* agf)
{
	AgFile = agf;

	CurrentState = AgentState::Showing;

	Window = nullptr;
	Renderer = nullptr;

	Frame = 0;
	Interval = 0;
	LastFrame = 0;
	Shown = true;
	StopRequested = false;
}

void Agent::DoStuff()
{
	ThreadMain();
	return;

	AgentThread = std::thread(&Agent::ThreadMain, this);
	AgentThread.detach();
}

bool Agent::AudioInitialized = false;

void Agent::SetupWindow()
{
	LocalizedInfo* loc = AgFile->GetLocalizedInfo(0x416); // pt-BR

	if (SDL_WasInit(SDL_INIT_EVERYTHING)) 
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		IMG_Init(SDL_INIT_EVERYTHING);
		TTF_Init();
	}

	Window = SDL_CreateWindow(
		"Agent",
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		AgFile->CharInfo.Width, 
		AgFile->CharInfo.Height, 
		SDL_WINDOW_ALWAYS_ON_TOP |
		SDL_WINDOW_BORDERLESS
	);

	Renderer = SDL_CreateRenderer(
		Window,
		-1,
		SDL_RENDERER_PRESENTVSYNC | 
		SDL_RENDERER_ACCELERATED
	);

	// FIXME: topmost ainda não foi consertado.
	// TODO: tonar isso multi plataforma
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(Window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	SetWindowLong(
		hwnd,
		GWL_STYLE,
		(GetWindowLong(hwnd, GWL_STYLE) | WS_POPUP) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
		);

	// Janela que não rouba o foco, com suporte para transparência e não sai da tela
	SetWindowLong(
		hwnd,
		GWL_EXSTYLE,
		WS_EX_LAYERED
	);

	SetForegroundWindow(hwnd);
	SetActiveWindow(hwnd);

	SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

	SetLayeredWindowAttributes(hwnd, 0x00FF00FF, 0xff, LWA_COLORKEY);

	SetWindowText(hwnd, loc->CharName.c_str());
	// ------------------------------

	SDL_SetWindowIcon(Window, AgFile->AgentTrayIcon);

	if (!AudioInitialized) 
	{
		AudioInitialized = true;

		Mix_ChannelFinished(AudioFinishedCallback);
		Mix_OpenAudio(11025, AUDIO_U8, 1, 64);
	}

	Balloon.Setup(&AgFile->CharInfo.BalloonInfo);

	SDL_SetWindowHitTest(Window, HitTestCallback, nullptr);
}

void Agent::WndLoop()
{
	unsigned long long lastAnimUpdate = SDL_GetTicks64();
	unsigned long long lastRedraw = SDL_GetTicks64();

	//TODO: remover isso
	Balloon.Show();
	Balloon.UpdateText(L"Ainda, que eu falasse a língua dos homens, ou falasse a língua dos anjos, sem amor nada eu seria.");

	SDL_Event e;
	while (true) 
	{
		SDL_PumpEvents();
		while (SDL_PollEvent(&e)); // TODO: encerrar o agente e liberar o objeto.

		if (SDL_GetTicks64() - lastAnimUpdate >= Interval * 10)
		{
			lastAnimUpdate = SDL_GetTicks64();

			UpdateAnim();
			PrepareFrame(Frame);

			if (GetFrame(Frame)->AudioIndex != -1)
				PlayAudio(GetFrame(Frame)->AudioIndex);
		}

		if (SDL_GetTicks64() - lastRedraw < 1000 / 60)
			continue;

		lastRedraw = SDL_GetTicks64();

		Render();

		Balloon.AttachToWindow(Window);
		Balloon.Render();
	}
}

void Agent::UpdateAnim()
{
	FrameInfo* currentFrame = GetFrame(Frame);

	switch (AnimState) 
	{
	case AnimationState::Progressing:
		LastFrame = Frame;

		AdvanceFrame(currentFrame->Branches);
		Interval = currentFrame->FrameDuration;

		if (Frame == CurrentAnimation.Frames.size() - 1 || Frame < 0)
			AnimState = AnimationState::Finished;

		if (GetFrame(Frame)->FrameDuration == 0) 
			UpdateAnim();
		break;
	case AnimationState::Finished:
		Frame = LastFrame;

		AnimState = AnimationState::IdlePose;

		if (CanSpeakOnFrame(LastFrame))
			AnimState = AnimationState::SpeakReady;
		break;
	case AnimationState::Returning:
		switch (CurrentAnimation.Transition)
		{
		case TransitionType::ExitBranches:
			LastFrame = Frame;
			Frame = currentFrame->ExitFrameIndex;

			if (Frame > 0)
				break;

			if (Frame == -1) 
			{
				Frame++;
				break;
			}

			// frame == -2

			Frame = LastFrame;

			AnimState = AnimationState::IdlePose;
			break;
		case TransitionType::ReturnAnimation:
			LoadAnimation(CurrentAnimation.ReturnAnimation);
			break;
		case TransitionType::None:
			AnimState = AnimationState::IdlePose;
			break;
		}
		break;
	}
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

void Agent::PrepareFrame(int index)
{
	if (AnimState == AnimationState::Finished)
		return;

	FrameInfo* fi = GetFrame(index);

	for (auto& surface : FrameLayers)
		SDL_FreeSurface(surface);

	for (auto& surface : FrameOverlays) 
		SDL_FreeSurface(surface);

	FrameLayers.clear();
	FrameOverlays.clear();

	for (auto& imageInfo : fi->Images)
		FrameLayers.push_back(AgFile->ReadImage(imageInfo.FrameIndex));

	for (auto& overlayInfo : fi->Overlays)
		FrameOverlays.push_back(AgFile->ReadImage(overlayInfo.ImageIndex));
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

void Agent::LoadAnimationFromState(AgentState state)
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

	string animName = stateInfo->Animations[stateInfo->Animations.size()];

	LoadAnimation(animName);
}

FrameInfo* Agent::GetFrame(uint index)
{
	return &CurrentAnimation.Frames[index];
}

bool Agent::CanSpeakOnFrame(uint frame)
{
	return GetFrame(frame)->Overlays.size();
}

void Agent::ThreadMain()
{
	SetupWindow();
	LoadAnimation(L"Idle2_1");

	WndLoop();
}

void Agent::Render()
{
	// TODO: renderizar overlays

	FrameInfo* fi = &CurrentAnimation.Frames[Frame];

	SDL_Rect targetRect;

	SDL_SetRenderDrawColor(Renderer, 255, 0, 255, 255);
	SDL_RenderClear(Renderer);

	std::vector<SDL_Texture*> textures = {};

	for (int i = (int)fi->Images.size() - 1; i >= 0; i--)
	{
		FrameImage* fImg = &fi->Images[i];

		SDL_Surface* surImg = FrameLayers[i];
		SDL_Texture* tex = SDL_CreateTextureFromSurface(Renderer, surImg);

		targetRect = { fImg->OffsetX, fImg->OffsetY, surImg->w, surImg->h };

		textures.push_back(tex);

		SDL_RenderCopyEx(
			Renderer, 
			tex, 
			NULL, 
			&targetRect, 
			0, 
			nullptr, 
			SDL_FLIP_VERTICAL
		);
	}

	for (auto& texture : textures)
		SDL_DestroyTexture(texture);

	SDL_RenderPresent(Renderer);
}

void Agent::Queue(Request req)
{
	RequestQueue.push(req);
}

void Agent::AnimationEndLogic()
{
	
}

void Agent::ShowWindow()
{
	SDL_ShowWindow(Window);
	Shown = true;
}

void Agent::HideWindow()
{
	SDL_HideWindow(Window);
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

	if (UsedChannels >= availableChannels)
		return;

	AudioInfo ai = AgFile->ReadAudio(index);

	SDL_RWops* rw = SDL_RWFromMem(ai.Buffer, ai.Size);
	Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1);

	ai.RW = rw;
	ai.Chunk = chunk;

	int channel = Mix_PlayChannel(-1, chunk, 0);
	AudioData.insert({ channel, ai });

	UsedChannels++;
}

void Agent::AudioFinishedCallback(int channel)
{
	AudioInfo* chanAi = &AudioData[channel];

	/*
	* "Only use SDL_FreeRW() on pointers returned by SDL_AllocRW(). The pointer is invalid as 
	* soon as this function returns. Any extra memory allocated during creation of the SDL_RWops 
	* is not freed by SDL_FreeRW(); the programmer must be responsible for managing that memory in 
	* their close method."
	* 
	* eis aí o motivo do antigo crash.
	* 
	* ou pode ser o freesrc na criação do mixchunk
	* 
	* foda-se, funciona agora e não vaza memória
	* big hugs
	*/
	//SDL_FreeRW(chanAi->RW);

	Mix_FreeChunk(chanAi->Chunk);

	free(chanAi->Buffer);

	AudioData.erase(channel);
	UsedChannels--;
}