#include <Windows.h>
#include <iostream>

struct MarchingSquareCorner
{
	POINT position;
	int radius;
	bool enabled;
	bool hovered;
};

struct MarchingSquare
{
	MarchingSquareCorner a;
	MarchingSquareCorner b;
	MarchingSquareCorner c;
	MarchingSquareCorner d;
	POINT e;
	POINT f;
	POINT g;
	POINT h;
	RECT rectangle;
	char state;
}square;

HWND hMainWindow;
HWND hRenderingPanel;

WNDPROC oldRenderingPanelProc;

static bool pointInCircle(int x, int y, int circleX, int circleY, int circleRadius)
{
	long double dist = sqrt(pow(x - circleX, 2.0) + pow(y - circleY, 2.0));
	return dist < circleRadius;
}

static bool pointInCircle(int x, int y, POINT circlePosition, int circleRadius)
{
	return pointInCircle(x, y, circlePosition.x, circlePosition.y, circleRadius);
}

static bool fillCircle(HDC dc, int x, int y, int radius, HBRUSH brush = NULL)
{
	if (brush) { SelectObject(dc, brush); }
	return Ellipse(dc, x - radius, y - radius, x + radius, y + radius);
}

static bool fillCircle(HDC dc, POINT positionCenter, int radius, HBRUSH brush = NULL)
{
	return fillCircle(dc, positionCenter.x, positionCenter.y, radius, brush);
}

static void drawLetters(HDC dc)
{
	SetTextColor(dc, RGB(255, 255, 255));
	TextOut(dc, square.a.position.x - 20, square.a.position.y + square.a.radius, L"A", 1);
	TextOut(dc, square.b.position.x + 20, square.b.position.y + square.b.radius, L"B", 1);
	TextOut(dc, square.c.position.x + 20, square.c.position.y - square.c.radius - 12, L"C", 1);
	TextOut(dc, square.d.position.x - 20, square.d.position.y - square.d.radius - 15, L"D", 1);

	SetTextColor(dc, RGB(255, 255, 0));
	TextOut(dc, square.e.x - 4, square.e.y + 15, L"E", 1);
	TextOut(dc, square.f.x + 15, square.f.y - 7, L"F", 1);
	TextOut(dc, square.g.x - 4, square.g.y + 15, L"G", 1);
	TextOut(dc, square.h.x - 25, square.h.y - 7, L"H", 1);
}

static void drawState(HDC dc, RECT* panelRectangle)
{
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(0, 255, 0));

	wchar_t t[16]{ 0 };
	swprintf_s(t, L"State: %d", square.state);
	int length = static_cast<int>(wcslen(t));
	int x = 160;
	if (panelRectangle)
	{
		SIZE size;
		if (GetTextExtentPoint32(dc, t, length, &size))
		{
			x = panelRectangle->right / 2 - size.cx / 2;
		}
	}

	TextOut(dc, x, 10, t, length);
}

static void drawIsolines(HDC dc)
{
	if (square.state > 0)
	{
		HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		if (pen)
		{
			SelectObject(dc, pen);

			switch (square.state)
			{
				case 1:
				{
					POINT points[]{ square.d.position, square.g, square.h };
					Polygon(dc, points, 3);
					break;
				}

				case 2:
				{
					POINT points[]{ square.c.position, square.g, square.f };
					Polygon(dc, points, 3);
					break;
				}

				case 3:
				{
					POINT points[]{ square.h, square.f, square.c.position, square.d.position};
					Polygon(dc, points, 4);
					break;
				}

				case 4:
				{
					POINT points[]{ square.b.position, square.f, square.e };
					Polygon(dc, points, 3);
					break;
				}

				case 5:
				{
					POINT points[]{ square.e, square.b.position, square.f, square.g, square.d.position, square.h };
					Polygon(dc, points, 6);
					break;
				}

				case 6:
				{
					POINT points[]{ square.e, square.b.position, square.c.position, square.g };
					Polygon(dc, points, 4);
					break;
				}

				case 7:
				{
					POINT points[]{ square.e, square.b.position, square.c.position, square.d.position, square.d.position, square.h };
					Polygon(dc, points, 6);
					break;
				}

				case 8:
				{
					POINT points[]{ square.a.position, square.e, square.h };
					Polygon(dc, points, 3);
					break;
				}

				case 9:
				{
					POINT points[]{ square.a.position, square.e, square.g, square.d.position };
					Polygon(dc, points, 4);
					break;
				}

				case 10:
				{
					POINT points[]{ square.a.position, square.e, square.f, square.c.position, square.g, square.h };
					Polygon(dc, points, 6);
					break;
				}

				case 11:
				{
					POINT points[]{ square.a.position, square.e, square.f, square.c.position, square.d.position };
					Polygon(dc, points, 5);
					break;
				}

				case 12:
				{
					POINT points[]{ square.a.position, square.b.position, square.f, square.h };
					Polygon(dc, points, 4);
					break;
				}

				case 13:
				{
					POINT points[]{ square.a.position, square.b.position, square.f, square.g, square.d.position };
					Polygon(dc, points, 5);
					break;
				}

				case 14:
				{
					POINT points[]{ square.a.position, square.b.position, square.c.position, square.g, square.h };
					Polygon(dc, points, 5);
					break;
				}

				case 15:
				{
					POINT points[]{ square.a.position, square.b.position, square.c.position, square.d.position };
					Polygon(dc, points, 4);
					break;
				}
			}

			DeleteObject(pen);
		}
	}
}

