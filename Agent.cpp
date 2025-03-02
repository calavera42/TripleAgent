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

	srand(1);
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

	static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter = {};
	std::string agentName = converter.to_bytes(loc->CharName);

	Window = SDL_CreateWindow(
		agentName.c_str(),
		AgFile->CharInfo.Width, 
		AgFile->CharInfo.Height, 
		SDL_WINDOW_ALWAYS_ON_TOP |
		SDL_WINDOW_BORDERLESS |
		/*SDL_WINDOW_NOT_FOCUSABLE |*/
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

// TODO: fazer com que vários frames nulos consecutivos sejam pulados juntos e não alterem o LastFrame
void Agent::UpdateAnim()
{
	FrameInfo* currentFrame = GetFrame(Frame);

	switch (AnimState) {
	case AnimationState::Progressing:
	{
		AdvanceFrame(currentFrame->Branches);
		currentFrame = GetFrame(Frame);

		Interval = GetFrame(Frame)->FrameDuration;

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
			|| (Frame >= CurrentAnimation.Frames.size() - 1 && exitFrame == -1)
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

void Agent::AnimationEndLogic()
{
	wprintf(L"%ws\t", CurrentAnimation.AnimationName.c_str());
	switch (AnimState)
	{
	case AnimationState::Progressing:
		// TODO: falar / mover
		AnimState = AnimationState::Returning;
		wprintf(L"Fim da animação, retornando...\n");
		if (CanSpeakOnFrame(Frame))
			wprintf(L"\tum maravilhoso sucesso!\n");
		break;
	case AnimationState::Finished:
		LoadAnimationFromState(AgentState::IdlingLevel3);
		wprintf(L"Animação encerrada. Reiniciando.\n");
		break;
	}
}

void Agent::AdvanceFrame(std::vector<BranchInfo>& branches)
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

void Agent::PrepareFrame(int index)
{
	if (AnimState == AnimationState::Paused)
		return;

	FrameInfo* fi = GetFrame(index);

	SDL_DestroyTexture(AgentTex);

	AgentSur = SDL_CreateSurface(AgFile->CharInfo.Width, AgFile->CharInfo.Height, SDL_PIXELFORMAT_RGBA8888);
	SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(AgentSur);

	// TODO: renderizar overlays
	SDL_FRect targetRect;

	SDL_RenderClear(renderer);

	std::vector<SDL_Texture*> textures = {};
	std::vector<SDL_Surface*> surfaces = {};

	for (int i = (int)fi->Images.size() - 1; i >= 0; i--)
	{
		FrameImage* fImg = &fi->Images[i];

		SDL_Surface* surImg = AgFile->ReadImage(fImg->FrameIndex);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surImg);

		targetRect = { (float)fImg->OffsetX, (float)fImg->OffsetY, (float)surImg->w, (float)surImg->h };

		textures.push_back(tex);
		surfaces.push_back(surImg);

		SDL_RenderTextureRotated(
			renderer,
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

	for (auto& surface : surfaces)
		SDL_DestroySurface(surface);

	SDL_SetWindowShape(Window, AgentSur);

	AgentTex = SDL_CreateTextureFromSurface(Renderer, AgentSur);

	SDL_DestroyRenderer(renderer);
	SDL_DestroySurface(AgentSur);
}

void Agent::LoadAnimation(string name)
{
	CurrentAnimation = AgFile->ReadAnimation(name);
	Frame = 0;
	LastFrame = -1;

	Interval = 0;

	AnimPaused = false;
	StopRequested = false;
	AnimState = AnimationState::Progressing;
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

	string animName = stateInfo->Animations[rand() % (stateInfo->Animations.size() - 1)];

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
	LoadAnimation(L"show");

	WndLoop();
}

void Agent::Render()
{
	SDL_RenderClear(Renderer);
	SDL_RenderTexture(Renderer, AgentTex, nullptr, nullptr);
	SDL_RenderPresent(Renderer);
}

void Agent::Queue(Request req)
{
	RequestQueue.push(req);
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
