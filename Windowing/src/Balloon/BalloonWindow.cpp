#include "BalloonWindow.h"

using namespace Gdiplus;

void BalloonWindow::Redraw()
{
	BalloonRenderer->Paint(
		Handle,
		{
			TipType,
			TipPosition,
			BalloonText,
			((int)CharInfo.Flags & (int)CharacterFlags::BalloonAutoPaceDisabled) != 0,
			SpeechPace
		}
	);
}

int BalloonWindow::Setup(CharacterInfo ci)
{
	return InternalSetup(ci);
}

LRESULT BalloonWindow::IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BalloonWindow* agx = (BalloonWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (agx != nullptr)
		return agx->WindowProc(hwnd, uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT BalloonWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_NEWEVENT:
			ProcessUserEvents();
			break;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

int BalloonWindow::InternalSetup(CharacterInfo ci)
{
	GUID guid;
	HRESULT hr = CoCreateGuid(&guid);
	wchar_t guidString[64];
	hr = StringFromGUID2(guid, guidString, 64);
	WndClassName = L"agntblwndclss" + std::wstring(guidString);
	
	WNDCLASS wc = {};

	wc.lpfnWndProc = (WNDPROC)IntWindowProc;
	wc.hInstance = hInstDll;
	wc.lpszClassName = WndClassName.c_str();

	if (!RegisterClass(&wc))
		return AGX_WND_CREATION_FAIL;

	Handle = CreateWindowEx(
		WindowExStyle,
		WndClassName.c_str(),
		L"bloon",
		WindowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300,
		300,
		NULL, NULL, hInstDll, NULL
	);

	if (!Handle)
		return AGX_WND_CREATION_FAIL;

	SetWindowLongPtr(Handle, GWLP_USERDATA, (LONG_PTR)this);

	BalloonRenderer = new SpeechBalloonRenderer();
	BalloonRenderer->Setup(ci);

	return AGX_WND_CREATION_SUCCESS;
}

struct PlaceAlgHelper 
{
	int* ChangeVector;
	int* LockedVector;

	int ChangeVectorDefaultPlace; // valor absoluto
	int LockedVectorPlacement; // valor absoluto
	int TryRange; // valor relativo
	int TipPosTarget; // valor absoluto
};
void BalloonWindow::Place(Gdiplus::Rect agRect, Gdiplus::Rect wkRect, TipQuadrant tq = TipQuadrant::Unset)
{
	bool success = false;
	bool intersects = false;

	// dois cantos (já que uma aresta tem dois vértices) + a largura da ponta
	int minBalloonCoverage = (SBCornerSpacingX + SBCornerDiameterY) + SBTipSpacing;

	int topDist = abs(agRect.GetTop() - wkRect.GetTop());
	int leftDist = abs(agRect.GetLeft() - wkRect.GetLeft());
	int rightDist = abs(wkRect.GetRight() - agRect.GetRight());
	int bottomDist = abs(wkRect.GetBottom() - agRect.GetBottom());

	if (tq == TipQuadrant::Unset) {
		int array[4] = // inacreditável q não dá pra inicializar um array como parâmetro...
		{
			topDist, rightDist, 
			bottomDist, leftDist
		};

		tq = GetBestTQ(array);
	}

	Rect myRect = BalloonRenderer->GetSize(tq, BalloonText);

	int halfW = myRect.Width / 2;
	int halfH = myRect.Height / 2;

	PlaceAlgHelper lookup[4] = {
		{ // TOP
			&myRect.X,
			&myRect.Y,
			agRect.GetLeft() - halfW,
			agRect.GetBottom(),
			myRect.Width * 2,
			agRect.GetLeft() + agRect.Width / 2
		},
		{ // RIGHT
			&myRect.Y,
			&myRect.X,
			agRect.GetTop() - halfH,
			agRect.GetLeft() - myRect.Width,
			myRect.Height * 2,
			agRect.GetTop() + agRect.Height / 2
		},
		{ // BOTTOM
			&myRect.X,
			&myRect.Y,
			agRect.GetLeft() - halfW,
			agRect.GetTop() - myRect.Height,
			myRect.Width * 2,
			agRect.GetLeft() + agRect.Width / 2
		},
		{ // LEFT
			&myRect.Y,
			&myRect.X,
			agRect.GetTop() - halfH,
			agRect.GetRight(),
			myRect.Height * 2,
			agRect.GetTop() + agRect.Height / 2
		}
	};
	int i = (int)tq; // o visual studio gosta muito de reclamar
	PlaceAlgHelper& pah = lookup[i];

	*pah.ChangeVector = pah.ChangeVectorDefaultPlace;
	*pah.LockedVector = pah.LockedVectorPlacement;

	for (int i = 0; i < pah.TryRange && !success; i++)
	{
		int balloonEdge = tq == TipQuadrant::Top || tq == TipQuadrant::Bottom
			? myRect.GetRight() 
			: myRect.GetBottom();

		// só deu certo se:
		// 1: o balão está dentro da tela, e,
		// 2: se a ponta do balão aponta para o agente
		if (success = (wkRect.Contains(myRect) && pah.TipPosTarget < balloonEdge - minBalloonCoverage))
			break;

		*pah.ChangeVector += 1;
	}

	int tipPos = pah.TipPosTarget - *pah.ChangeVector;

	TipPosition = tipPos;
	TipType = tq;

	SetWindowPos(Handle, nullptr, myRect.X, myRect.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void BalloonWindow::ProcessUserEvents() 
{
	AgEvent e = PopUserEvent();
	auto& data = e.Data;

	switch (e.Type) 
	{
		case AgEventType::AgentBalloonShow:
			Show(std::get<string>(data));
			break;
		case AgEventType::AgentBalloonHide:
			Hide();
			break;
		case AgEventType::AgentBalloonAttach:
			AttachToAgent(std::get<std::shared_ptr<IAgentWindow>>(data));
			break;
		case AgEventType::AgentBalloonPace:
			SpeechPace = std::get<int>(data);
			break;
	}
}

AgEvent BalloonWindow::PopUserEvent()
{
	std::scoped_lock<std::mutex> guard(UpdateMutex);

	if (AgentUpdates.empty())
		return {};

	AgEvent e = AgentUpdates.front();
	AgentUpdates.pop();

	return e;
}

void BalloonWindow::UpdateState(AgEvent e) 
{
	std::scoped_lock<std::mutex> guard(UpdateMutex);

	AgentUpdates.push(e);
	PostMessage(Handle, WM_NEWEVENT, 0, 0);
}

BalloonWindow::~BalloonWindow()
{
	delete BalloonRenderer;
}

void BalloonWindow::Show(string t)
{
	ShowWindow(Handle, SW_HIDE);

	AgRect arect = OwnerAgentWnd->GetRect();
	RECT wndApiRect = { arect.Left, arect.Top, arect.Right, arect.Bottom };
	HMONITOR curScreen = MonitorFromRect(&wndApiRect, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monInfo = {};

	memset(&monInfo, 0, sizeof(monInfo));
	monInfo.cbSize = sizeof(monInfo);

	GetMonitorInfoW(curScreen, &monInfo);

	Rect workingRect(
		monInfo.rcWork.left,
		monInfo.rcWork.top,
		monInfo.rcWork.right - monInfo.rcWork.left,
		monInfo.rcWork.bottom - monInfo.rcWork.top
	);
	Rect agentRect(
		arect.Left,
		arect.Top,
		arect.Right - arect.Left,
		arect.Bottom - arect.Top
	);

	// não é interessante que o balão grude na borda da tela, por isso, 
	// recue um pouco o retângulo para efetivamente criar uma margem
	workingRect.Inflate(WorkingRectMargin, WorkingRectMargin);

	BalloonText = t;
	SpeechPace = -1; // TODO: lembrar q tem o pace rsxd

	Place(agentRect, workingRect);

	Redraw();

	ShowWindow(Handle, SW_SHOW);
}

void BalloonWindow::Hide()
{
	ShowWindow(Handle, SW_HIDE);
}

void BalloonWindow::AttachToAgent(std::shared_ptr<IAgentWindow> iaw)
{
	OwnerAgentWnd = iaw;
}

// ordem: top, right, bottom, left
TipQuadrant BalloonWindow::GetBestTQ(int dist[4])
{
	int smallestIndex = 0;
	for (size_t i = 0; i < 4; i++)
		if (dist[i] < dist[smallestIndex])
			smallestIndex = i;

	printf("Mais perto de: %d\n", (TipQuadrant)smallestIndex);

	return (TipQuadrant)smallestIndex; // faz sentido confia
}
