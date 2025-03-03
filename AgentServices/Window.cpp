#include "Window.h"

bool AgWindow::AudioInitialized;
std::map<uint, AudioInfo> AgWindow::AudioData;
byte AgWindow::UsedChannels;

void AgWindow::PlayAudio(uint index)
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

void AgWindow::AudioFinishedCallback(int channel)
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

void AgWindow::Setup(AgentFile* agentFile)
{
	AgFile = agentFile;

	Window = SDL_CreateWindow(
		"AgentWindow",
		AgFile->CharInfo.Width,
		AgFile->CharInfo.Height,
		SDL_WINDOW_ALWAYS_ON_TOP |
		SDL_WINDOW_BORDERLESS |
		SDL_WINDOW_NOT_FOCUSABLE |
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

	SDL_SetWindowHitTest(Window, HitTestCallback, nullptr);

	SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
}

void AgWindow::PrepareFrame(FrameInfo* fi)
{
	SDL_Surface* agentSur;

	SDL_DestroyTexture(AgentTex);

	agentSur = SDL_CreateSurface(AgFile->CharInfo.Width, AgFile->CharInfo.Height, SDL_PIXELFORMAT_RGBA8888);
	SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(agentSur);

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

	SDL_SetWindowShape(Window, agentSur);

	AgentTex = SDL_CreateTextureFromSurface(Renderer, agentSur);

	SDL_DestroyRenderer(renderer);
	SDL_DestroySurface(agentSur);
}

void AgWindow::Update()
{
	SDL_RenderClear(Renderer);
	SDL_RenderTexture(Renderer, AgentTex, nullptr, nullptr);
	SDL_RenderPresent(Renderer);
}

void AgWindow::ShowWindow()
{
	SDL_ShowWindow(Window);
	Shown = true;
}

void AgWindow::HideWindow()
{
	SDL_HideWindow(Window);
	Shown = false;
}

SDL_HitTestResult AgWindow::HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data)
{
	return SDL_HITTEST_DRAGGABLE;
}