static void drawPoints(HDC dc)
{
	HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	if (pen)
	{
		SelectObject(dc, pen);
		SelectObject(dc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

		fillCircle(dc, square.a.position, 10);
		fillCircle(dc, square.b.position, 10);
		fillCircle(dc, square.c.position, 10);
		fillCircle(dc, square.d.position, 10);

		fillCircle(dc, square.e, 10);
		fillCircle(dc, square.f, 10);
		fillCircle(dc, square.g, 10);
		fillCircle(dc, square.h, 10);

		DeleteObject(pen);
	}
}

static void drawCorner(HDC dc, MarchingSquareCorner* corner)
{
	bool mustDeleteObject;
	HPEN pen;
	if (corner->hovered)
	{
		pen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		mustDeleteObject = true;
	}
	else
	{
		pen = static_cast<HPEN>(GetStockObject(WHITE_PEN));
		mustDeleteObject = false;
	}

	if (pen)
	{
		HBRUSH brush = static_cast<HBRUSH>(GetStockObject(corner->enabled ? WHITE_BRUSH : BLACK_BRUSH));
		SelectObject(dc, pen);
		SelectObject(dc, brush);
		fillCircle(dc, corner->position, corner->radius);

		if (mustDeleteObject) { DeleteObject(pen); }
	}
}

static void renderSquare(HDC dc, RECT* panelRectangle)
{
	SelectObject(dc, static_cast<HPEN>(GetStockObject(WHITE_PEN)));

	drawCorner(dc, &square.a);
	drawCorner(dc, &square.b);
	drawCorner(dc, &square.c);
	drawCorner(dc, &square.d);

	drawPoints(dc);
	drawIsolines(dc);

	drawState(dc, panelRectangle);
	drawLetters(dc);
}

static bool render(HWND panel)
{
	RECT panelRectangle;
	if (!GetClientRect(panel, &panelRectangle))
	{
		std::cerr << "Can't get panel size!" << std::endl;
		return false;
	}

	HDC dc = GetDC(panel);
	if (!dc)
	{
		std::cerr << "Can't get panel DC!" << std::endl;
		return false;
	}

	HDC dcBuffer = CreateCompatibleDC(dc);
	if (!dcBuffer)
	{
		std::cerr << "Can't create compatible DC!" << std::endl;
		ReleaseDC(panel, dc);
		return false;
	}

	HBITMAP bitmap = CreateCompatibleBitmap(dc, panelRectangle.right, panelRectangle.bottom);
	if (!bitmap)
	{
		std::cerr << "Can't create compatible bitmap!" << std::endl;
		DeleteDC(dcBuffer);
		ReleaseDC(panel, dc);
		return false;
	}

	SelectObject(dcBuffer, bitmap);
	HBRUSH brush = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	FillRect(dcBuffer, &panelRectangle, brush);

	renderSquare(dcBuffer, &panelRectangle);

	BitBlt(dc, 0, 0, panelRectangle.right, panelRectangle.bottom, dcBuffer, 0, 0, SRCCOPY);

	DeleteDC(dcBuffer);
	DeleteObject(bitmap);
	ReleaseDC(panel, dc);

	return true;
}

static char updateSquareState()
{
	square.state = 15 - (square.a.enabled * 8 + square.b.enabled * 4 + square.c.enabled * 2 + square.d.enabled);
	return square.state;
}

static void initializeSquare(int margin)
{
	if (margin < 10) { margin = 10; }
	square.a.radius = square.b.radius = square.c.radius = square.d.radius = margin;

	RECT panelRect;
	if (GetClientRect(hRenderingPanel, &panelRect))
	{
		square.rectangle = RECT{ margin, margin, panelRect.right - margin, panelRect.bottom - margin };

		square.a.position = POINT{ square.rectangle.left, square.rectangle.top };
		square.b.position = POINT{ square.rectangle.right, square.rectangle.top };
		square.c.position = POINT{ square.rectangle.right, square.rectangle.bottom };
		square.d.position = POINT{ square.rectangle.left, square.rectangle.bottom };

		int halfSize = panelRect.right / 2;
		square.e = POINT{ halfSize, square.rectangle.top };
		square.f = POINT{ square.rectangle.right, halfSize};
		square.g = POINT{ halfSize, square.rectangle.bottom };
		square.h = POINT{ square.rectangle.left, halfSize };
	}

	updateSquareState();
}

static MarchingSquareCorner* GetHoveredCorner(int mouseX, int mouseY)
{
	if (pointInCircle(mouseX, mouseY, square.a.position, square.a.radius))
	{
		return &square.a;
	} else if (pointInCircle(mouseX, mouseY, square.b.position, square.b.radius))
	{
		return &square.b;
	} else if (pointInCircle(mouseX, mouseY, square.c.position, square.c.radius))
	{
		return &square.c;
	} else if (pointInCircle(mouseX, mouseY, square.d.position, square.d.radius))
	{
		return &square.d;
	}

	return NULL;
}

static void unhoverAll()
{
	square.a.hovered = square.b.hovered = square.c.hovered = square.d.hovered = false;
}

static LRESULT CALLBACK renderingPanelProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_MOUSEMOVE:
		{
			short x = LOWORD(lParam);
			short y = HIWORD(lParam);

			unhoverAll();
			MarchingSquareCorner* corner = GetHoveredCorner(x, y);
			if (corner) { corner->hovered = true; }
			render(hRenderingPanel);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			SetFocus(hWnd);

			short x = LOWORD(lParam);
			short y = HIWORD(lParam);

			MarchingSquareCorner* corner = GetHoveredCorner(x, y);
			if (corner)
			{
				corner->enabled = !corner->enabled;
				updateSquareState();
				render(hRenderingPanel);
			}

			return 0;
		}

		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_ESCAPE:
				case VK_RETURN:
					DestroyWindow(hMainWindow);
			}
			return 0;
	}

	return CallWindowProc(oldRenderingPanelProc, hWnd, uMessage, wParam, lParam);
}

