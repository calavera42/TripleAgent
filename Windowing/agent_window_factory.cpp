#include "include/AGXWin.h"

#include "src/Agent/AgentWindow.h"
#include "src/Balloon/BalloonWindow.h"

#include <windows.h>

AGENT_WIN std::shared_ptr<IAgentWindow> CreateAgentWindow()
{
    return std::make_shared<AgentWindow>();
}

AGENT_WIN std::shared_ptr<IBalloonWindow> CreateBalloonWindow()
{
    return std::make_shared<BalloonWindow>();
}