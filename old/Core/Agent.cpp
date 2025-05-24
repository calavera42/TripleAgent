#include "Agent.h"

void Agent::Loop()
{
	unsigned long int lastAnimUpdate = SDL_GetTicks();
	unsigned long int lastRedraw = SDL_GetTicks();

	float delay = 1000.f / 30.f;

	SDL_Event ev;

	while (true) 
	{
		while (SDL_PollEvent(&ev));

		if (SDL_GetTicks() - lastAnimUpdate >= AnimProvider.GetInterval() * 10)
		{
			FrameInfo* curFrame = AnimProvider.GetCurrentFrame();

			lastAnimUpdate = SDL_GetTicks();

			if (curFrame->AudioIndex != -1)
				AgentWindow.PlayAudio(curFrame->AudioIndex);

			AnimProvider.Update();
			AgentWindow.PrepareFrame(curFrame);
		}

		if (SDL_GetTicks() - lastRedraw >= delay)
			AgentWindow.Update();
	}
}

void Agent::AnimationEnd()
{
	//AnimProvider.LoadAnimation(L"doMagic1");
	
	AnimProvider.SetAnimationState(AnimationState::Returning);
	printf("Fim da animação\n");
}

void Agent::DoStuff()
{
	AgentWindow.Setup(&AgFile);
	AnimProvider.Setup(
		&AgFile, 
		std::bind(&Agent::AnimationEnd, this)
	);

	AnimProvider.LoadAnimation(L"doMagic1");

	Loop();
}
