#include <windows.h>
#include <gdiplus.h>
#include <windowsx.h>

#define CLASS_NAME		L"Test123"
#define PB_CLASS_NAME	L"PaintBox"

#define IDB_CHANGE		0x1000
#define IDB_INVALIDATE	0x1001
#define IDB_CLOSEWINDOW 0x1002

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI PaintBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnPaintPaintBox(HDC hdc);
void AddCtls(HWND hWnd);
void ChangePaintBox();

HINSTANCE hInst;
HWND hPaintBox;
HWND hWnd;

int mouseHold = FALSE;

DWORD x;
DWORD y;
int i = 0;

Gdiplus::Color clr(0xFF00FF00);

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hInstancePrev, _In_ PWSTR lpCmdLine, _In_ int nCmdShow) {
	using namespace Gdiplus;

	hInst = hInstance;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.hInstance = hInstance;
	wcex.lpszClassName = CLASS_NAME;
	wcex.lpfnWndProc = WndProc;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.hIcon = LoadIcon(NULL, IDC_ICON);

	RegisterClassEx(&wcex);

	hWnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Hello world!",
		WS_VISIBLE | WS_POPUP | WS_THICKFRAME,
		400, 200,
		1280, 720,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hWnd == NULL) {
		DWORD err = GetLastError();
		wchar_t errStr[256];
		wsprintf(errStr, L"Could not create main window! Error Code: %d", err );
		MessageBox(NULL, errStr , L"Fatal error", MB_OK | MB_ICONERROR);
		return -1;
	}

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, RGB(255, 0, 0), 0, LWA_COLORKEY);

	WNDCLASSEXW wpbClass = { 0 };
	wpbClass.cbSize = sizeof(WNDCLASSEXW);
	wpbClass.hInstance = hInstance;
	wpbClass.lpszClassName = PB_CLASS_NAME;
	wpbClass.lpfnWndProc = PaintBoxProc;
	wpbClass.style = CS_HREDRAW | CS_VREDRAW;
	wpbClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wpbClass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	RegisterClassEx(&wpbClass);

	hPaintBox = CreateWindowEx(
		0,
		PB_CLASS_NAME,
		NULL,
		WS_VISIBLE | WS_CHILD,
		100, 100,
		300, 300,
		hWnd,
		NULL,
		hInstance,
		NULL
	);

	if (hPaintBox == NULL) {
		DWORD err = GetLastError();
		wchar_t errStr[256];
		wsprintf(errStr, L"Could not create paintbox window! Error Code: %d", err);
		MessageBox(NULL, errStr, L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	MSG msg = { 0 };
	
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	return (int) msg.wParam;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static DWORD err;
	RECT rectW;

	switch (uMsg) {
	case WM_CREATE:
		AddCtls(hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDB_INVALIDATE:
			MessageBox(hWnd, L"Invalidated!", L"Complete", MB_OK | MB_ICONINFORMATION);
			InvalidateRect(hPaintBox, NULL, TRUE);
			break;
		case IDB_CHANGE:
			MessageBox(hWnd, L"Paint box changed!", L"Complete", MB_OK | MB_ICONINFORMATION);
			ChangePaintBox();
			break;
		case IDB_CLOSEWINDOW:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_NCHITTEST:

		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);

		if ((x - 400) > 1280 || (y - 200) > 30) return HTBOTTOMRIGHT;

		return HTCAPTION;
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT WINAPI PaintBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static HDC hdc = {0};
	static PAINTSTRUCT ps = {0};

	switch (uMsg) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		
		OnPaintPaintBox(hdc);

		EndPaint(hWnd, &ps);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void OnPaintPaintBox(HDC hdc) {
	using namespace Gdiplus;

	wchar_t bufStr[256];
	wsprintf(bufStr, L"%d %d", x, y);
	Font font(&FontFamily(L"Tahoma"), 12);
	SolidBrush sb(Color(0xFF000000));

	Graphics graphics(hdc);
	Pen      pen(clr);
	graphics.DrawLine(&pen, 0, 0, 200, 100);
	graphics.DrawString(bufStr, -1, &font, PointF( 0.f, 0.f ), &sb);
}


void AddCtls(HWND hWnd) {
	CreateWindow(
		L"Button",
		L"Invalidate",
		WS_CHILD | WS_VISIBLE,
		50, 50,
		120, 30,
		hWnd,
		(HMENU)IDB_INVALIDATE,
		hInst,
		0
	);

	CreateWindow(
		L"Button",
		L"Change",
		WS_CHILD | WS_VISIBLE,
		180, 50,
		120, 30,
		hWnd,
		(HMENU)IDB_CHANGE,
		hInst,
		0
	);

	CreateWindow(
		L"Button",
		L"Close Window",
		WS_CHILD | WS_VISIBLE,
		320, 50,
		120, 50,
		hWnd,
		(HMENU)IDB_CLOSEWINDOW,
		hInst,
		0
	);

}

void ChangePaintBox() {
	clr.SetValue(0xFFFF0000);
}
