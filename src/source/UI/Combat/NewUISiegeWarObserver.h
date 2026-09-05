// NewUISiegeWarObserver.h: interface for the CSiegeWarObserver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_)
#define AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_

#pragma once

#include "UI/Combat/NewUISiegeWarBase.h"

namespace mu::ui::window
{
class CSiegeWarObserver : public CSiegeWarBase
{
public:
    CSiegeWarObserver();
    virtual ~CSiegeWarObserver();

private:
    virtual bool OnCreate(int x, int y);
    virtual bool OnUpdate();
    virtual bool OnRender();
    virtual void OnRelease();

    virtual bool OnUpdateMouseEvent();
    virtual bool OnUpdateKeyEvent();
    virtual bool OnBtnProcess();
    virtual void OnSetPos(int x, int y);

    virtual void OnLoadImages();
    virtual void OnUnloadImages();

    void RenderCharPosInMiniMap();
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_)
