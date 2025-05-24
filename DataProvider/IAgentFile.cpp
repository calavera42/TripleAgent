#include "include/AGXDpv.h"
#include "src/AgentFile.h"

AGENT_DPV IAgentFile* CreateAgentFile()
{
	return new AgentFile();
}

AGENT_DPV void DeleteAgentFile(IAgentFile* agent)
{
	delete agent;
}
