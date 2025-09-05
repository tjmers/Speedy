#include "graphics.h"

#include "config.h"


Graphics::Graphics()
    : factory(NULL), render_target(NULL), brush(NULL), w_factory(NULL), text_format(nullptr) {}


Graphics::~Graphics()
{
    if (factory) factory->Release();
    if (render_target) render_target->Release();
    if (brush) brush->Release();
    if (w_factory) w_factory->Release();
    if (text_format) text_format->Release();
}

bool Graphics::Init(HWND hWnd)
{
    HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
    if (res != S_OK) return false;

    RECT client_rect;
    GetClientRect(hWnd, &client_rect);

    res = factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            hWnd,
            D2D1::SizeU(client_rect.right, client_rect.bottom)
        ),
        &render_target
    );

    if (res != S_OK) return false;

    res = render_target->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0), &brush);
    if (res != S_OK) return false;

    res = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(w_factory),
        reinterpret_cast<IUnknown**>(&w_factory));

    if (res != S_OK) return false;

    std::string font_family = Config::get_instance()->get_font_family();
    // Convert to std::wstring
    std::wstring w_font_family(font_family.begin(), font_family.end());
    static const wchar_t* msc_fontName = w_font_family.c_str();
    static FLOAT msc_fontSize = Config::get_instance()->get_font_size();

    res = w_factory->CreateTextFormat(
        msc_fontName,
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        msc_fontSize,
        L"",
        &text_format
    );

    if (res != S_OK) return false;

    return true;

}



void Graphics::Resize(const short client_width, const short client_height)
{
    render_target->Resize(D2D1::SizeU(client_width, client_height));
}


void Graphics::BeginDraw() { render_target->BeginDraw(); }
void Graphics::EndDraw() { render_target->EndDraw(); }


void Graphics::SetColor(const D2D1::ColorF& color) { brush->SetColor(color); }

void Graphics::ClearScreen(const D2D1::ColorF& color)
{
    render_target->Clear(color);
}




void Graphics::DrawLine(const float x1, const float y1, const float x2, const float y2, const float stroke_width)
{
    render_target->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), brush, stroke_width);
}


void Graphics::DrawRect(float x, float y, float width, float height)
{
    render_target->DrawRectangle(D2D1::RectF(x, y, x + width, y + height), brush);
}
void Graphics::DrawRect(float x, float y, float width, float height, float stroke_width)
{
    render_target->DrawRectangle(D2D1::RectF(x, y, x + width, y + height), brush, stroke_width);
}

void Graphics::FillRect(float x1, float y1, float x2, float y2)
{
    render_target->FillRectangle(D2D1::RectF(x1, y1, x2, y2), brush);
}

void Graphics::FillRect(const D2D1_RECT_F& rect)
{
    render_target->FillRectangle(rect, brush);
}


void Graphics::DrawString(const wchar_t* str, int str_len, float x, float y, float width, float height)
{
    render_target->DrawTextA(str, str_len, text_format, D2D1::RectF(x, y, x + width, y + height), brush);
}