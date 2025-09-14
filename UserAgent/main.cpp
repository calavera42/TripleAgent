#include <agxdpv.h>
#include "../Windowing/include/AGXWin.h"

#include <time.h>
#include <windows.h>
#include <chrono>

#include <thread>

#include "../Markdown/include/AgentMarkdown.h"

void loop(std::string map);

int main()
{
	Lexer l{};

	l.ParseString(L"acredite ou não eu simplesmente estou esgotado");

	return 1;
}

void loop(std::string map) {
	IAgentFile* af = CreateAgentFile();

	IAgentWindow* aw = CreateAgentWindow();
	IBalloonWindow* bw = CreateBalloonWindow();

	af->Load(map);

	AnimationInfo ai = af->GetAnimationInfo(L"search");
	CharacterInfo ci = af->GetCharacterInfo();

	FrameInfo fi = ai.Frames[8];

	aw->Setup(af);
	bw->Setup(ci);

	int i = 0, i2 = 0, delay = 0;

	string texts[2] = {
		L"Ah eu não aguento mais\nFicar longe do teu amor\nDessa voz doce, quente, tão rouca\nE do teu corpo sedutor\n\nAh eu não aguento mais\nFicar longe de ti, meu bem\nDos teus beijos na boca, tão louca\nNinguém beija melhor, meu bem\nComo eu te adoro, amor\n\nE não cabe mais em mim\nDesejo te amar, meu bem\nTe abraçar\nTe apertar\nTe beijar\nDepois ir muito mais além\n\nViver não me interessa mais\nSe acaso um dia eu te perder\nVou chorar\nVou sofrer\nMe matar\nNão me interessa mais viver\nSem teu amor meu bem",
		L"This is a sample line of text\nThis is a sample line of text"
	};

	auto ft = std::chrono::system_clock::now();
	auto st = std::chrono::system_clock::now();

	aw->UpdateState({ AgEventType::AgentVisibleChange, true });
	bw->UpdateState({ AgEventType::AgentBalloonAttach, std::shared_ptr<IAgentWindow>(aw) });

	while (true)
	{
		AgEvent ev;
		ev = aw->QueryEvent();

		switch (ev.Type)
		{
			case AgEventType::WindowMouseRUp:
			case AgEventType::WindowMouseLUp:
			case AgEventType::WindowMouseRDown:
			case AgEventType::WindowMouseLDown:
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("MOUSE: %d %d %d\n", p.X, p.Y, ev.Type);
				break;
			}
			case AgEventType::WindowDragStart:
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("WDS: %d %d\n", p.X, p.Y);
				bw->UpdateState({ AgEventType::AgentBalloonHide });
				break;
			}
			case AgEventType::WindowDragEnd:
			{
				AgPoint p = std::get<AgPoint>(ev.Data);
				printf("WDE: %d %d\n", p.X, p.Y);
				bw->UpdateState({ AgEventType::AgentBalloonShow, texts[1] });
				break;
			}
		}

		if (st - ft >= std::chrono::milliseconds(delay * 10))
		{
			FrameInfo fi = ai.Frames[i++ % ai.Frames.size()];
			delay = fi.FrameDuration;

			ft = std::chrono::system_clock::now();

			aw->UpdateState({ AgEventType::AgentFrameChange, std::make_shared<FrameInfo>(fi) });
		}
		st = std::chrono::system_clock::now();

		ProcessMessages();
	}

	DestroyAgentFile(af);
	DestroyAgentWindow(aw);
	DestroyBalloonWindow(bw);
}