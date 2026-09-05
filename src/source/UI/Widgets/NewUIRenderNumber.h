// NewUIRenderNumber.h: interface for the CNewUIRenderNumber class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIRENDERNUMBER_H__A97FE76D_3AC4_4835_B4B2_61103E6AF831__INCLUDED_)
#define AFX_NEWUIRENDERNUMBER_H__A97FE76D_3AC4_4835_B4B2_61103E6AF831__INCLUDED_

#pragma once

#include "Render/Textures/ZzzTexture.h"

namespace mu::ui::window
{
class CNewUIRenderNumber
{
public:
    enum IMAGE_LIST
    {                                                      //. max: 1개
        IMAGE_NUMBER1 = BITMAP_INTERFACE_NEW_NUMBER_BEGIN, // newui_number1.tga
    };

    virtual ~CNewUIRenderNumber();
    static CNewUIRenderNumber* GetInstance();

    float RenderNumber(float x, float y, int iNum, float fScale = 1.0f);

private:
    CNewUIRenderNumber();

    void LoadImage();
    void ReleaseImage();
};
} // namespace mu::ui::window

#define g_RenderNumber CNewUIRenderNumber::GetInstance()

#endif // !defined(AFX_NEWUIRENDERNUMBER_H__A97FE76D_3AC4_4835_B4B2_61103E6AF831__INCLUDED_)
