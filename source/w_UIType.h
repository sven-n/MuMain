#pragma once

namespace ui
{
	enum eEventType
	{
		eEvent_None,
		eEvent_LButtonUp,
		eEvent_LButtonDown,
		eEvent_LButtonClick,
		eEvent_LButtonDbClk,

		eEvent_RButtonUp,
		eEvent_RButtonDown,
		eEvent_RButtonClick,
		eEvent_RButtonDbClk,

		eEvent_Focus,
		eEvent_KillFocus,

		eEvent_Tab,
		eEvent_Enter,
		eEvent_Escape,

		eEvent_Move,
		eEvent_Resize,

		eEvent_MouseOver,
		eEvent_MouseOut,

		eEvent_Scroll,

		eEvent_IMEEscape,
		eEvent_IMEEnter,

		eEvent_Select,
		eEvent_Deselect,

		eEvent_Check    = eEvent_Select,
		eEvent_Uncheck  = eEvent_Deselect,

		eEventTypeMax,
		eEventTypeCount = eEventTypeMax - 1,
	};

	enum ePanelType
    {
        ePanel_Piece_Full,					//9장의 이미지로 하나의 그림을 만든다.
		ePanel_Piece_Width_Three,           //3장의 이미지로 하나의 그림을 만든다(가로).
		ePanel_Piece_Height_Three,			//3장의 이미지로 하나의 그림을 만든다(세로).
		ePanel_Piece_One,					//1장의 이미지로 하나의 그림을 만든다.

		ePanel_Piece_Count,
    };
	
	enum eDrawType
	{
		eDraw_None = 0,

		eDraw_Image,
		eDraw_Mesh,
		eDraw_ViewPortMesh,
		eDraw_Rect,

		eDraw_Count,
	};

	enum eUIEffectType
	{
		eEffect_None = 0,
		eEffect_Slowly,
		eEffect_Move,

		eEffect_Count,
	};
};