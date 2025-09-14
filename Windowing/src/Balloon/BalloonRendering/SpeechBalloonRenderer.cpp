#include "SpeechBalloonRenderer.h"

using namespace Gdiplus;

// TODO: dar a opção de modificar o tamanho da fonte
void SpeechBalloonRenderer::Setup(CharacterInfo& ci)
{
    Style = ci.BalloonInfo;
    CharFlags = ci.Flags;

    int fontSize = -MulDiv(Style.FontHeight, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);

    // nem vale a pena tentar carregar as fontes dos agentes, a maioria nem existe mais
    AgFntFamily = new FontFamily(L"arial");
    AgFont = new Font(AgFntFamily, fontSize, FontStyleRegular, UnitPixel);

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

    // TODO: a minha indecisão me mata, se for fazer o mínimo do balão seguir a especificação lembrar de mudar aq e no getsize.
    Rect bodyBounds = {
        0, 
        0,
        (int)round(textSize.Width) + SBCornerSpacingX * 2,
        (int)round(textSize.Height) + SBCornerSpacingY * 2
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
    g.SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit);

    // renderização começa aq

    sf.SetAlignment(StringAlignmentNear);

    DrawShape(&g, bodyBounds, bri.TipOffsetInLine, bri.TipQuad);

    Pen p = Pen(Color(0, 255, 0), 1.0f);

    int stts = g.DrawString(
        bri.Text.c_str(), 
        bri.SpeechProgress,
        AgFont, 
        RectF
        {
            (float)bodyBounds.X + SBCornerSpacingX,
            (float)bodyBounds.Y + SBCornerSpacingY,
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

    // default pro TipQuadrant::Left e TipQuadrant::Right
    int tipMiddle = 0;
    int tipEdgeSize = bodyBounds.Height;
    int cornerSpacing = SBCornerSpacingY;

    if (tq == TipQuadrant::Top || tq == TipQuadrant::Bottom) 
    {
        tipMiddle = SBTipMiddle;
        tipEdgeSize = bodyBounds.Width;
        cornerSpacing = SBCornerSpacingX;
    }

    tipOffsetInLine -= SBTipSpacing + SBTipMiddle + cornerSpacing;
    tipOffsetInLine = max(tipOffsetInLine, 0);
    tipOffsetInLine = min(tipOffsetInLine + cornerSpacing, tipEdgeSize - cornerSpacing * 2 - SBTipSpacing);

    Rect arcRects[4] = {
        { bodyBounds.X, bodyBounds.Y, SBCornerDiameterX, SBCornerDiameterY },
        { bodyBounds.GetRight() - SBCornerDiameterX, bodyBounds.Y, SBCornerDiameterX, SBCornerDiameterY },
        { bodyBounds.GetRight() - SBCornerDiameterX, bodyBounds.GetBottom() - SBCornerDiameterY, SBCornerDiameterX, SBCornerDiameterY },
        { bodyBounds.X, bodyBounds.GetBottom() - SBCornerDiameterY, SBCornerDiameterX, SBCornerDiameterY }
    };

    Point tipPoints[4][3] = {
        // CIMA
        {
            { bodyBounds.X + SBCornerSpacingX + tipOffsetInLine, bodyBounds.Y },
            { bodyBounds.X + SBCornerSpacingX + tipOffsetInLine + tipMiddle, bodyBounds.Y - SBTipDepth },
            { bodyBounds.X + SBCornerSpacingX + tipOffsetInLine + SBTipSpacing, bodyBounds.Y }
        },
        // DIREITA
        {
            { bodyBounds.GetRight(), bodyBounds.Y + SBCornerSpacingY + tipOffsetInLine },
            { bodyBounds.GetRight() + SBTipDepth, bodyBounds.Y + SBCornerSpacingY + tipOffsetInLine + tipMiddle },
            { bodyBounds.GetRight(), bodyBounds.Y + SBCornerSpacingY + tipOffsetInLine + SBTipSpacing },
        },
        // BAIXO
        {
            { bodyBounds.X + SBCornerSpacingX + tipOffsetInLine + SBTipSpacing, bodyBounds.GetBottom() },
            { bodyBounds.X + SBCornerSpacingX + tipOffsetInLine + tipMiddle, bodyBounds.GetBottom() + SBTipDepth },
            { bodyBounds.X + SBCornerSpacingX + tipOffsetInLine, bodyBounds.GetBottom() },
        },
        // ESQUERDA
        {
            { bodyBounds.X, bodyBounds.Y + SBCornerSpacingY + tipOffsetInLine + SBTipSpacing },
            { bodyBounds.X - SBTipDepth, bodyBounds.Y + SBCornerSpacingY + tipOffsetInLine + tipMiddle },
            { bodyBounds.X, bodyBounds.Y + SBCornerSpacingY + tipOffsetInLine },
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

    g.MeasureString(text.c_str(), -1, AgFont, maxSize, StringFormat::GenericTypographic(), &outp);

    return outp;
}

float SpeechBalloonRenderer::GetAverageCharWidth(const Font* f) const
{
    // deus q me perdoe
    Bitmap b(1, 1);
    Graphics g(&b);

    const WCHAR* testString = L"abcdefghijklmnopqrstuvwxyz";
    int numChars = wcslen(testString);

    RectF layoutRect;
    g.MeasureString(testString, numChars, AgFont, SizeF{ 0, 0 }, StringFormat::GenericTypographic(), &layoutRect);

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
    Rect r = { 0, 0, SBCornerSpacingX * 2, SBCornerSpacingY * 2 };

    SizeF size = GetTextSize(text, MaxSize);

    r.Width += size.Width;
    r.Height += size.Height;

    if (tq == TipQuadrant::Top || tq == TipQuadrant::Bottom)
        r.Height += SBTipDepth;
    else
        r.Width += SBTipDepth;

    return r;
}
