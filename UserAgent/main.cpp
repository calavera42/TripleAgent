#include "../DataProvider/include/AGXDpv.h"
#include "../Windowing/include/AGXWin.h"

int main() 
{
	IAgentWindow* aw = CreateAgentWindow();
	IAgentFile* af = CreateAgentFile();

	af->Load("d:/desktop/peedy.acs");

	AnimationInfo ai = af->GetAnimationInfo(L"restpose");
	FrameInfo fi = ai.Frames[0];

	UpdateInfo ui{};

	ui.Type = UpdateType::VisibleChange;
	ui.WindowVisible = true;

	aw->Setup(af);
	aw->UpdateState(ui);
	aw->UpdateState({UpdateType::FrameChange, &fi});

	while (true);

	return 1;
}