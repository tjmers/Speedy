#include "client.h"
#include "command_controller.h"
#include "config.h"
#include "graphics.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
Graphics* g;

constexpr int LINE_HEIGHT = 16;
constexpr int LINE_SPACING = 8; // the space in between lines
constexpr int LINE_SPACING_AND_HEIGHT = 24;
constexpr int ONE_HALF_LINE_SPACING = LINE_SPACING / 2;
constexpr int MAX_CHARS_PER_LINE = 30;
int client_width;
int client_height;
constexpr int SPACING_LEFT = 64;

bool show_cursor;

// Global variables for mouse tracking
bool is_mouse_selecting = false;
POINT last_mouse_pos = {0, 0};

// Helper function to convert mouse position to text position
void MouseToTextPosition(int mouse_x, int mouse_y, int& line, int& character) {
    const float font_size = Config::get_instance()->get_font_size();
    const int line_height = static_cast<int>(font_size * 1.25f);
    const float char_width = font_size * 0.6f;
    const float offset_x = Config::get_instance()->get_left_margin() + Config::get_instance()->get_explorer_width();
    
    // Calculate line
    line = mouse_y / line_height;
    
    // Calculate character position
    float relative_x = mouse_x - offset_x;
    character = static_cast<int>(relative_x / char_width);
    
    // Clamp to valid ranges
    OpenedFile& file = Client::get_instance()->get_working_file();
    if (line < 0) line = 0;
    if (line >= file.get_num_lines()) line = file.get_num_lines() - 1;
    if (character < 0) character = 0;
    if (character > file.get_num_characters(line)) character = file.get_num_characters(line);
}

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
		window.style = CS_OWNDC | CS_DBLCLKS; // Added CS_DBLCLKS for double-click support
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
	Client::init();
	Client::get_instance()->open_file("test.txt");
	Config::create();
	CommandController::init(Client::get_instance());
	
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

	Client::get_instance()->begin_autosave();
	
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
		Client::cleanup();
		Config::get_instance()->save();
		Config::destroy();
		std::cerr << "An error occured while running the message loop. Code: " << GetLastError() << "\n";
		return -1;
	}
	delete g;
	Client::cleanup();
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
		Client::get_instance()->process_character(static_cast<char>(wParam));
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
	case WM_LBUTTONDOWN:
	{
		int mouse_x = LOWORD(lParam);
		int mouse_y = HIWORD(lParam);
		
		int line, character;
		MouseToTextPosition(mouse_x, mouse_y, line, character);
		
		OpenedFile& file = Client::get_instance()->get_working_file();
		file.set_current_line(line);
		file.set_current_character(character);
		
		// Start selection if shift is held
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			if (!file.get_selection().has_selection()) {
				file.start_selection();
			}
			file.update_selection();
		} else {
			file.clear_selection();
			file.start_selection();
		}
		
		is_mouse_selecting = true;
		last_mouse_pos.x = mouse_x;
		last_mouse_pos.y = mouse_y;
		SetCapture(hWnd);
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		if (is_mouse_selecting) {
			is_mouse_selecting = false;
			ReleaseCapture();
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (is_mouse_selecting) {
			int mouse_x = LOWORD(lParam);
			int mouse_y = HIWORD(lParam);
			
			int line, character;
			MouseToTextPosition(mouse_x, mouse_y, line, character);
			
			OpenedFile& file = Client::get_instance()->get_working_file();
			file.set_current_line(line);
			file.set_current_character(character);
			file.update_selection();
			
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	}
	case WM_LBUTTONDBLCLK:
	{
		// Double-click to select word
		int mouse_x = LOWORD(lParam);
		int mouse_y = HIWORD(lParam);
		
		int line, character;
		MouseToTextPosition(mouse_x, mouse_y, line, character);
		
		OpenedFile& file = Client::get_instance()->get_working_file();
		file.set_current_line(line);
		file.set_current_character(character);
		
		const std::wstring& line_contents = file.get_current_line_contents();
		
		// Find word boundaries
		int word_start = character;
		int word_end = character;
		
		// Move start to beginning of word
		while (word_start > 0 && line_contents[word_start - 1] != L' ') {
			--word_start;
		}
		
		// Move end to end of word
		while (word_end < static_cast<int>(line_contents.length()) && 
		       line_contents[word_end] != L' ') {
			++word_end;
		}
		
		// Select the word
		file.set_current_character(word_start);
		file.start_selection();
		file.set_current_character(word_end);
		file.update_selection();
		
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_PAINT:
		g->BeginDraw();
		g->ClearScreen(Config::get_instance()->get_background_color());
		Client::get_instance()->draw(g);
		g->EndDraw();
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
}