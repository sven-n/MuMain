
#if !defined(AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_)
#define AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_

#pragma once

#include "UI/Combat/SiegeWarBase.h"

namespace mu::ui::window
{
class CSiegeWarObserver : public CSiegeWarBase
{
public:
    CSiegeWarObserver();
    virtual ~CSiegeWarObserver();

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

#endif // !defined(AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_)
