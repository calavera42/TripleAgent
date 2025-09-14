#include "agent.h"

IRequest& Agent::Show()
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::Hide()
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::MoveTo(int x, int y)
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::GestureAt(int x, int y)
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::Speak(std::wstring text)
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::Play(std::wstring animationName)
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::Stop()
{
    // TODO: inserir instrução return aqui
}

IRequest& Agent::Stop(IRequest& request)
{
    // TODO: inserir instrução return aqui
}

void Agent::CompletionSink(void(*callfunc)(IRequest& r))
{
}

void Agent::CancellationSink(void(*callfunc)(IRequest& r))
{
}
