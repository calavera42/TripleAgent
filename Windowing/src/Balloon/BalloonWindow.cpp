#include "BalloonWindow.h"

using namespace Gdiplus;

LRESULT BalloonWindow::IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void BalloonWindow::MessageLoop()
{
	while (true)
	{
		MSG message = { 0 };
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&message);

		ProcessUserEvent();
	}
}

int BalloonWindow::Setup(CharacterInfo ci)
{
	std::promise<int> promise;
	std::future<int> future = promise.get_future();

	std::thread([this, ci, prom = std::move(promise)]() mutable {
		this->InternalSetup(ci, prom);
	}).detach();

	return future.get();
}

void BalloonWindow::InternalSetup(CharacterInfo ci, std::promise<int>& prom)
{
	const wchar_t wndClass[] = L"agntblnclss";
	
	WNDCLASS wc = {};

	wc.lpfnWndProc = (WNDPROC)IntWindowProc;
	wc.hInstance = hInstDll;
	wc.lpszClassName = wndClass;

	if (!RegisterClass(&wc))
	{
		prom.set_value(1);
		return;
	}

	Handle = CreateWindowEx(
		WindowExStyle = WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
		wndClass,
		L"bloon",
		WindowStyle = WS_POPUP | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300,
		300,
		NULL, NULL, hInstDll, NULL
	);

	if (!Handle)
	{
		prom.set_value(2);
		return;
	}

	BalloonRenderer = new SpeechBalloonRenderer();
	BalloonRenderer->Setup(ci);

	prom.set_value(0);

	MessageLoop();
}

void BalloonWindow::ProcessUserEvent()
{
	if (!UpdateAdded)
		return;

	std::lock_guard<std::mutex> guard(UserUpdateMutex);

	while (!UserEvents.empty()) 
	{
		Event e = UserEvents.front();
		UserEvents.pop();

		switch (e.Type) {
			case EventType::AgentBalloonShow: 
			{
				string s = std::get<string>(e.Data);

				BalloonText = s;

				ShowWindow(Handle, SW_SHOW);
				break;
			}
			case EventType::AgentBalloonPace:
			{
				// TODO: procesar pace update
				break;
			}
			case EventType::AgentBalloonHide:
				ShowWindow(Handle, SW_HIDE);
				break;
			case EventType::AgentBalloonAttach:
			{
				AgRect arect = std::get<AgRect>(e.Data);
				RECT wndApiRect = { arect.Left, arect.Top, arect.Right, arect.Bottom };
				HMONITOR curScreen = MonitorFromRect(&wndApiRect, MONITOR_DEFAULTTOPRIMARY);
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

				workingRect.Inflate(-40, -40); // margem da borda

				Place(TipQuadrant::Top, agentRect, workingRect);

				BalloonRenderer->Paint(
					Handle,
					{
						TipType,
						TipPosition,
						BalloonText,
						false,
						-1
					}
				);
				break;
			}
			default:
				break;
		}
	}
	UpdateAdded = false;
}


struct PlaceAlgHelper 
{
	int* ChangeVector;
	int* LockedVector;
	int FirstVal;
	int SecondVal;
	int ChangeVectorDefaultPlace;
	int LockedVectorPlacement;
	int TryRange;
	int TipPosTarget;
};
void BalloonWindow::Place(TipQuadrant tq, Gdiplus::Rect agRect, Gdiplus::Rect wkRect)
{
	if ((int)tq > 3)
		return;

	Rect myRect = BalloonRenderer->GetSize(tq, BalloonText);

	bool success = false;
	bool intersects = false;

	int topDist = abs(agRect.GetTop() - wkRect.GetTop());
	int leftDist = abs(agRect.GetLeft() - wkRect.GetLeft());
	int rightDist = abs(wkRect.GetRight() - agRect.GetRight());
	int bottomDist = abs(wkRect.GetBottom() - agRect.GetBottom());

	int halfW = myRect.Width / 2;
	int halfH = myRect.Height / 2;

	PlaceAlgHelper data[4] = {
		{ // TOP
			&myRect.X, 
			&myRect.Y, 
			leftDist, 
			rightDist,  
			agRect.GetLeft() - myRect.Width + SBCornerSpacing, 
			agRect.GetBottom(), 
			myRect.Width, 
			agRect.GetLeft() + agRect.Width / 2 
		},
		{ // RIGHT
			&myRect.Y, 
			&myRect.X, 
			topDist,  
			bottomDist, 
			agRect.GetTop(),
			agRect.GetLeft() - myRect.Width, 
			myRect.Height, 
			agRect.GetTop() + agRect.Height / 2 
		},
		{ // BOTTOM
			&myRect.X, 
			&myRect.Y, 
			leftDist, 
			rightDist, 
			agRect.GetLeft(), 
			agRect.GetTop() - myRect.Height, 
			myRect.Width,  
			agRect.GetLeft() + agRect.Width / 2 
		},
		{ // LEFT
			&myRect.Y, 
			&myRect.X, 
			topDist,  
			bottomDist, 
			agRect.GetTop(),
			agRect.GetRight(), 
			myRect.Height, 
			agRect.GetTop() + agRect.Height / 2 
		},
	};

	PlaceAlgHelper& pah = data[static_cast<size_t>(tq)];

	int tryDelta = (pah.FirstVal > pah.SecondVal) ? -1 : 1;

	*pah.ChangeVector = pah.ChangeVectorDefaultPlace;
	*pah.LockedVector = pah.LockedVectorPlacement;

	for (int i = 0; i < pah.TryRange; i++)
	{
		if (success = wkRect.Contains(myRect))
			break;

		*pah.ChangeVector += tryDelta;
	}

	int tipPos = pah.TipPosTarget - *pah.ChangeVector;

	if (success && tipPos < pah.TryRange - SBCornerSpacing * 2)
	{
		TipPosition = tipPos;
		TipType = tq;

		SetWindowPos(Handle, nullptr, myRect.X, myRect.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	else
		Place((TipQuadrant)((int)tq + 1), agRect, wkRect);
}

void BalloonWindow::UpdateState(Event e)
{
	std::lock_guard<std::mutex> guard(UserUpdateMutex);

	UpdateAdded = true;
	UserEvents.push(e);
}

Event BalloonWindow::QueryEvent()
{
	std::lock_guard<std::mutex> guard(WindowEventMutex);

	if (WindowEvents.empty())
		return {};

	Event e = WindowEvents.front();
	WindowEvents.pop();

	return e;
}

BalloonWindow::~BalloonWindow()
{
	delete BalloonRenderer;
}
