
#if !defined(AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_)
#define AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_

#pragma once

#include "UI/Combat/SiegeWarBase.h"

namespace mu::ui::window
{
class CSiegeWarSoldier : public CSiegeWarBase
{
public:
    CSiegeWarSoldier();
    virtual ~CSiegeWarSoldier();

private:
    virtual bool OnCreate();
    virtual bool OnUpdate();
    virtual void OnRelease();

    virtual bool OnUpdateMouseEvent();
    virtual bool OnUpdateKeyEvent();
    virtual bool OnBtnProcess();

    void OnFillRmlModel(SiegeWarfareRmlModel& model) override;
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_)
