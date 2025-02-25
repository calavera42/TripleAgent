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
	/*
					--= IMPORTANTÍSSIMO, NÃO APAGAR =--

		É importante que o agente seja inicializado num estado seguro.

			Caso o servidor não encontre a animação que o usuário solicitou, 
		não há uma maneira segura de escolher um substituto. Assumindo que
		no mínimo o agente tenha uma animação de entrada, caso o servidor
		não encontre a IdlePose, podemos assumir que o último frame da
		animação de entrada é a IdlePose.
	*/

	ThreadMain();
	return;

	AgentThread = std::thread(&Agent::ThreadMain, this);
	AgentThread.detach();
}

bool Agent::AudioInitialized;

void Agent::SetupWindow()
{
	LocalizedInfo* loc = AgFile->GetLocalizedInfo(0x416); // pt-BR

	Window = SDL_CreateWindow(
		"Agent",
		AgFile->CharInfo.Width, 
		AgFile->CharInfo.Height, 
		SDL_WINDOW_ALWAYS_ON_TOP |
		SDL_WINDOW_BORDERLESS |
		SDL_WINDOW_TRANSPARENT
	);

	Renderer = SDL_CreateRenderer(
		Window,
		NULL
	);

	SDL_SetWindowIcon(Window, AgFile->AgentTrayIcon);

	if (!AudioInitialized)
	{
		AudioInitialized = true;

		SDL_AudioSpec spec;

		spec.freq = 11025;
		spec.format = SDL_AudioFormat::SDL_AUDIO_U8;
		spec.channels = 1;

		Mix_ChannelFinished(AudioFinishedCallback);
		Mix_OpenAudio(0, &spec);
	}

	Balloon.Setup(&AgFile->CharInfo.BalloonInfo);

	SDL_SetWindowHitTest(Window, HitTestCallback, nullptr);

	SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
}

void Agent::WndLoop()
{
	unsigned long int lastAnimUpdate = SDL_GetTicks();
	unsigned long int lastRedraw = SDL_GetTicks();

	//TODO: remover isso
	Balloon.Show();
	Balloon.UpdateText(L"Se eu perder esse trem, que sai agora às onze horas, só amanhã de manhã.");

	SDL_Event e;
	while (true) 
	{
		SDL_PumpEvents();
		while (SDL_PollEvent(&e)); // TODO: encerrar o agente e liberar o objeto.

		if (SDL_GetTicks() - lastAnimUpdate >= Interval * 10)
		{
			lastAnimUpdate = SDL_GetTicks();

			UpdateAnim();
			PrepareFrame(Frame);

			if (GetFrame(Frame)->AudioIndex != -1)
				PlayAudio(GetFrame(Frame)->AudioIndex);
		}

		if (SDL_GetTicks() - lastRedraw < 1000 / 60)
			continue;

		lastRedraw = SDL_GetTicks();

		Render();

		if (!Balloon.Shown)
			continue;

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

		if (CanSpeakOnFrame(LastFrame)) {
			AnimState = AnimationState::SpeakReady;
			break;
		}

		if (CurrentState > AgentState::IdlingLevel3 && CurrentState < AgentState::Speaking)
			AnimState = AnimationState::MoveReady;
		break;
	case AnimationState::Returning:
		switch (CurrentAnimation.Transition)
		{
		case TransitionType::ExitBranches:
			LastFrame = Frame;
			Frame = currentFrame->ExitFrameIndex;

			if (Frame >= 0)
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

void Agent::AdvanceFrame(std::vector<BranchInfo>& branches)
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
		SDL_DestroySurface(surface);

	for (auto& surface : FrameOverlays) 
		SDL_DestroySurface(surface);

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
	LoadAnimation(L"searching");

	WndLoop();
}

void Agent::Render()
{
	// TODO: renderizar overlays
	FrameInfo* fi = &CurrentAnimation.Frames[Frame];

	SDL_FRect targetRect;

	SDL_RenderClear(Renderer);

	std::vector<SDL_Texture*> textures = {};

	for (int i = (int)fi->Images.size() - 1; i >= 0; i--)
	{
		FrameImage* fImg = &fi->Images[i];

		SDL_Surface* surImg = FrameLayers[i];
		SDL_Texture* tex = SDL_CreateTextureFromSurface(Renderer, surImg);

		targetRect = { (float)fImg->OffsetX, (float)fImg->OffsetY, (float)surImg->w, (float)surImg->h };

		textures.push_back(tex);

		SDL_RenderTextureRotated(
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

	SDL_IOStream* rw = SDL_IOFromMem(ai.Buffer, ai.Size);
	Mix_Chunk* chunk = Mix_LoadWAV_IO(rw, 1);

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

	Mix_FreeChunk((Mix_Chunk*)chanAi->Chunk);

	free(chanAi->Buffer);

	AudioData.erase(channel);
	UsedChannels--;
}
