#include "AgentWindow.h"

using namespace Gdiplus;

// TODO: POR FAVOR: AO FINALIZAR A PARTE FUNCIONAL DO PROJETO, REFATORAR TENDO EM MENTE O CICLO DE VIDA DESSA BIBLIOTECA.

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
			AgEvent ev = {};

			ev.Type = (AgEventType)(uMsg & 0xFF); // se deus quiser a especificação não muda
			ev.Data = p;

			assert(ev.Type != AgEventType::WindowDragStart); // imagina q chato q seria

			PushWindowEvent(ev);
			return TRUE;
		}

		case WM_NCHITTEST:
		{
			LRESULT lr = DefWindowProc(hwnd, uMsg, wParam, lParam);

			if (lr == HTCLIENT)
				return HTCAPTION;

			return TRUE;
		}

		case WM_MOVING:
		{
			if (WindowStartDragging)
				return TRUE;

			WindowStartDragging = true;

			RECT* r = (RECT*)lParam;
			AgPoint ap = { r->left, r->top };

			PushWindowEvent({ AgEventType::WindowDragStart, ap });
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

			PushWindowEvent({ AgEventType::WindowDragEnd, ap });
			return TRUE;
		}

		case WM_NEWEVENT:
			ProcessUserEvent();
			break;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

// TODO: carregar o ícone do agente
int AgentWindow::InternalSetup(IAgentFile* af, uint16_t langId)
{
	CharacterInfo ci = af->GetCharacterInfo();

	GUID guid;
	HRESULT hr = CoCreateGuid(&guid);

	wchar_t guidString[64];
	hr = StringFromGUID2(guid, guidString, 64);

	WndClassName = L"agntwndclss" + std::wstring(guidString);

	WNDCLASS wc = {};

	wc.lpfnWndProc = (WNDPROC)IntWindowProc;
	wc.hInstance = hInstDll;
	wc.lpszClassName = WndClassName.c_str();

	if (!RegisterClass(&wc))
		return AGX_WND_CREATION_FAIL;

	RECT windowRect = {
		0,
		0,
		ci.Width,
		ci.Height
	};

	AdjustWindowRectEx(&windowRect, WindowStyle, false, WindowExStyle);

	HWND hwnd = CreateWindowEx(
		WindowExStyle,
		WndClassName.c_str(),
		af->GetLocalizedInfo(langId).CharName.c_str(),
		WindowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, 
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstDll, NULL
	);

	if (hwnd == NULL)
		return AGX_WND_CREATION_FAIL;

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	Width = ci.Width;
	Height = ci.Height;

	Handle = hwnd;

	AgRender.Setup(af);

	return AGX_WND_CREATION_SUCCESS;
}

void AgentWindow::PushWindowEvent(AgEvent e)
{
	std::scoped_lock<std::mutex> guard(EventsMutex);

	WindowEvents.push(e);
}

void AgentWindow::ProcessUserEvent()
{
	AgEvent e = PopUserEvent();
	auto& data = e.Data;

	switch (e.Type)
	{
		case AgEventType::AgentVisibleChange:
			ShowWindow(Handle, (std::get<bool>(data) ? SW_SHOW : SW_HIDE));
			break;
		case AgEventType::AgentFrameChange:
		{
			CurFrame = std::get<std::shared_ptr<FrameInfo>>(data);
			AgRender.Paint(Handle, CurFrame, CurMouth);
			break;
		}
		case AgEventType::AgentMouthChange:
		{
			CurMouth = std::get<MouthOverlayType>(data);
			AgRender.Paint(Handle, CurFrame, CurMouth);
			break;
		}
		case AgEventType::AgentMoveWindow:
		{
			AgRect ar = std::get<AgRect>(data);
			SetWindowPos(Handle, nullptr, ar.Left, ar.Top, -1, -1, SWP_NOMOVE | SWP_NOZORDER);
			break;
		}
	}
}

AgEvent AgentWindow::PopUserEvent()
{
	std::scoped_lock<std::mutex> guard(UpdateMutex);

	if (AgentUpdates.empty())
		return {};

	AgEvent e = AgentUpdates.front();
	AgentUpdates.pop();

	return e;
}

int AgentWindow::Setup(IAgentFile* af, uint16_t langId)
{
	return InternalSetup(af, langId);
}

bool AgentWindow::IsVisible()
{
	return IsWindowVisible(Handle);
}

void AgentWindow::UpdateState(AgEvent e) 
{
	std::scoped_lock<std::mutex> guard(EventsMutex);

	AgentUpdates.push(e);
	PostMessage(Handle, WM_NEWEVENT, 0, 0);
}

AgEvent AgentWindow::QueryEvent()
{
	if (WindowEvents.empty())
		return {};

	AgEvent e = WindowEvents.front();
	WindowEvents.pop();

	return e;
}

AgRect AgentWindow::GetRect()
{
	RECT r = {};

	GetWindowRect(Handle, &r);

	return AgRect{ r.left, r.top, r.right, r.bottom };
}

AgentWindow::~AgentWindow()
{
	UnregisterClass(WndClassName.c_str(), hInstDll);
}
