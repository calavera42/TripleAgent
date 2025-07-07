#include "SpeechBalloonRenderer.h"

using namespace Gdiplus;

void SpeechBalloonRenderer::Setup(CharacterInfo& ci)
{
    GdiplusStartupInput gdiStart = {};
    GdiplusStartup(&GdiToken, &gdiStart, nullptr);

    Style = ci.BalloonInfo;
    CharFlags = ci.Flags;

    // nem vale a pena tentar carregar as fontes dos agentes, a maioria nem existe mais
    AgFntFamily = new FontFamily(L"Arial");
    AgFont = new Font(AgFntFamily, 15, FontStyleRegular, UnitPixel);

    int lineSpacing = 
        AgFont->GetSize()
        * AgFntFamily->GetLineSpacing(FontStyleRegular)
        / AgFntFamily->GetEmHeight(FontStyleRegular);

    float avgChar = GetAverageCharWidth(AgFont);

    float maxWidth = (float)(avgChar * Style.CharsPerLine);
    float maxHeight = (float)(lineSpacing * Style.TextLines);

    MaxSize = {
        maxWidth,
        ((int)CharFlags & (int)CharacterFlags::BalloonSizeToText) != 0 ? 0 : maxHeight
    };

    TextBrush = new SolidBrush(
        Color(
            Style.ForegroundColor.Red,
            Style.ForegroundColor.Green,
            Style.ForegroundColor.Blue
        )
    );

    BackBrush = new SolidBrush(
        Color(
            Style.BackgroundColor.Red,
            Style.BackgroundColor.Green,
            Style.BackgroundColor.Blue
        )
    );

    BorderPen = new Pen(
        Color(
            Style.BorderColor.Red,
            Style.BorderColor.Green,
            Style.BorderColor.Blue
        ),
        1.0f
    );
}

void SpeechBalloonRenderer::Paint(HWND hwnd, BalloonRenderInfo bri)
{
    SizeF textSize = GetTextSize(bri.Text, MaxSize);
    StringFormat sf;
    Rect bodyBounds = {
        0, 
        0,
        (int)ceil(textSize.Width) + SBCornerSpacing * 2, 
        (int)ceil(textSize.Height) + SBCornerSpacing * 2
    };
    Point lookupSkews[4] = { // usado para compensar pela ponta do balão de fala
        { 0, SBTipDepth }, // cima
        { 0, 0 }, // direita
        { 0, 0 }, // baixo
        { SBTipDepth, 0 }, // esquerda
    };

    Point skew = lookupSkews[(int)bri.TipQuad];
    bodyBounds.X += skew.X;
    bodyBounds.Y += skew.Y;

    HDC hdc = GetDC(hwnd);
    HDC screenDC = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    SIZE wndSize = {
        bodyBounds.GetRight() + 1, // +1 pq a borda é desenhada pra fora do corpo do balão
        bodyBounds.GetBottom() + 1
    };

    if (bri.TipQuad == TipQuadrant::Right)
        wndSize.cx += SBTipDepth;
    else if (bri.TipQuad == TipQuadrant::Bottom)
        wndSize.cy += SBTipDepth;

    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, wndSize.cx, wndSize.cy);

    SelectObject(memDC, memBitmap);

    Graphics g(memDC);

    g.Clear(Color(0, 0, 0, 0));

    POINT ptSrc = { 0, 0 };

    BLENDFUNCTION blendFunction = {};
    blendFunction.AlphaFormat = AC_SRC_ALPHA;
    blendFunction.BlendFlags = 0;
    blendFunction.BlendOp = AC_SRC_OVER;
    blendFunction.SourceConstantAlpha = 255;

    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    // renderização começa aq

    sf.SetAlignment(StringAlignmentNear);

    DrawShape(&g, bodyBounds, bri.TipOffsetInLine, bri.TipQuad);

    g.DrawString(
        bri.Text.c_str(), 
        bri.SpeechProgress,
        AgFont, 
        RectF
        {
            (float)bodyBounds.X + SBCornerSpacing,
            (float)bodyBounds.Y + SBCornerSpacing,
            MaxSize.Width, 
            MaxSize.Height 
        }, 
        &sf, 
        TextBrush
    );

    // renderização termina aq

    SetWindowPos(
        hwnd,
        nullptr,
        0,
        0,
        wndSize.cx,
        wndSize.cy,
        SWP_NOMOVE |
        SWP_NOZORDER
    );

    UpdateLayeredWindow(
        hwnd, 
        screenDC, 
        nullptr, 
        &wndSize,
        memDC, 
        &ptSrc, 
        0, 
        &blendFunction, 
        ULW_ALPHA
    );

    DeleteDC(memDC);
    DeleteObject(memBitmap);
    ReleaseDC(hwnd, hdc);
}

