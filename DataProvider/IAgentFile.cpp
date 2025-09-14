#include "include/AGXDpv.h"
#include "src/AgentFile.h"

AGENT_DPV IAgentFile* CreateAgentFile()
{
	return new AgentFile;
}

AGENT_DPV void DestroyAgentFile(IAgentFile* agent)
{
	delete agent;
}
