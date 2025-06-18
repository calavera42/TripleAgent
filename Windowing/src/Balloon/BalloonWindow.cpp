#include "BalloonWindow.h"

using namespace Gdiplus;

LRESULT BalloonWindow::IntWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BalloonWindow* agb = (BalloonWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (agb != nullptr)
		return agb->WindowProc(hwnd, uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT BalloonWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		Graphics screen(hdc);

		
		
		EndPaint(hwnd, &ps);
		break;
	}

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void BalloonWindow::MessageLoop()
{
	while (true)
	{
		MSG message = { 0 };
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&message);
		}
	}
}

int BalloonWindow::Setup(BalloonInfo bi, CharacterInfo ci)
{
	std::promise<int> promise;
	std::future<int> future = promise.get_future();

	std::thread([this, bi, ci, prom = std::move(promise)]() mutable {
		this->InternalSetup(bi, ci, prom);
	}).detach();

	return future.get();
}

void BalloonWindow::InternalSetup(BalloonInfo bi, CharacterInfo ci, std::promise<int>& prom)
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
		WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED,
		wndClass,
		L"bloonl",
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		200,
		70,
		NULL, NULL, hInstDll, NULL
	);

	if (!Handle)
	{
		prom.set_value(2);
		return;
	}

	SetWindowLongPtr(Handle, GWLP_USERDATA, (LONG_PTR)this);

	Width = 200;
	Height = 70;

	SetLayeredWindowAttributes(Handle, 0x00880000, 255, LWA_COLORKEY);

	prom.set_value(0);

	MessageLoop();
}

AgRect BalloonWindow::CalcWinSize(AgRect textSize) // TODO: rsrs
{
	return AgRect();
}

void BalloonWindow::Show(std::wstring text) // TODO: rsrs
{
}

void BalloonWindow::PaceUpdate() // TODO: rsrs
{
}
