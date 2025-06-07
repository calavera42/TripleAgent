#include "AgentWindow.h"

LRESULT AgentWindow::IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AgentWindow* agx = (AgentWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (agx != nullptr)
		return agx->WindowProc(hwnd, uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT AgentWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT: 
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		Gdiplus::Graphics g(hdc);

		if (CurFrame)
			AgRender.Paint(&g, CurFrame, MouthOverlayType::Narrow);

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_NCHITTEST: {
		LRESULT hit = DefWindowProc(Handle, uMsg, wParam, lParam);
		if (hit == HTCLIENT) hit = HTCAPTION;
		return hit;
	}

	case WM_ERASEBKGND:
	{
		return 1;
	}

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

		ProcessUpdate();
	}
}

void AgentWindow::InternalSetup(IAgentFile* af, uint16_t langId, std::promise<int>& prom)
{
	CharacterInfo ci = af->GetCharacterInfo();

	AgRender.Setup(af);

	const wchar_t wndClass[] = L"agntwndclss";

	WNDCLASS wc = {};

	wc.lpfnWndProc = (WNDPROC)IntWindowProc;
	wc.hInstance = hInstDll;
	wc.lpszClassName = wndClass;

	if (!RegisterClass(&wc))
	{
		prom.set_value(0);
		return;
	}

	HWND hwnd = CreateWindowEx(
		WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED,
		wndClass,
		af->GetLocalizedInfo(langId).CharName.c_str(),
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		ci.Width, ci.Height,
		NULL, NULL, hInstDll, NULL
	);

	if (hwnd == NULL)
	{
		prom.set_value(1);
		return;
	}

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	Width = ci.Width;
	Height = ci.Height;

	Handle = hwnd;

	SetLayeredWindowAttributes(Handle, 0x00ff00ff, 255, LWA_COLORKEY);

	prom.set_value(0);

	MessageLoop();
}

UpdateInfo AgentWindow::GetUpdate()
{
	std::lock_guard<std::mutex> lock(QueueMutex);

	if (UpdateQueue.empty())
		return {};

	UpdateInfo ui = UpdateQueue.front();
	UpdateQueue.pop();

	return ui;
}

void AgentWindow::ProcessUpdate()
{
	UpdateInfo ui = GetUpdate();

	switch (ui.Type) 
	{
	case UpdateType::VisibleChange:
		ShowWindow(Handle, ui.WindowVisible ? SW_SHOW : SW_HIDE);
		break;
	case UpdateType::FrameChange:
	{
		CurFrame = ui.Frame;
		InvalidateRect(Handle, NULL, TRUE);
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

uint16_t AgentWindow::GetWidth()
{
	return Width;
}

uint16_t AgentWindow::GetHeight()
{
	return Height;
}

bool AgentWindow::IsVisible()
{
	return IsWindowVisible(Handle);
}

void AgentWindow::UpdateState(UpdateInfo info)
{
	std::lock_guard<std::mutex> lock(QueueMutex);
	UpdateQueue.push(info);
}

void AgentWindow::Do(IAgentFile* f)
{
	MessageLoop();
}
