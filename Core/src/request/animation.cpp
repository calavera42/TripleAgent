#include "Animation.h"

UpdateResult Animation::Update(IAgentWindow* iaw, IBalloonWindow* ibw)
{
    return UpdateResult();
}

UpdateResult Animation::Stop(IAgentWindow* iaw, IBalloonWindow* ibw)
{
    return UpdateResult();
}

int Animation::GetID()
{
    return 0;
}

void Animation::Wait()
{
}

void Animation::OnComplete(void(*callfunc)())
{
}

void Animation::OnCancel(void(*callfunc)())
{
}
