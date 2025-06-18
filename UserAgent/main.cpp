#include "../DataProvider/include/AGXDpv.h"
#include "../Windowing/include/AGXWin.h"

#include <time.h>
#include <windows.h>

int main()
{
	IAgentFile* af = CreateAgentFile();
	IAgentWindow* aw = CreateAgentWindow();

	af->Load("d:/desktop/merlin.acs");

	AnimationInfo ai = af->GetAnimationInfo(L"restpose");
	FrameInfo fi = ai.Frames[0];

	aw->Setup(af);
	aw->UpdateState({ EventType::AgentVisibleChange, true });
	aw->UpdateState({ EventType::AgentFrameChange, std::make_shared<FrameInfo>(fi) });

	while (true)
	{
		aw->UpdateState({ EventType::AgentMouthChange, (MouthOverlayType)(rand() % 7) });

		Event ev;
		ev = aw->QueryInfo();

		switch (ev.Type) 
		{
			case EventType::WindowMouseRUp:
			case EventType::WindowMouseLUp:
			case EventType::WindowMouseRDown:
			case EventType::WindowMouseLDown: 
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("MOUSE: %d %d %d\n", p.X, p.Y, ev.Type);
				break;
			}
			case EventType::WindowDragStart:
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("WDS: %d %d\n", p.X, p.Y);
				break;
			}
			case EventType::WindowDragEnd:
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("WDE: %d %d\n", p.X, p.Y);
				break;
			}
		}

		Sleep(100);
	}

	while (true);

	return 1;
}