#pragma once

#include <string>
#include <functional>

#ifdef CORE_EXPORTS
	#define AGENT_CORE __declspec(dllexport) 
#else
	#define AGENT_CORE __declspec(dllimport) 
#endif

class Request
{
public:
	virtual int GetID() = 0;
	virtual void Wait() = 0;

	virtual void OnComplete(std::function<void()> callback) = 0;
};

class IAgent
{
public:
	virtual Request* Show() = 0;
	virtual Request* Hide() = 0;
	virtual Request* MoveTo(int x, int y) = 0;
	virtual Request* GestureAt(int x, int y) = 0;
	virtual Request* Speak(std::wstring text) = 0;
	virtual Request* Play(std::wstring animationName) = 0;
	virtual Request* Stop() = 0;
	virtual Request* Stop(Request& request) = 0;
	virtual void CompletionSink(std::function<void(Request&)> r) = 0;
	virtual void CancellationSink(std::function<void(Request&)> r) = 0;
};

extern "C" AGENT_CORE IAgent* CreateAgent();
extern "C" AGENT_CORE void DestroyAgent(IAgent* agent);