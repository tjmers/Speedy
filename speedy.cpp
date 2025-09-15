#include "client.h"
#include "command_controller.h"
#include "config.h"
#include "graphics.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
Graphics* g;
Client* client;

constexpr int LINE_HEIGHT = 16;
constexpr int LINE_SPACING = 8; // the space in between lines
constexpr int LINE_SPACING_AND_HEIGHT = 24;
constexpr int ONE_HALF_LINE_SPACING = LINE_SPACING / 2;
constexpr int MAX_CHARS_PER_LINE = 30;
int client_width;
int client_height;
constexpr int SPACING_LEFT = 64;

bool show_cursor;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
	{
		WNDCLASSEXW window{};
		window.hInstance = hInstance;
		window.lpszClassName = L"mainwin";
		window.lpfnWndProc = WndProc;
		window.cbSize = sizeof(window);
		window.style = CS_OWNDC;
		window.cbClsExtra = 0;
		window.cbWndExtra = 0;
	window.hIcon = nullptr;
	window.hCursor = nullptr;
	window.lpszMenuName = nullptr;
	window.hIconSm = nullptr;
	RegisterClassExW(&window);
	
	RECT client_region = { 0, 0, STARTING_SCREEN_WIDTH, STARTING_SCREEN_HEIGHT };
	AdjustWindowRectEx(&client_region, WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_BORDER | WS_SIZEBOX, FALSE, 0);
	
	HWND hWnd = CreateWindowExW(
		0,
		L"mainwin",
		L"Speedy",
		WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_BORDER | WS_SIZEBOX,
		75, 25, client_region.right - client_region.left, client_region.bottom - client_region.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	
	g = new Graphics();
	client = new Client();
	client->open_file("test.txt");
	Config::create();
	CommandController::init(client);
	
	if (!g->Init(hWnd))
	{
		delete g;
		std::cerr << "An error occured while initializing the graphics object\n";
		return -1;
	}
	
	client_width = STARTING_SCREEN_WIDTH;
	client_height = STARTING_SCREEN_HEIGHT;
	
	
	
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	MSG msg;
	BOOL gResult;
	
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wparentheses"
	while (gResult = GetMessage(&msg, nullptr, 0, 0) > 0)
	#pragma GCC diagnostic pop // -Wparentheses
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (gResult == -1) {
		delete g;
		delete client;
		Config::get_instance()->save();
		Config::destroy();
		std::cerr << "An error occured while running the message loop. Code: " << GetLastError() << "\n";
		return -1;
	}
	delete g;
	delete client;
	Config::get_instance()->save();
	CommandController::get_instance()->save_commands();
	delete CommandController::get_instance();
	Config::destroy();
	return (int)msg.wParam;
}
#pragma GCC diagnostic pop // -Wunused-parameter



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		client_width = *(short*)&lParam;
		client_height = *((short*)&lParam + 1);
		g->Resize(client_width, client_height);
		return 0;
	case WM_SETFOCUS:
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_KILLFOCUS:
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_TIMER:
		return 0;
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_CHAR:
		client->process_character(static_cast<char>(wParam));
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_KEYDOWN:
		if (CommandController::get_instance()->run_commands()) {
			InvalidateRect(hWnd, NULL, TRUE);
			return 0;
		} 
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	case WM_KEYUP:
		return 0;
	case WM_PAINT:
		g->BeginDraw();
		g->ClearScreen(Config::get_instance()->get_background_color());
		client->draw(g);
		g->EndDraw();
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

}