void SpeechBalloonRenderer::DrawShape(Gdiplus::Graphics* g, Gdiplus::Rect bodyBounds, int tipOffsetInLine, TipQuadrant tq)
{
    GraphicsPath gp(FillModeWinding);
    int tipMiddle = tq == TipQuadrant::Top || tq == TipQuadrant::Bottom ? SBTipMiddle : 0;

    tipOffsetInLine -= tipMiddle + SBCornerSpacing;
    tipOffsetInLine = max(tipOffsetInLine, 0);

    Rect arcRects[4] = {
        { bodyBounds.X, bodyBounds.Y, SBCornerDiameter, SBCornerDiameter },
        { bodyBounds.GetRight() - SBCornerDiameter, bodyBounds.Y, SBCornerDiameter, SBCornerDiameter },
        { bodyBounds.GetRight() - SBCornerDiameter, bodyBounds.GetBottom() - SBCornerDiameter, SBCornerDiameter, SBCornerDiameter },
        { bodyBounds.X, bodyBounds.GetBottom() - SBCornerDiameter, SBCornerDiameter, SBCornerDiameter }
    };

    Point tipPoints[4][3] = {
        // CIMA
        {
            { bodyBounds.X + SBCornerSpacing + tipOffsetInLine, bodyBounds.Y },
            { bodyBounds.X + SBCornerSpacing + tipOffsetInLine + tipMiddle, bodyBounds.Y - SBTipDepth },
            { bodyBounds.X + SBCornerSpacing + tipOffsetInLine + SBTipSpacing, bodyBounds.Y }
        },
        // DIREITA
        {
            { bodyBounds.GetRight(), bodyBounds.Y + SBCornerSpacing + tipOffsetInLine },
            { bodyBounds.GetRight() + SBTipDepth, bodyBounds.Y + SBCornerSpacing + tipOffsetInLine + tipMiddle },
            { bodyBounds.GetRight(), bodyBounds.Y + SBCornerSpacing + tipOffsetInLine + SBTipSpacing },
        },
        // BAIXO
        {
            { bodyBounds.X + SBCornerSpacing + tipOffsetInLine + SBTipSpacing, bodyBounds.GetBottom() },
            { bodyBounds.X + SBCornerSpacing + tipOffsetInLine + tipMiddle, bodyBounds.GetBottom() + SBTipDepth },
            { bodyBounds.X + SBCornerSpacing + tipOffsetInLine, bodyBounds.GetBottom() },
        },
        // ESQUERDA
        {
            { bodyBounds.X, bodyBounds.Y + SBCornerSpacing + tipOffsetInLine + SBTipSpacing },
            { bodyBounds.X - SBTipDepth, bodyBounds.Y + SBCornerSpacing + tipOffsetInLine + tipMiddle },
            { bodyBounds.X, bodyBounds.Y + SBCornerSpacing + tipOffsetInLine },
        },
    };

    int sAngle = 180;
    for (size_t i = 0; i < 4; i++)
    {
        gp.AddArc(arcRects[i], sAngle, 90);

        if ((int)tq == i) // a ponta está nessa aresta
        {
            auto points = tipPoints[i];

            gp.AddLine(points[0], points[1]);
            gp.AddLine(points[1], points[2]);
        }

        sAngle += 90;
    }

    gp.CloseFigure();

    g->FillPath(BackBrush, &gp);
    g->DrawPath(BorderPen, &gp);
}

SizeF SpeechBalloonRenderer::GetTextSize(string text, SizeF maxSize) const
{
    Bitmap b(1, 1);
    Graphics g(&b);

    SizeF outp = {};

    g.MeasureString(text.c_str(), -1, AgFont, maxSize, nullptr, &outp);

    return outp;
}

float SpeechBalloonRenderer::GetAverageCharWidth(const Font* f) const
{
    // deus q me perdoe
    Bitmap b(1, 1);
    Graphics g(&b);

    const WCHAR* testString = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int numChars = wcslen(testString);

    RectF layoutRect;
    g.MeasureString(testString, numChars, AgFont, { 0, 0 }, &layoutRect);

    return layoutRect.Width / numChars;
}

SpeechBalloonRenderer::~SpeechBalloonRenderer()
{
    delete AgFntFamily;
    delete AgFont;

    delete TextBrush;
    delete BackBrush;
    delete BorderPen;

    GdiplusShutdown(GdiToken);
}

Rect SpeechBalloonRenderer::GetSize(TipQuadrant tq, string text)
{
    Rect r = { 0, 0, SBCornerSpacing * 2, SBCornerSpacing * 2 };

    SizeF size = GetTextSize(text, MaxSize);

    r.Width += size.Width;
    r.Height += size.Height;

    if (tq == TipQuadrant::Top || tq == TipQuadrant::Bottom)
        r.Height += SBTipDepth;
    else
        r.Width += SBTipDepth;

    return r;
}
