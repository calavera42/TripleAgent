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
		HDC hdcBitmap = CreateCompatibleDC(hdc);

		auto oldBmp = SelectObject(hdcBitmap, FrameBitmap);

		BitBlt(hdc, 0, 0, Width, Height, hdcBitmap, 0, 0, SRCCOPY);

		SelectObject(hdc, oldBmp);
		DeleteDC(hdcBitmap);
		EndPaint(hwnd, &ps);
	}
		break;

	case WM_DESTROY:
		free(FrameBuffer.get());
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void AgentWindow::MessageLoop() const
{
	while (true) 
	{
		MSG message = { 0 };
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
		{ 
			DispatchMessage(&message); 
		}

		if (UserDrawFunction != nullptr)
			UserDrawFunction(*FrameBuffer);

		InvalidateRect(Handle, NULL, FALSE);
		UpdateWindow(Handle);

		Sleep(15);
	}
}

void AgentWindow::SetupBitmap()
{
	HDC hdcScreen = GetDC(NULL);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = Width;
	bmi.bmiHeader.biHeight = Height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	FrameBitmap = CreateDIBSection(
		hdcScreen, 
		&bmi, 
		DIB_RGB_COLORS, 
		reinterpret_cast<void**>(FrameBuffer.get()), 
		NULL, 
		NULL
	);

	ReleaseDC(NULL, hdcScreen);
}

int AgentWindow::Setup(uint16_t width, uint16_t height)
{
	const wchar_t wndClass[] = L"agntwndclss";

	WNDCLASS wc = {};

	wc.lpfnWndProc = (WNDPROC)IntWindowProc;
	wc.hInstance = hInstDll;
	wc.lpszClassName = wndClass;

	if (!RegisterClass(&wc))
	{
		MessageBoxW(NULL, L"Falha ao registrar a classe da janela do agente.", L"Agent Windowing API", MB_ICONERROR);
		return 0;
	}

	HWND hwnd = CreateWindowEx(
		WS_EX_NOACTIVATE,
		wndClass,
		L"agntsmth",
		WS_BORDER | WS_ACTIVECAPTION | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL, NULL, hInstDll, NULL
	);

	if (hwnd == NULL)
	{
		MessageBoxW(NULL, L"Falha ao criar janela do agente.", L"Agent Windowing API", MB_ICONERROR);
		return 0;
	}

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	Width = width;
	Height = height;

	PixelBufferSize = ((size_t)Width * (size_t)Height) * 4;

	uint32_t* frameBuffer = (uint32_t*)malloc(PixelBufferSize);

	if (frameBuffer == nullptr)
	{
		MessageBoxW(NULL, L"Falha ao alocar o buffer da janela.", L"Agent Windowing API", MB_ICONERROR);
		return 0;
	}

	memset(frameBuffer, 0, PixelBufferSize);

	FrameBuffer = std::make_shared<uint32_t*>(frameBuffer);

	Handle = hwnd;

	SetupBitmap();

	return 1;
}

void AgentWindow::RegisterUserFuntion(std::function<void(uint32_t*)> func)
{
	UserDrawFunction = func;
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
 
void AgentWindow::SetPosition(uint16_t x, uint16_t y)
{
	MoveWindow(Handle, x, y, Width, Height, false);
}

void AgentWindow::Hide()
{
	ShowWindow(Handle, SW_HIDE);
}

void AgentWindow::Show()
{
	ShowWindow(Handle, SW_SHOW);
}

void AgentWindow::StartMessageLoop()
{
	if (Handle == nullptr) 
	{
		MessageBox(NULL, L"A janela não foi criada.", L"Agent Windowing API", MB_ICONERROR);
		return;
	}

	MessageLoop();
}
