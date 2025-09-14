#pragma once

#include <string>

#ifdef CORE_EXPORTS
	#define AGENT_CORE __declspec(dllexport) 
#else
	#define AGENT_CORE __declspec(dllimport) 
#endif

class IRequest
{
public:
	virtual int GetID() = 0;
	virtual void Wait() = 0;

	virtual void OnComplete(void (*callfunc)()) = 0;
	virtual void OnCancel(void (*callfunc)()) = 0;
};

class IAgent 
{
public:
	virtual IRequest& Show() = 0;
	virtual IRequest& Hide() = 0;
	virtual IRequest& MoveTo(int x, int y) = 0;
	virtual IRequest& GestureAt(int x, int y) = 0;
	virtual IRequest& Speak(std::wstring text) = 0;
	virtual IRequest& Play(std::wstring animationName) = 0;
	virtual IRequest& Stop() = 0;
	virtual IRequest& Stop(IRequest& request) = 0;
	virtual void CompletionSink(void (*callfunc)(IRequest& r)) = 0;
	virtual void CancellationSink(void (*callfunc)(IRequest& r)) = 0;
};

extern "C" AGENT_CORE IAgent* CreateAgent();
extern "C" AGENT_CORE void DestroyAgent(IAgent* agent);