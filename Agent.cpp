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

	Balloon.Setup(&AgFile->CharInfo.BalloonInfo);

	SDL_SetWindowHitTest(Window, HitTestCallback, nullptr);

	SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

	SDL_AudioSpec spec;

	SDL_zero(spec);

	spec.freq = 11025;
	spec.format = SDL_AUDIO_S16;
	spec.channels = 1;

	AudioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
}

void Agent::WndLoop()
{
	unsigned long int lastAnimUpdate = SDL_GetTicks();
	unsigned long int lastRedraw = SDL_GetTicks();

	//TODO: remover isso
	//Balloon.Show();
	//Balloon.UpdateText(L"Ainda, que eu falasse a língua dos homens, ou falasse a língua dos anjos, sem amor nada eu seria.");

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
	LoadAnimation(L"reading");

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

void Agent::PlayAudio(uint index)
{
	SDL_Log("FIXME: sistema de áudio não funcionando.");
}