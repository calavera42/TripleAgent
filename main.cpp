#include "ACSFormat/AgentFile.h"
#include "Agent.h"
#include "SDL.h"
#include <fstream>

int main(int argc, char** argv) {
	AgentFile af = {};
	af.Load("c:/users/gregd/desktop/merlin.acs");

	Agent ag = Agent(&af);

	ag.DoStuff();

	while (true);

	return 1;
}