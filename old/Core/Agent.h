#pragma once

#include "AgentServices/AnimationProvider.h"
#include "AgentServices/Window.h"

class Agent
{
private:
	AgentFile AgFile;

	AgWindow AgentWindow;
	AnimationProvider AnimProvider;

	void Loop();
	void AnimationEnd();
public:
	Agent(std::string path)
	{
		AgFile = {};
		AgFile.Load(path);
	}

	void DoStuff();
};

