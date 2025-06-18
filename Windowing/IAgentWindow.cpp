#include "include/AGXWin.h"

#include "src/Agent/AgentWindow.h"
#include "src/Balloon/BalloonWindow.h"

AGENT_WIN IAgentWindow* CreateAgentWindow()
{
    return new AgentWindow();
}

AGENT_WIN void DeleteAgentWindow(IAgentWindow* win)
{
    delete win;
}

AGENT_WIN IBalloonWindow* CreateBalloonWindow()
{
    return new BalloonWindow();
}

AGENT_WIN void DeleteBalloonWindow(IBalloonWindow* win)
{
    delete win;
}
