#include "../include/trplagnt.h"

#include "agent.h"

AGENT_CORE IAgent* CreateAgent()
{
    return new Agent();
}

AGENT_CORE void DestroyAgent(IAgent* agent)
{
    delete agent;
}
