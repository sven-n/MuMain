// IInventoryActionContext.h
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IINVENTORYACTIONCONTEXT_H__INCLUDED_)
#define AFX_IINVENTORYACTIONCONTEXT_H__INCLUDED_

#pragma once

#include "_struct.h"

namespace SEASON3B
{

enum REPAIR_MODE
{
    REPAIR_MODE_OFF = 0,
    REPAIR_MODE_ON,
};

class IInventoryActionContext
{
public:
    virtual ~IInventoryActionContext() = default;

    virtual REPAIR_MODE GetRepairMode() const = 0;
    virtual bool IsEquipable(int iIndex, ITEM* pItem) const = 0;
    virtual void ResetMouseRButton() = 0;
    virtual void ResetMouseLButton() = 0;
    virtual int  FindEmptySlot(ITEM* pItem) const = 0;
    virtual bool IsRepairEnableLevel() const = 0;
};

} // namespace SEASON3B

#endif // !defined(AFX_IINVENTORYACTIONCONTEXT_H__INCLUDED_)