static LRESULT CALLBACK mainWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			render(hRenderingPanel);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_SIZE:
		{
			short w = LOWORD(lParam);
			short h = HIWORD(lParam);
			SetWindowPos(hRenderingPanel, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			initializeSquare(40);
			render(hRenderingPanel);
			return 0;
		}

		case WM_SETFOCUS:
			SetFocus(hRenderingPanel);
			return 0;

		case WM_CREATE:
			hRenderingPanel = CreateWindowW(L"Static", L"Rendering panel",
				WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,
				0, 0, 0, 0, hWnd, NULL, NULL, NULL);
			oldRenderingPanelProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hRenderingPanel, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(renderingPanelProc)));
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

int main()
{
	const LPCWSTR MY_CLASS = L"winapi_window_class";
	const LPCWSTR MY_TITLE = L"Marching square WinAPI";
	const HINSTANCE selfInstance = GetModuleHandle(NULL);

	WNDCLASSEX wndClassEx{ 0 };
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = mainWndProc;
	wndClassEx.hInstance = selfInstance;
	wndClassEx.lpszClassName = MY_CLASS;

	if (!RegisterClassEx(&wndClassEx))
	{
		std::cerr << "Window class registering failed!" << std::endl;
		MessageBox(NULL, L"Window class registering failed!", MY_TITLE, MB_ICONERROR);
		return EXIT_FAILURE;
	}

	const int windowWidth = 400;
	const int windowHeight = 400;
	hMainWindow = CreateWindow(wndClassEx.lpszClassName, MY_TITLE,
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
		NULL, NULL, selfInstance, NULL);

	if (hMainWindow)
	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0u, 0u))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
	{
		std::cerr << "Failed to create main window!" << std::endl;
	}

	if (!UnregisterClass(wndClassEx.lpszClassName, wndClassEx.hInstance))
	{
		std::cerr << "Window class unregistering failed!" << std::endl;
	}

	return EXIT_SUCCESS;
}
