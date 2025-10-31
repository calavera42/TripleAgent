#pragma once

#include "chore.h"

#include "request/animation.h"

#include <queue>
#include <stack>
#include <thread>
#include <chrono>

class Agent : public IAgent
{
private:
	enum Stage 
	{
		Running,
		Stopping,
		Waiting
	};

	friend Chore;

	IAgentFile* _agentFile;
	IAgentWindow* _agentWindow;
	IBalloonWindow* _balloonWindow;

	std::queue<Chore> _requestQueue{};
	std::stack<Chore*> _runningRequests{};

	Chore* _currentRequest = nullptr;

	Stage _requestStage = Running;
	Stage _nextStage = Running;

	bool _running = true;
	
	void ServeRequest(Chore* i);
	void Loop();

public:
	Agent();
	Agent(const Agent&) = delete;
	~Agent();

	Request* Show() override;
	Request* Hide() override;
	Request* MoveTo(int x, int y) override;
	Request* GestureAt(int x, int y) override;
	Request* Speak(std::wstring text) override;
	Request* Play(std::wstring animationName) override;
	Request* Stop() override;
	Request* Stop(Request& request) override;

	void CompletionSink(std::function<void(Request&)> r) override;
	void CancellationSink(std::function<void(Request&)> r) override;
};
