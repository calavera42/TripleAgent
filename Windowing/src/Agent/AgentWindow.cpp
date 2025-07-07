#include "AgentWindow.h"

using namespace Gdiplus;

LRESULT AgentWindow::IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AgentWindow* agx = (AgentWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (agx != nullptr)
		return agx->WindowProc(hwnd, uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
// TODO: consertar o evento de click
LRESULT AgentWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN: 
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		{
			AgPoint p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			Event ev = {};

			ev.Type = (EventType)(uMsg & 0xFF); // se deus quiser a especificação não muda
			ev.Data = p;

			assert(ev.Type != EventType::WindowDragStart); // imagina q chato q seria

			PushWindowEvent(ev);
			break;
		}

		case WM_NCHITTEST:
		{
			LRESULT lr = DefWindowProc(hwnd, uMsg, wParam, lParam);
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);


			if (lr == HTCLIENT)
			{
				return HTCAPTION;
			}

			break;
		}
		case WM_MOVING:
		{
			if (WindowStartDragging)
				return TRUE;

			WindowStartDragging = true;

			RECT* r = (RECT*)lParam;
			AgPoint ap = { r->left, r->top };

			PushWindowEvent({ EventType::WindowDragStart, ap });
			return TRUE;
		}

		case WM_EXITSIZEMOVE:
		{
			if (!WindowStartDragging)
				return TRUE;

			WindowStartDragging = false;

			RECT r = {};
			GetWindowRect(Handle, &r);

			AgPoint ap = { r.left, r.top };

			PushWindowEvent({ EventType::WindowDragEnd, ap });
			return TRUE;
		}

		case WM_ERASEBKGND:
			return 1;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void AgentWindow::MessageLoop() 
{
	while (true)
	{
		MSG message = { 0 };
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
		{ 
			DispatchMessage(&message); 
		}

		ProcessAgentUpdate();
	}
}

// TODO: carregar o ícone do agente
void AgentWindow::InternalSetup(IAgentFile* af, uint16_t langId, std::promise<int>& prom)
{
	CharacterInfo ci = af->GetCharacterInfo();

	const wchar_t wndClass[] = L"agntwndclss";

	int windowStyle = WS_CAPTION | WS_SYSMENU;
	int windowExStyle = WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED;

	WNDCLASS wc = {};

	wc.lpfnWndProc = (WNDPROC)IntWindowProc;
	wc.hInstance = hInstDll;
	wc.lpszClassName = wndClass;

	if (!RegisterClass(&wc))
	{
		prom.set_value(1);
		return;
	}

	RECT windowRect = {
		0,
		0,
		ci.Width,
		ci.Height
	};

	AdjustWindowRectEx(&windowRect, windowStyle, false, windowExStyle);

	HWND hwnd = CreateWindowEx(
		windowExStyle,
		wndClass,
		af->GetLocalizedInfo(langId).CharName.c_str(),
		windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, 
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstDll, NULL
	);

	if (hwnd == NULL)
	{
		prom.set_value(2);
		return;
	}

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	Width = ci.Width;
	Height = ci.Height;

	Handle = hwnd;

	prom.set_value(0);

	AgRender.Setup(af);

	MessageLoop();
}

void AgentWindow::UpdateState(Event info)
{
	std::lock_guard<std::mutex> lock(AgentQueueMutex);
	AgentUpdateQueue.push(info);
}

Event AgentWindow::GetAgentUpdate()
{
	std::lock_guard<std::mutex> lock(AgentQueueMutex);

	if (AgentUpdateQueue.empty())
		return {};

	Event ui = AgentUpdateQueue.front();
	AgentUpdateQueue.pop();

	return ui;
}

void AgentWindow::PushWindowEvent(Event e)
{
	std::lock_guard<std::mutex> guard(WindowEventsMutex);

	WindowEventsQueue.push(e);
}

void AgentWindow::ProcessAgentUpdate()
{
	Event ui = GetAgentUpdate();

	switch (ui.Type) 
	{
		case EventType::AgentVisibleChange:
			ShowWindow(Handle, std::get<bool>(ui.Data) ? SW_SHOW : SW_HIDE);
			break;
		case EventType::AgentFrameChange:
		{
			CurFrame = std::get<std::shared_ptr<FrameInfo>>(ui.Data);
			AgRender.Paint(Handle, CurFrame, CurMouth);
			break;
		}
		case EventType::AgentMoveWindow:
		{
			AgPoint targetPos = std::get<AgPoint>(ui.Data);
			SetWindowPos(
				Handle, 
				NULL, 
				targetPos.X, 
				targetPos.Y, 
				-1, -1, 
				SWP_NOSIZE | 
				SWP_NOZORDER
			);
			break;
		}
		case EventType::AgentMouthChange:
		{
			CurMouth = std::get<MouthOverlayType>(ui.Data);
			AgRender.Paint(Handle, CurFrame, CurMouth);
			break;
		}
		default:
			break;
	}
}

int AgentWindow::Setup(IAgentFile* af, uint16_t langId)
{
	std::promise<int> promise;
	std::future<int> future = promise.get_future();

	std::thread([this, af, langId, prom = std::move(promise)]() mutable {
		this->InternalSetup(af, langId, prom);
	}).detach();

	return future.get();
}

bool AgentWindow::IsVisible()
{
	return IsWindowVisible(Handle);
}

Event AgentWindow::QueryEvent()
{
	std::lock_guard<std::mutex> lock(WindowEventsMutex);

	if (WindowEventsQueue.empty())
		return {};

	Event e = WindowEventsQueue.front();
	WindowEventsQueue.pop();

	return e;
}

AgPoint AgentWindow::GetSize()
{
	return AgPoint{ Width, Height };
}

AgPoint AgentWindow::GetPos()
{
	RECT r = {};

	GetWindowRect(Handle, &r);

	return AgPoint{ r.left, r.top };
}
