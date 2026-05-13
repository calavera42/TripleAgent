#include "include/agxdpv.h"
#include "src/agent_file.h"

AGENT_DPV std::shared_ptr<IAgentFile> CreateAgentFile()
{
	return std::make_shared<AgentFile>();
}
