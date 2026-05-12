#include "include/agxdpv.h"
#include "src/agent_file.h"

AGENT_DPV IAgentFile* CreateAgentFile()
{
	return new AgentFile;
}

AGENT_DPV void DestroyAgentFile(IAgentFile* agent)
{
	delete agent;
}
