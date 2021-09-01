//ウィンドウプロシージャ
#include "DefferdRenderer.h"
#include <Windows.h>
#include <thread>
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	if (msg == WM_PAINT)
	{
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
	if (msg == WM_DESTROY)
	{
		//アプリケーション終了通知
		PostQuitMessage(0);
		return 0;
	}
	//デフォルト処理
	return DefWindowProc(hwnd, msg, wparam, lparam);

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	constexpr int WINDOW_WIDTH = 680;
	constexpr int WINDOW_HEGHT = WINDOW_WIDTH*3/4;

	DefferdTest theApp;

	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = WindowProcedure;
	w.hCursor = LoadCursor(NULL, IDC_ARROW);
	w.lpszClassName = L"DX12App";
	RegisterClassEx(&w);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = { 0,0,WINDOW_WIDTH,WINDOW_HEGHT };
	AdjustWindowRect(&rect, dwStyle, FALSE);


	auto hwnd = CreateWindow(w.lpszClassName, L"DX12App",
		dwStyle,
		CW_USEDEFAULT,CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr,
		nullptr,
		hInstance,
		&theApp
		);

	theApp.intialize(hwnd);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&theApp));
	ShowWindow(hwnd, nCmdShow);

	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		theApp.render();
	}

	theApp.terminate();

	return static_cast<int>(msg.wParam);
}