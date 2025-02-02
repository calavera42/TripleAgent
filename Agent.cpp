#include "Agent.h"

Agent::Agent(AgentFile* agf)
{
	AgFile = agf;

	CurrentState = States::Showing;

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
		GetWindowLong(hwnd, GWL_EXSTYLE) |
		WS_EX_TOPMOST |
		WS_EX_NOACTIVATE |
		WS_EX_LAYERED |
		WS_EX_MDICHILD
	);

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
	unsigned long lastAnimUpdate = SDL_GetTicks64();
	unsigned long lastRedraw = SDL_GetTicks64();

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
		}

		if (SDL_GetTicks64() - lastRedraw < 1000 / 60)
			continue;

		lastRedraw = SDL_GetTicks64();

		Balloon.AttachToWindow(Window);
		Render();
		Balloon.Render();
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

void Agent::PrepareFrame(int index)
{
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

void Agent::LoadAnimationFromState(States state)
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
	LoadAnimation(L"Reading");

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

void Agent::QueueLogic()
{
	StopRequested = false;

	if (RequestQueue.empty())
	{
		// TODO: idle logic
		return;
	}

	CurrentState = States::None;
	CurrentRequest = RequestQueue.front();

	switch (CurrentRequest.Type) {
	case RequestType::Speak:
		CurrentState = States::Speaking;

		if (!CanSpeakOnFrame(Frame))
			LoadAnimationFromState(CurrentState);

		string text = *(string*)CurrentRequest.Data;
		free(CurrentRequest.Data); // IMPORTANTÍSSIMO (se apagar vc com certeza vai esquecer)


		break;
	}
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

	SDL_FreeRW(chanAi->RW);
	Mix_FreeChunk(chanAi->Chunk);

	free(chanAi->Buffer);

	AudioData.erase(channel);
	UsedChannels--;
}