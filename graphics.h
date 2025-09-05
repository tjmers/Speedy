#pragma once

#include <windows.h>
#include <d2d1.h>

#include <iostream>
#include <string>
constexpr int STARTING_SCREEN_WIDTH = 1080;
constexpr int STARTING_SCREEN_HEIGHT = 720;
#include <dwrite.h>

class Graphics
{
    ID2D1Factory* factory;
    ID2D1HwndRenderTarget* render_target;
    ID2D1SolidColorBrush* brush;
    IDWriteFactory* w_factory;
    IDWriteTextFormat* text_format;
public:
    Graphics();
    ~Graphics();

    bool Init(HWND hWnd);
    void Resize(const short client_width, const short client_height);

    void BeginDraw();
    void EndDraw();

    void SetColor(const D2D1::ColorF& color);
    void ClearScreen(const D2D1::ColorF& color);
    void DrawLine(const float x1, const float y1, const float x2, const float y2, const float stroke_width);
    void DrawRect(float x, float y, float width, float height);
    void DrawRect(float x, float y, float width, float height, float stroke_width);
    void FillRect(float x, float y, float width, float height);
    void FillRect(const D2D1_RECT_F& rect);
    void DrawString(const wchar_t* string, int str_len, float x, float y, float width, float height);
};