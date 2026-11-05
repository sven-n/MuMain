
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
    bool OnCreate() override;
    bool OnUpdate() override;
    void OnRelease() override;

    bool OnUpdateMouseEvent() override;
    bool OnUpdateKeyEvent() override;
    bool OnBtnProcess() override;

    void OnFillRmlModel(SiegeWarfareRmlModel& model) override;
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_)
