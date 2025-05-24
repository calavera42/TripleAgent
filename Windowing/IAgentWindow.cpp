#include "include/AGXWin.h"
#include "src/AgentWindow.h"

AGENT_WIN IAgentWindow* CreateAgentWindow()
{
    return new AgentWindow();
}

AGENT_WIN void DeleteAgentWindow(IAgentWindow* win)
{
    delete win;
}
