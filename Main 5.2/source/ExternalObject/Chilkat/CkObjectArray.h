// CkObjectArray.h: interface for the CkObjectArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CKOBJECTARRAY_H__10278D9D_3CD5_4AC8_A322_CE62A7DF95B6__INCLUDED_)
#define AFX_CKOBJECTARRAY_H__10278D9D_3CD5_4AC8_A322_CE62A7DF95B6__INCLUDED_

#pragma once

#include "CkString.h"
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkObjectArray
class CkObjectArray  : public CkObject
{
    private:
	void *m_impl;

	// Don't allow implicit copying.
	CkObjectArray(const CkObjectArray &a) { }


    public:
	CkObjectArray();
	virtual ~CkObjectArray();

	// BEGIN PUBLIC INTERFACE

	CkObject *GetAt(int index) const;
	CkObject *GetLast(void) const;

	void InsertAt(int index, CkObject *obj);
	void RemoveAt(int index);

	// Clear the array and delete all objects contained within.
	void DeleteAllObjects(void);

	// Clear the array without deleting the objects.
	void ClearWithoutDeleting(void);

	void Append(CkObject *obj);
	CkObject *Pop(void);  

	// Return the number of objects in the array.
	int get_Size(void) const;

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE

};
#pragma pack (pop)

#endif // !defined(AFX_CKOBJECTARRAY_H__10278D9D_3CD5_4AC8_A322_CE62A7DF95B6__INCLUDED_)
