#include "agent.h"

using namespace std;

void Agent::ServeRequest(Chore* i)
{

}

void Agent::Loop()
{
    _agentWindow->Setup(_agentFile);
    _balloonWindow->Setup(_agentFile->GetCharacterInfo());

    Animation n = Animation{ 0, L"reading", _agentFile };

    _currentRequest = &n;

    auto t1 = chrono::steady_clock::now();
    auto t2 = chrono::steady_clock::now();

    int delta = 0;

    while (_running) 
    {
        delta = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

        t1 = t2;
        t2 = chrono::steady_clock::now();

        if (_currentRequest == nullptr) 
        {
            _currentRequest = &_requestQueue.front();
            _runningRequests.push(&_requestQueue.front());
        }

        int result;

        switch (_requestStage) 
        {
            case Running:
            {
                result = _currentRequest->Update(this, _agentWindow, _balloonWindow);

                
            }
            break;

            case Stopping:
            {

            }
            break;

            case Waiting:
            default:
                break;
        }

        _requestStage = _nextStage;

        ProcessMessages();
    }
}

Agent::Agent()
{
    _agentWindow = CreateAgentWindow();
    _balloonWindow = CreateBalloonWindow();

    _agentFile = CreateAgentFile();
    _agentFile->Load("d:/desktop/peedy.acs"); // TODO: checar valores de retorno

    std::thread(Loop, this).detach();
}

Agent::~Agent()
{
    DestroyAgentWindow(_agentWindow);
    DestroyBalloonWindow(_balloonWindow);

    DestroyAgentFile(_agentFile);
}

Request* Agent::Show()
{
    return nullptr;
}

Request* Agent::Hide()
{
    return nullptr;
}

Request* Agent::MoveTo(int x, int y)
{
    return nullptr;
}

Request* Agent::GestureAt(int x, int y)
{
    return nullptr;
}

Request* Agent::Speak(std::wstring text)
{
    return nullptr;
}

Request* Agent::Play(std::wstring animationName)
{
    return nullptr;
}

Request* Agent::Stop()
{
    return nullptr;
}

Request* Agent::Stop(Request& request)
{
    return nullptr;
}

void Agent::CompletionSink(std::function<void(Request&)> r)
{
}

void Agent::CancellationSink(std::function<void(Request&)> r)
{
}
