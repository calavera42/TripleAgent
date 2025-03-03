#include "ACSFormat/AgentFile.h"
#include "Agent.h"
#include <fstream>

int main(int argc, char** argv) {
	Agent ag = Agent("c:/users/gregd/desktop/peedy.acs");

	ag.DoStuff();

	while (true);

	return 1;
}