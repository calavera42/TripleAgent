#include "../DataProvider/include/AGXDpv.h"
#include "../Windowing/include/AGXWin.h"

#include <time.h>

void UserFuntion(uint32_t* buffer)
{
	for (size_t i = 0; i < 160 * 128; i++)
		buffer[i] = ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 8) | (rand() & 0xFF);
}

int main() 
{
	IAgentWindow* iaw = CreateAgentWindow();
	IAgentFile* iaf = CreateAgentFile();

	iaf->Load("d:/desktop/peedy.acs");

	srand(time(NULL));
	std::printf("%ls", iaf->GetLocalizedInfo(0x416).CharName.c_str());

	iaw->Setup(160, 128);
	iaw->RegisterUserFuntion(UserFuntion);
	iaw->StartMessageLoop();


	while (true);

	return 1;
}