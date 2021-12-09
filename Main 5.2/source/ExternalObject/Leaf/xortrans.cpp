
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "xortrans.h"

using namespace leaf;

CCyclicXorTrans::CCyclicXorTrans() 
{}
CCyclicXorTrans::~CCyclicXorTrans() 
{}

size_t CCyclicXorTrans::GetKeyTableSize() const 
{ return m_xfKeyTable.size(); }

bool CCyclicXorTrans::LoadKeyTable(const std::string& filename) 
{ return m_xfKeyTable.load(filename); }
bool CCyclicXorTrans::LoadKeyTable(const void* pKey, size_t size) 
{
	UnloadKeyTable();
	return AppendKeyTable(pKey, size);
}
void CCyclicXorTrans::UnloadKeyTable() 
{
	m_xfKeyTable.resize(0);
}

bool CCyclicXorTrans::AppendKeyTable(const std::string& filename) 
{
	xfstreambuf xfAppendTable;
	if(xfAppendTable.load(filename)) {
		m_xfKeyTable.write(xfAppendTable.data(), xfAppendTable.size());
		return true;
	}
	return false;
}
bool CCyclicXorTrans::AppendKeyTable(const void* pKey, size_t size) 
{
	if(size == 0 || IsBadReadPtr(pKey, size))
		return false;
	
	m_xfKeyTable.write(pKey, size);
	return true;
}
void CCyclicXorTrans::JumbleKeyTable() 
{
	if(!m_xfKeyTable.empty()) {
		const BYTE* pbyKeyTable = (const BYTE*)m_xfKeyTable.data();
		size_t size = m_xfKeyTable.size();
		
		xstreambuf xResult;
		int count = 0;

		while(count < (int)size) 
		{
			if(count % 2 == 0) 
			{
				xResult.write(pbyKeyTable + (size-(count/2)-1), 1);
			}
			else {
				xResult.write(pbyKeyTable + (count/2), 1);
			}
			count++;
		}
		m_xfKeyTable = xResult;
	}
}
bool CCyclicXorTrans::SaveKeyTable(const std::string& filename) 
{
	if(m_xfKeyTable.empty())
		return false;
	return m_xfKeyTable.save(filename);
}

bool CCyclicXorTrans::Trans(const void* src, size_t size, void* dest, DWORD dwStreamKey) 
{
	if(m_xfKeyTable.empty())
		return false;
/*	
	BYTE* pbyKeyTable = (BYTE*)m_xfKeyTable.data();
	size_t KeyTableSize = m_xfKeyTable.size();
	
	for(int i=0; i<size; i++) {
		BYTE* _pbySrc = (BYTE*)src;
		BYTE* _pbyDest = (BYTE*)dest;
		_pbyDest[i] = _pbySrc[i] ^ pbyKeyTable[(dwStreamKey+i)%KeyTableSize];
	}*/

	// Head
	DWORD* pdwKeyTable = (DWORD*)m_xfKeyTable.data();
	size_t KeyTableSize = m_xfKeyTable.size()/4;

	DWORD* pdwSrc = (DWORD*)src;
	DWORD* pdwDest = (DWORD*)dest;

	int iHead;

	for(iHead = 0; iHead<(int)size/4; iHead++)
		pdwDest[iHead] = pdwSrc[iHead] ^ pdwKeyTable[(dwStreamKey+iHead)%KeyTableSize];

	// Tail
	BYTE* pbyKeyTable = (BYTE*)m_xfKeyTable.data();
	KeyTableSize = m_xfKeyTable.size();

	BYTE* pbySrc = (BYTE*)src + iHead*4;
	BYTE* pbyDest = (BYTE*)dest + iHead*4;

	int iTail;

	for(iTail = 0; iTail<(int)size%4; iTail++)
		pbyDest[iTail] = pbySrc[iTail] ^ pbyKeyTable[(dwStreamKey+iHead*4+iTail)%KeyTableSize];

	return true;
}
bool CCyclicXorTrans::Trans(const void* src, size_t size, xstreambuf& out, DWORD dwStreamKey) 
{
	if(m_xfKeyTable.empty())
		return false;
/*	
	BYTE* pbyKeyTable = (BYTE*)m_xfKeyTable.data();
	size_t KeyTableSize = m_xfKeyTable.size();
	
	out.resize(size);
	for(int i=0; i<size; i++) {
		BYTE* _pbySrc = (BYTE*)src;
		out << (BYTE)(_pbySrc[i] ^ pbyKeyTable[(dwStreamKey+i)%KeyTableSize]);
	}
	*/

	// Head
	DWORD* pdwKeyTable = (DWORD*)m_xfKeyTable.data();
	size_t KeyTableSize = m_xfKeyTable.size()/4;

	DWORD* pdwSrc = (DWORD*)src;

	int iHead;

	for(iHead = 0; iHead<(int)size/4; iHead++)
		out << (DWORD)(pdwSrc[iHead] ^ pdwKeyTable[(dwStreamKey+iHead)%KeyTableSize]);

	// Tail
	BYTE* pbyKeyTable = (BYTE*)m_xfKeyTable.data();
	KeyTableSize = m_xfKeyTable.size();

	BYTE* pbySrc = (BYTE*)src + iHead*4;

	int iTail;
	for(iTail = 0; iTail<(int)size%4; iTail++)
		out << (BYTE)(pbySrc[iTail] ^ pbyKeyTable[(dwStreamKey+iHead*4+iTail)%KeyTableSize]);

	return true;
}
