// NewUIRenderNumber.cpp: implementation of the CNewUIRenderNumber class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UI/Widgets/NewUIRenderNumber.h"	// self

mu::ui::window::CNewUIRenderNumber* mu::ui::window::CNewUIRenderNumber::GetInstance()
{
    static CNewUIRenderNumber s_NewUIRenderNumber;
    return &s_NewUIRenderNumber;
}

mu::ui::window::CNewUIRenderNumber::CNewUIRenderNumber()
{
    LoadImage();
}

mu::ui::window::CNewUIRenderNumber::~CNewUIRenderNumber()
{
    ReleaseImage();
}

void mu::ui::window::CNewUIRenderNumber::LoadImage()
{
    LoadBitmap(L"Interface\\newui_number1.tga", IMAGE_NUMBER1, GL_LINEAR);
}

void mu::ui::window::CNewUIRenderNumber::ReleaseImage()
{
    DeleteBitmap(IMAGE_NUMBER1);
}

float mu::ui::window::CNewUIRenderNumber::RenderNumber(float x, float y, int iNum, float fScale)
{
    EnableAlphaTest();

    float width, height;

    if (fScale < 0.3f)
    {
        return x;
    }

    width = 12.f * (fScale - 0.3f);
    height = 16.f * (fScale - 0.3f);

    wchar_t strText[32];
    _itow(iNum, strText, 10);
    int iLength = (int)wcslen(strText);

    x -= width * iLength / 2;

    for (int i = 0; i < iLength; ++i)
    {
        float fU = (float)(strText[i] - 48) * 12.f / 128.f;
        RenderBitmap(IMAGE_NUMBER1, x, y, width, height, fU, 0.f, 12.f / 128.f, 14.f / 16.f);
        x += width * 0.8f;
    }

    return x;
}