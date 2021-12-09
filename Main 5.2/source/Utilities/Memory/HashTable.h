// HashTable.h: interface for the CHashTable class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HASHTABLE_H__BDC457BB_BA46_47A2_83DC_0617C57FF876__INCLUDED_)
#define AFX_HASHTABLE_H__BDC457BB_BA46_47A2_83DC_0617C57FF876__INCLUDED_

#pragma once

#include "./Utilities/Log/ErrorReport.h"


#define HASH_ERROR		( ( DWORD)-1)


template <class T, class S>
class CHashTable  
{
public:
	CHashTable();
	virtual ~CHashTable();
	void Clear( void);

protected:
	T *m_pDataTable;
	S *m_pKeyTable;
	DWORD m_dwTableSize;

public:
	virtual void Create( DWORD dwTableSize);
	virtual void Destroy( void);
	virtual DWORD Hash( S Key);
	BOOL Insert( T Data, S Key);
protected:
	DWORD GetIndex( S Key);
public:
	T Get( S Key);
	T Delete( S Key);
};


template <class T, class S>
CHashTable<T,S>::~CHashTable()
{
	Clear();
}

template <class T, class S>
CHashTable<T,S>::CHashTable()
{
}

template <class T, class S>
void CHashTable<T,S>::Clear( void)
{
	m_dwTableSize = 0;
	m_pDataTable = NULL;
	m_pKeyTable = NULL;
}

template <class T, class S>
void CHashTable<T,S>::Create( DWORD dwTableSize)
{
	m_dwTableSize = dwTableSize;
	m_pDataTable = new T [m_dwTableSize];
	m_pKeyTable = new S [m_dwTableSize];
	ZeroMemory( m_pDataTable, m_dwTableSize * sizeof ( T));
	ZeroMemory( m_pKeyTable, m_dwTableSize * sizeof ( S));
}

template <class T, class S>
void CHashTable<T,S>::Destroy( void)
{
	delete [] m_pKeyTable;
	delete [] m_pDataTable;
	m_dwTableSize = 0;
}

template <class T, class S>
DWORD CHashTable<T,S>::Hash( S Key)
{
	DWORD dwCalc = 0;
	BYTE *pbyCheck = ( BYTE*)&Key;
	for ( int i = 0; i < sizeof ( S); ++i, ++pbyCheck)
	{
		dwCalc = (dwCalc * 131) + *pbyCheck;
	}
	return ( dwCalc % m_dwTableSize);
}

template <class T, class S>
BOOL CHashTable<T,S>::Insert( T Data, S Key)
{
	DWORD dwIndex = Hash( Key);
	S NullKey;
	ZeroMemory( &NullKey, sizeof ( S));
	for ( DWORD i = 0; i < m_dwTableSize; ++i)
	{
		if ( 0 == memcmp( &Key, &m_pKeyTable[dwIndex], sizeof ( S))
			|| 0 == memcmp( &NullKey, &m_pKeyTable[dwIndex], sizeof ( S)))
		{
			m_pDataTable[dwIndex] = Data;
			m_pKeyTable[dwIndex] = Key;
			return ( TRUE);
		}

		dwIndex = ( dwIndex + 1) % m_dwTableSize;
	}

	g_ErrorReport.Write( "Hash table full!!! - Insert\r\n");
	return ( FALSE);
}

template <class T, class S>
DWORD CHashTable<T,S>::GetIndex( S Key)
{
	DWORD dwIndex = Hash( Key);
	S NullKey;
	ZeroMemory( &NullKey, sizeof ( S));
	for ( DWORD i = 0; i < m_dwTableSize; ++i)
	{
		if ( 0 == memcmp( &NullKey, &m_pKeyTable[dwIndex], sizeof ( S)))
		{
			return ( HASH_ERROR);
		}
		if ( 0 == memcmp( &Key, &m_pKeyTable[dwIndex], sizeof ( S)))
		{
			return ( dwIndex);
		}

		dwIndex = ( dwIndex + 1) % m_dwTableSize;
	}

	g_ErrorReport.Write( "Hash table full!!! - GetIndex\r\n");
	return ( HASH_ERROR);
}

template <class T, class S>
T CHashTable<T,S>::Get( S Key)
{
	DWORD dwIndex = GetIndex( Key);
	if ( dwIndex != HASH_ERROR)
	{
		return ( m_pDataTable[dwIndex]);
	}

	GO_DEBUG;
	T NullData;
	ZeroMemory( &NullData, sizeof ( T));
	return ( NullData);
}

template <class T, class S>
T CHashTable<T,S>::Delete( S Key)
{
	DWORD dwIndex = GetIndex( Key);
	if ( dwIndex != HASH_ERROR)
	{
		ZeroMemory( &( m_pKeyTable[dwIndex]), sizeof ( S));
		return ( m_pDataTable[dwIndex]);
	}

	T NullData;
	ZeroMemory( &NullData, sizeof ( T));
	return ( NullData);
}


#endif // !defined(AFX_HASHTABLE_H__BDC457BB_BA46_47A2_83DC_0617C57FF876__INCLUDED_)
