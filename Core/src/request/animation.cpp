#include "Animation.h"

FramePointer Animation::GetFrame(int frame)
{
    return std::unique_ptr<FrameInfo>(&_animation.Frames[frame]);
}

void Animation::DoFrameProceed(std::vector<BranchInfo>& branches)
{
    int selection = (rand() % 100) + 1;
    int count = 0;
    for (size_t i = 0; i < branches.size(); i++)
    {
        count += branches[i].Probability;
        if (selection <= count)
        {
            _currentFrame = branches[i].TargetFrame;
            return;
        }
    }

    _currentFrame++;
}

Animation::Animation(int id, string animName, IAgentFile* agentFile)
{
    srand(10);

    _id = id;
    _animation = agentFile->GetAnimationInfo(animName);

    _lastValidFrame = 0;
    _currentFrame = 0;

    _animationLength = _animation.Frames.size();
}

int Animation::Update(Agent* current, IAgentWindow* iaw, IBalloonWindow* ibw)
{
    FramePointer fp = GetFrame(_currentFrame);

    if (fp->FrameDuration != 0) 
    {
        _lastValidFrame = _currentFrame;
        iaw->UpdateState({ AgEventType::AgentFrameChange, fp });
    }

    DoFrameProceed(fp->Branches);

    if (_currentFrame == _animationLength)
    {
        _currentFrame = _lastValidFrame;
        return REQUEST_DONE;
    }

    return fp->FrameDuration;
}

int Animation::Return(Agent* current, IAgentWindow* iaw, IBalloonWindow* ibw)
{
    FramePointer fp = GetFrame(_currentFrame);

    if (fp->FrameDuration != 0)
    {
        _lastValidFrame = _currentFrame;
        iaw->UpdateState({ AgEventType::AgentFrameChange, fp });
    }

    int exitFrame = fp->ExitFrameIndex;

    if (exitFrame == -2) 
    {
        iaw->UpdateState({ AgEventType::AgentFrameChange, GetFrame(_lastValidFrame) });
        return REQUEST_DONE;
    }

    if (exitFrame == -1)
        _currentFrame++;
    else
        _currentFrame = exitFrame;

    return fp->FrameDuration;
}

Type Animation::GetType()
{
    return Type::Animate;
}
