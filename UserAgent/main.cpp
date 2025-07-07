#include <agxdpv.h>
#include "../Windowing/include/AGXWin.h"

#include <time.h>
#include <windows.h>

int main()
{
	IAgentFile* af = CreateAgentFile();

	IAgentWindow* aw = CreateAgentWindow();
	IBalloonWindow* bw = CreateBalloonWindow();

	af->Load("d:/desktop/peedy.acs");

	AnimationInfo ai = af->GetAnimationInfo(L"explain");
	CharacterInfo ci = af->GetCharacterInfo();

	TrayIcon ti = af->GetAgentIcon();

	FrameInfo fi = ai.Frames[8];

	bw->Setup(ci);

	aw->Setup(af);
	aw->UpdateState({ EventType::AgentVisibleChange, true });

	AgPoint ap = aw->GetPos();
	AgPoint as = aw->GetSize();
	int i = 0;

	while (true)
	{
		//aw->UpdateState({ EventType::AgentMouthChange, (MouthOverlayType)(rand() % 7) });
		aw->UpdateState({ EventType::AgentFrameChange, std::make_shared<FrameInfo>(ai.Frames[i++ % 9])});

		Event ev;
		ev = aw->QueryEvent();

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
				bw->UpdateState({ EventType::AgentBalloonHide });
				break;
			}
			case EventType::WindowDragEnd:
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("WDE: %d %d\n", p.X, p.Y);
				bw->UpdateState({ EventType::AgentBalloonShow, L"Ah eu não aguento mais\nFicar longe do teu amor\nDessa voz doce, quente, tão rouca\nE do teu corpo sedutor\n\nAh eu não aguento mais\nFicar longe de ti, meu bem\nDos teus beijos na boca, tão louca\nNinguém beija melhor, meu bem\nComo eu te adoro, amor\n\nE não cabe mais em mim\nDesejo te amar, meu bem\nTe abraçar\nTe apertar\nTe beijar\nDepois ir muito mais além\n\nViver não me interessa mais\nSe acaso um dia eu te perder\nVou chorar\nVou sofrer\nMe matar\nNão me interessa mais viver\nSem teu amor meu bem" });
				bw->UpdateState({ EventType::AgentBalloonAttach, AgRect{ p.X, p.Y, p.X + ci.Width, p.Y + ci.Height } });
				break;
			}
		}

		Sleep(99);
	}

	while (true);

	return 1;
}