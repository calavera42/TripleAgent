#pragma once

#include "../include/trplagnt.h"
#include "requestman.h"

#include <queue>

class Agent : public IAgent
{
private:
	RequestManager _requestMan = {};

public:
	IRequest& Show() override;
	IRequest& Hide() override;
	IRequest& MoveTo(int x, int y) override;
	IRequest& GestureAt(int x, int y) override;
	IRequest& Speak(std::wstring text) override;
	IRequest& Play(std::wstring animationName) override;
	IRequest& Stop() override;
	IRequest& Stop(IRequest& request) override;
	void CompletionSink(void(*callfunc)(IRequest& r)) override;
	void CancellationSink(void(*callfunc)(IRequest& r)) override;
};

