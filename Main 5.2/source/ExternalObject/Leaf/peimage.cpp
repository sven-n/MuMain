
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "peimage.h"

#include "xstreambuf.h"
#include "stdleaf.h"
#include "xortrans.h"

#include <assert.h>

/* Image Format */

PIMAGE_DOS_HEADER leaf::GetPtrOfImageDosHeader(void* pImage)
{
	if(IsBadReadPtr(pImage, sizeof(IMAGE_DOS_HEADER)))
		return NULL;

	PIMAGE_DOS_HEADER _pImageDosHeader = (PIMAGE_DOS_HEADER)pImage;
	if(_pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	return _pImageDosHeader;
}
PCIMAGE_DOS_HEADER leaf::GetPtrOfImageDosHeader(const void* pImage)
{ return GetPtrOfImageDosHeader(const_cast<void*>(pImage)); }

PIMAGE_NT_HEADERS leaf::GetPtrOfImageNtHeaders(void* pImage)
{
	PIMAGE_DOS_HEADER pImageDosHeader = GetPtrOfImageDosHeader(pImage);
	if(NULL == pImageDosHeader)
		return NULL;

	PIMAGE_NT_HEADERS _pImageNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pImage+pImageDosHeader->e_lfanew);
	if(IsBadReadPtr(_pImageNtHeaders, sizeof(IMAGE_NT_HEADERS)))
		return NULL;
	if(_pImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return _pImageNtHeaders;
}
PCIMAGE_NT_HEADERS leaf::GetPtrOfImageNtHeaders(const void* pImage)
{ return GetPtrOfImageNtHeaders(const_cast<void*>(pImage)); }

PIMAGE_SECTION_HEADER leaf::GetPtrOfImageSectionHeader(void* pImage, int index)
{
	PIMAGE_NT_HEADERS pImageNtHeaders = GetPtrOfImageNtHeaders(pImage);
	if(NULL == pImageNtHeaders)
		return NULL;

	if(index < 0 || index >= pImageNtHeaders->FileHeader.NumberOfSections)
		return NULL;
	
	PIMAGE_SECTION_HEADER _pSectionHeader = IMAGE_FIRST_SECTION(pImageNtHeaders) + index;
	if(IsBadReadPtr(_pSectionHeader, sizeof(IMAGE_SECTION_HEADER)))
		return false;

	return _pSectionHeader;
}
PCIMAGE_SECTION_HEADER leaf::GetPtrOfImageSectionHeader(const void* pImage, int index)
{ return GetPtrOfImageSectionHeader(const_cast<void*>(pImage), index); }

PIMAGE_SECTION_HEADER leaf::GetPtrOfImageSectionHeader(void* pImage, const std::string& name)
{
	PIMAGE_NT_HEADERS pImageNtHeaders = GetPtrOfImageNtHeaders(pImage);
	if(NULL == pImageNtHeaders)
		return NULL;

	for(int i=0; i<pImageNtHeaders->FileHeader.NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER _pSectionHeader = IMAGE_FIRST_SECTION(pImageNtHeaders) + i;
		if(IsBadReadPtr(_pSectionHeader, sizeof(IMAGE_SECTION_HEADER)))
			return NULL;
		if(0 == strcmp((const char*)_pSectionHeader->Name, name.c_str()))
		{
			return _pSectionHeader;
		}
	}
	return NULL;
}
PCIMAGE_SECTION_HEADER leaf::GetPtrOfImageSectionHeader(const void* pImage, const std::string& name)
{ return GetPtrOfImageSectionHeader(const_cast<void*>(pImage), name); }

PIMAGE_DATA_DIRECTORY leaf::GetPtrOfDataDirectory(void* pImage, int entry)
{
	PIMAGE_NT_HEADERS pImageNtHeaders = GetPtrOfImageNtHeaders(pImage);
	if(NULL == pImageNtHeaders)
		return NULL;

	if(entry < 0 || entry >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
		return false;
	
	return pImageNtHeaders->OptionalHeader.DataDirectory + entry;
}
PCIMAGE_DATA_DIRECTORY leaf::GetPtrOfDataDirectory(const void* pImage, int entry)
{ return GetPtrOfDataDirectory(const_cast<void*>(pImage), entry); }

PIMAGE_SECTION_HEADER leaf::FindEnclosingSectionHeaderFromRVA(void* pImage, DWORD dwRVA)
{
	return NULL;
}
PCIMAGE_SECTION_HEADER leaf::FindEnclosingSectionHeaderFromRVA(const void* pImage, DWORD dwRVA)
{ return FindEnclosingSectionHeaderFromRVA(const_cast<void*>(pImage), dwRVA); }

LPVOID leaf::GetPtrDirectoryPtrFromRVA(void* pImage, DWORD dwRVA)
{
	return NULL;
}

bool leaf::GetImageDosHeader(const void* pImage, OUT PIMAGE_DOS_HEADER pImageDosHeader)
{
	PCIMAGE_DOS_HEADER _pImageDosHeader = GetPtrOfImageDosHeader(pImage);
	if(NULL == _pImageDosHeader)
		return false;

	*pImageDosHeader = *_pImageDosHeader;

	return true;
}
bool leaf::GetImageNtHeaders(const void* pImage, OUT PIMAGE_NT_HEADERS pImageNtHeaders)
{
	PCIMAGE_NT_HEADERS _pImageNtHeaders = GetPtrOfImageNtHeaders(pImage);
	if(NULL == _pImageNtHeaders)
		return false;

	*pImageNtHeaders = *_pImageNtHeaders;

	return true;
}
bool leaf::GetImageSectionHeader(const void* pImage, int index, OUT PIMAGE_SECTION_HEADER pImageSectionHeader)
{
	PCIMAGE_SECTION_HEADER _pImageSectionHeader = GetPtrOfImageSectionHeader(pImage, index);
	if(NULL == _pImageSectionHeader)
		return false;

	*pImageSectionHeader = *_pImageSectionHeader;

	return true;
}
bool leaf::GetImageSectionHeader(const void* pImage, const std::string& name, OUT PIMAGE_SECTION_HEADER pImageSectionHeader)
{
	PCIMAGE_SECTION_HEADER _pImageSectionHeader = GetPtrOfImageSectionHeader(pImage, name);
	if(NULL == _pImageSectionHeader)
		return false;

	*pImageSectionHeader = *_pImageSectionHeader;

	return true;
}
bool leaf::GetImageDataDirectory(const void* pImage, int entry, OUT PIMAGE_DATA_DIRECTORY pImageDataDirectory)
{
	PCIMAGE_DATA_DIRECTORY _pDataDirectory = GetPtrOfDataDirectory(pImage, entry);
	if(NULL == _pDataDirectory)
		return false;

	*pImageDataDirectory = *_pDataDirectory;

	return true;
}


/* Data Injection into Portable Executable File */

using namespace leaf;

PPE_INJECTION_PACK_HEADER leaf::GetPtrOfInjectionPackHeader(void* pImage)
{
	PIMAGE_DOS_HEADER pImageDosHeader = GetPtrOfImageDosHeader(pImage);
	if(NULL == pImageDosHeader)
		return NULL;

	PIMAGE_NT_HEADERS pImageNtHeaders = GetPtrOfImageNtHeaders(pImage);
	if(NULL == pImageNtHeaders)
		return NULL;

	DWORD dwBaseOfIPH = pImageDosHeader->e_lfanew /* DOS Headers */
		+ sizeof(IMAGE_NT_HEADERS) /* NT Headers */
		+ (sizeof(IMAGE_SECTION_HEADER)*pImageNtHeaders->FileHeader.NumberOfSections); /* Section Headers */

	PPE_INJECTION_PACK_HEADER _pIPH = (PPE_INJECTION_PACK_HEADER)((BYTE*)pImage + dwBaseOfIPH);
	if(_pIPH->dwSignature != PE_INJECTION_PACK_SIGNATURE)	//. PIPH
		return NULL;

	return _pIPH;
}
PCPE_INJECTION_PACK_HEADER leaf::GetPtrOfInjectionPackHeader(const void* pImage)
{ return GetPtrOfInjectionPackHeader(const_cast<void*>(pImage)); }

PPE_INJECTION_DATA_HEADER leaf::GetPtrOfInjectionDataHeader(void* pImage, int index)
{
	PPE_INJECTION_PACK_HEADER _pIPH = GetPtrOfInjectionPackHeader(pImage);
	if(NULL == _pIPH)
		return NULL;

	BYTE* pbyIPD = (BYTE*)pImage + _pIPH->dwBaseOfIPD;
	
	PPE_INJECTION_DATA_HEADER _pNextIDH = (PPE_INJECTION_DATA_HEADER)pbyIPD;

	for(int i = 0; i < (int)_pIPH->dwNumberOfIPD; i++)
	{
		PPE_INJECTION_DATA_HEADER _pCurIDH = _pNextIDH;
		if(IsBadReadPtr(_pCurIDH, sizeof(PE_INJECTION_DATA_HEADER)))
			return NULL;

		if(i==index)
		{
			if(_pCurIDH->dwSignature == PE_INJECTION_DATA_SIGNATURE)
			{
				return _pCurIDH;
			}
			break;
		}
		_pNextIDH = (PPE_INJECTION_DATA_HEADER)((BYTE*)_pCurIDH + sizeof(PE_INJECTION_DATA_HEADER) + (_pCurIDH->wSizeOfDataName + _pCurIDH->dwSizeOfData));
	}
	return NULL;
}
PCPE_INJECTION_DATA_HEADER leaf::GetPtrOfInjectionDataHeader(const void* pImage, int index)
{ return GetPtrOfInjectionDataHeader(const_cast<void*>(pImage), index); }

PPE_INJECTION_DATA_HEADER leaf::GetPtrOfInjectionDataHeader(void* pImage, const std::string& name, OUT int* piIndex)
{
	PPE_INJECTION_PACK_HEADER _pIPH = GetPtrOfInjectionPackHeader(pImage);
	if(NULL == _pIPH)
		return NULL;

	PCIMAGE_SECTION_HEADER pOrigCodeSectionHeader = GetPtrOfImageSectionHeader(pImage, ".text");
	if(NULL == pOrigCodeSectionHeader)
		return false;

	CCyclicXorTrans XorTrans;	//. Section Alignment: 0x1000
	if(false == XorTrans.LoadKeyTable((PBYTE)pImage+pOrigCodeSectionHeader->PointerToRawData, 0x200))
		return false;

	BYTE* pbyIPD = (BYTE*)pImage+_pIPH->dwBaseOfIPD;
	PPE_INJECTION_DATA_HEADER _pNextIDH = (PPE_INJECTION_DATA_HEADER)pbyIPD;
	for(int i = 0; i < (int)_pIPH->dwNumberOfIPD; i++)
	{
		PPE_INJECTION_DATA_HEADER _pCurIDH = _pNextIDH;
		if(IsBadReadPtr(_pCurIDH, sizeof(PE_INJECTION_DATA_HEADER)))
			return NULL;

		//. decoding
		xstreambuf xDecodedName;
		DWORD dwSeed = ((PBYTE)pImage+pOrigCodeSectionHeader->PointerToRawData)[0x200+i];
		XorTrans.Trans((PBYTE)pImage+_pCurIDH->dwPointerToDataName, _pCurIDH->wSizeOfDataName, xDecodedName, dwSeed);

		std::string strName = (const char*)xDecodedName.data();
		if(strName == name)
		{
			if(_pCurIDH->dwSignature == PE_INJECTION_DATA_SIGNATURE)	//. PIDH
			{
				if(piIndex)
					*piIndex = i;
				return _pCurIDH;
			}
			break;
		}
		//. move to next header
		_pNextIDH = (PPE_INJECTION_DATA_HEADER)((BYTE*)_pCurIDH + sizeof(PE_INJECTION_DATA_HEADER) + 
			(_pCurIDH->wSizeOfDataName + _pCurIDH->dwSizeOfData));
	}
	return NULL;
}
PCPE_INJECTION_DATA_HEADER leaf::GetPtrOfInjectionDataHeader(const void* pImage, const std::string& name, OUT int* piIndex)
{ return GetPtrOfInjectionDataHeader(const_cast<void*>(pImage), name, piIndex); }

bool leaf::GetInjectionPackHeader(const void* pImage, OUT PPE_INJECTION_PACK_HEADER pIPH)
{
	PCPE_INJECTION_PACK_HEADER _pIPH = GetPtrOfInjectionPackHeader(pImage);
	if(NULL == _pIPH)
		return false;

	*pIPH = *_pIPH;

	return true;
}
bool leaf::GetInjectionDataHeader(const void* pImage, int index, OUT PPE_INJECTION_DATA_HEADER pIDH)
{
	PCPE_INJECTION_DATA_HEADER _pIDH = GetPtrOfInjectionDataHeader(pImage, index);
	if(NULL == _pIDH)
		return false;

	*pIDH = *_pIDH;

	return true;
}
bool leaf::GetInjectionDataHeader(const void* pImage, const std::string& name, OUT PPE_INJECTION_DATA_HEADER pIDH)
{
	PCPE_INJECTION_DATA_HEADER _pIDH = GetPtrOfInjectionDataHeader(pImage, name);
	if(NULL == _pIDH)
		return false;

	*pIDH = *_pIDH;

	return true;
}

/* functions for injection of single data to PE file */
bool leaf::InjectDataToPeFile(const std::string& strInPeImagePath, const std::string& strOutPeImagePath, 
						const std::string& strDataName, const std::string& strDataFilePath, bool bInjectNew)
{
	CPeImageDataInjector DataInjector;
	if(false == DataInjector.PushBack(strDataName, strDataFilePath))
		return false;

	return DataInjector.Inject(strInPeImagePath, strOutPeImagePath, bInjectNew);
}
bool leaf::InjectDataToPeFile(const std::string& strInPeImagePath, const std::string& strOutPeImagePath, 
						const std::string& strDataName, const void* pcvBuffer, size_t SizeOfBuffer, bool bInjectNew)
{
	CPeImageDataInjector DataInjector;
	if(false == DataInjector.PushBack(strDataName, pcvBuffer, SizeOfBuffer))
		return false;

	return DataInjector.Inject(strInPeImagePath, strOutPeImagePath, bInjectNew);
}

bool leaf::ExtractDataFromPeFile(const void* pImage, int Index, OUT void* pvBuffer, size_t SizeOfBuffer)
{
	PCPE_INJECTION_DATA_HEADER pInjectionDataHeader = GetPtrOfInjectionDataHeader(pImage, Index);
	if(NULL == pInjectionDataHeader)
		return false;

	DWORD nByteToCopy = SizeOfBuffer;
	if(SizeOfBuffer > pInjectionDataHeader->dwSizeOfData)
		nByteToCopy = pInjectionDataHeader->dwSizeOfData;

	PCIMAGE_SECTION_HEADER pOrigCodeSectionHeader = GetPtrOfImageSectionHeader(pImage, ".text");
	if(NULL == pOrigCodeSectionHeader)
		return false;

	CCyclicXorTrans XorTrans;	//. Section Alignment: 0x1000
	if(false == XorTrans.LoadKeyTable((PBYTE)pImage+pOrigCodeSectionHeader->PointerToRawData, 0x200))
		return false;

	//. decoding
	DWORD dwSeed = ((PBYTE)pImage+pOrigCodeSectionHeader->PointerToRawData)[0x200+Index];
	return XorTrans.Trans((PBYTE)pImage+pInjectionDataHeader->dwPointerToRawData, nByteToCopy, pvBuffer, dwSeed);
}
bool leaf::ExtractDataFromPeFile(const void* pImage, const std::string& strDataName, OUT void* pvBuffer, size_t SizeOfBuffer)
{
	int Index;
	PCPE_INJECTION_DATA_HEADER pInjectionDataHeader = GetPtrOfInjectionDataHeader(pImage, strDataName, &Index);
	if(NULL == pInjectionDataHeader)
		return false;

	DWORD nByteToCopy = SizeOfBuffer;
	if(SizeOfBuffer > pInjectionDataHeader->dwSizeOfData)
		nByteToCopy = pInjectionDataHeader->dwSizeOfData;

	PCIMAGE_SECTION_HEADER pOrigCodeSectionHeader = GetPtrOfImageSectionHeader(pImage, ".text");
	if(NULL == pOrigCodeSectionHeader)
		return false;

	CCyclicXorTrans XorTrans;	//. Section Alignment: 0x1000
	if(false == XorTrans.LoadKeyTable((PBYTE)pImage+pOrigCodeSectionHeader->PointerToRawData, 0x200))
		return false;

	//. decoding
	DWORD dwSeed = ((PBYTE)pImage+pOrigCodeSectionHeader->PointerToRawData)[0x200+Index];
	return XorTrans.Trans((PBYTE)pImage+pInjectionDataHeader->dwPointerToRawData, nByteToCopy, pvBuffer, dwSeed);
}
bool leaf::ExtractDataFromPeFile(const void* pImage, int Index, OUT xstreambuf& xBuffer)
{
	PCPE_INJECTION_DATA_HEADER pInjectionDataHeader = GetPtrOfInjectionDataHeader(pImage, Index);
	if(NULL == pInjectionDataHeader)
		return false;

	xBuffer.flush();
	xBuffer.write((BYTE*)pImage + pInjectionDataHeader->dwPointerToRawData, pInjectionDataHeader->dwSizeOfData);

	return true;
}
bool leaf::ExtractDataFromPeFile(const void* pImage, const std::string& strDataName, OUT xstreambuf& xBuffer)
{
	PCPE_INJECTION_DATA_HEADER pInjectionDataHeader = GetPtrOfInjectionDataHeader(pImage, strDataName);
	if(NULL == pInjectionDataHeader)
		return false;

	xBuffer.flush();
	xBuffer.write((BYTE*)pImage + pInjectionDataHeader->dwPointerToRawData, pInjectionDataHeader->dwSizeOfData);

	return true;
}
bool leaf::ExtractDataFromPeFile(const void* pImage, int Index, OUT const std::string& strFileName)
{
	PCPE_INJECTION_DATA_HEADER pInjectionDataHeader = GetPtrOfInjectionDataHeader(pImage, Index);
	if(NULL == pInjectionDataHeader)
		return false;

	xfstreambuf xfInjectionData;
	xfInjectionData.write((BYTE*)pImage + pInjectionDataHeader->dwPointerToRawData, 
		pInjectionDataHeader->dwSizeOfData);

	return xfInjectionData.save(strFileName);
}
bool leaf::ExtractDataFromPeFile(const void* pImage, const std::string& strDataName, OUT const std::string& strFileName)
{
	PCPE_INJECTION_DATA_HEADER pInjectionDataHeader = GetPtrOfInjectionDataHeader(pImage, strDataName);
	if(NULL == pInjectionDataHeader)
		return false;

	xfstreambuf xfInjectionData;
	xfInjectionData.write((BYTE*)pImage + pInjectionDataHeader->dwPointerToRawData, 
		pInjectionDataHeader->dwSizeOfData);

	return xfInjectionData.save(strFileName);
}


/* Implementation of CPeImageIDM class */

CPeImageDataInjector::CPeImageDataInjector()
{}
CPeImageDataInjector::~CPeImageDataInjector()
{ RemoveAll(); }

bool CPeImageDataInjector::PushBack(const std::string& strDataName, IN const std::string& strFilePath)
{
	HANDLE hFile = CreateFile( strFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
		return false;

	DWORD dwSizeOfFile = GetFileSize(hFile, NULL);
	if(0xFFFFFFFF == dwSizeOfFile || NO_ERROR != GetLastError())
		return false;

	INJECTION_DATA* pInjectionData = new INJECTION_DATA;
	pInjectionData->strDataName = strDataName;
	pInjectionData->SizeOfBuffer = dwSizeOfFile;
	pInjectionData->pvBuffer = new char[dwSizeOfFile];
	
	DWORD dwNumberOfBytesRead = 0;
	if( ReadFile(hFile, pInjectionData->pvBuffer, dwSizeOfFile, &dwNumberOfBytesRead, NULL) &&
		(dwNumberOfBytesRead == dwSizeOfFile) )
	{
		m_listInjectionData.push_back(pInjectionData);

		CloseHandle(hFile);

		return true;
	}

	delete [] pInjectionData->pvBuffer;
	delete pInjectionData;

	CloseHandle(hFile);

	return false;
	
}
bool CPeImageDataInjector::PushBack(const std::string& strDataName, IN const void* pcvBuffer, size_t SizeOfBuffer)
{
	if(IsBadReadPtr(pcvBuffer, SizeOfBuffer))
		return false;

	INJECTION_DATA* pInjectionData = new INJECTION_DATA;
	pInjectionData->strDataName = strDataName;
	pInjectionData->SizeOfBuffer = SizeOfBuffer;
	pInjectionData->pvBuffer = new char[SizeOfBuffer];

	memcpy(pInjectionData->pvBuffer, pcvBuffer, SizeOfBuffer);

	m_listInjectionData.push_back(pInjectionData);

	return true;
}

bool CPeImageDataInjector::Insert(int Index, const std::string& strDataName, IN const std::string& strFilePath)
{
	HANDLE hFile = CreateFile( strFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
		return false;

	DWORD dwSizeOfFile = GetFileSize(hFile, NULL);
	if(0xFFFFFFFF == dwSizeOfFile || NO_ERROR != GetLastError())
		return false;

	INJECTION_DATA* pInjectionData = new INJECTION_DATA;
	pInjectionData->strDataName = strDataName;
	pInjectionData->SizeOfBuffer = dwSizeOfFile;
	pInjectionData->pvBuffer = new char[dwSizeOfFile];
	
	DWORD dwNumberOfBytesRead = 0;
	if( ReadFile(hFile, pInjectionData->pvBuffer, dwSizeOfFile, &dwNumberOfBytesRead, NULL) 
		&& (dwNumberOfBytesRead == dwSizeOfFile) )
	{	
		if(Index >= 0 && Index < (int)m_listInjectionData.size())
		{
			std::vector<INJECTION_DATA*>::iterator where = m_listInjectionData.begin() + Index;
			m_listInjectionData.insert(where, pInjectionData);
			
			CloseHandle(hFile);
			
			return true;
		}
	}

	delete [] pInjectionData->pvBuffer;
	delete pInjectionData;
	
	CloseHandle(hFile);

	return false;
}
bool CPeImageDataInjector::Insert(int Index, const std::string& strDataName, IN const void* pcvBuffer, size_t SizeOfBuffer)
{
	if(IsBadReadPtr(pcvBuffer, SizeOfBuffer))
		return false;

	INJECTION_DATA* pInjectionData = new INJECTION_DATA;
	pInjectionData->strDataName = strDataName;
	pInjectionData->SizeOfBuffer = SizeOfBuffer;
	pInjectionData->pvBuffer = new char[SizeOfBuffer];

	memcpy(pInjectionData->pvBuffer, pcvBuffer, SizeOfBuffer);

	if(Index >= 0 && Index < (int)m_listInjectionData.size())
	{
		std::vector<INJECTION_DATA*>::iterator where = m_listInjectionData.begin() + Index;
		m_listInjectionData.insert(where, pInjectionData);
		
		return true;
	}

	delete [] pInjectionData->pvBuffer;
	delete pInjectionData;
	
	return false;
}
void CPeImageDataInjector::Remove(int Index)
{
	if(Index >= 0 && Index < (int)m_listInjectionData.size())
	{
		std::vector<INJECTION_DATA*>::iterator where = m_listInjectionData.begin() + Index;
		
		delete [] (*where)->pvBuffer;
		delete (*where);

		m_listInjectionData.erase(where);
	}
}
void CPeImageDataInjector::Remove(const std::string& strDataName)
{
	std::vector<INJECTION_DATA*>::iterator vi = m_listInjectionData.begin();
	for(; vi != m_listInjectionData.end(); vi++)
	{
		if((*vi)->strDataName == strDataName)
		{
			delete [] (*vi)->pvBuffer;
			delete (*vi);

			m_listInjectionData.erase(vi);
		}
	}
}
void CPeImageDataInjector::RemoveAll()
{
	std::vector<INJECTION_DATA*>::iterator vi = m_listInjectionData.begin();
	for(; vi != m_listInjectionData.end(); vi++)
	{
		delete [] (*vi)->pvBuffer;
		delete (*vi);
	}
	m_listInjectionData.clear();
}

bool CPeImageDataInjector::Inject(const std::string& strInPeImagePath, const std::string& strOutPeImagePath, bool bInjectNew)
{
	xfstreambuf xfOrigImage, xfNewImage;

	if(false == xfOrigImage.load(strInPeImagePath))
		return false;

	PE_INJECTION_PACK_HEADER NewIPH;
	xstreambuf xNewIPD;
	if(false == BuildInjectionPack(xfOrigImage.data(), bInjectNew, &NewIPH, xNewIPD))
		return false;

	if(false == WriteDosHeader(xfNewImage, xfOrigImage.data(), NULL))
		return false;

	if(false == WriteNtHeaders(xfNewImage, xfOrigImage.data(), &NewIPH, NULL))
		return false;
	
	if(false == WriteSectionHeader(xfNewImage, xfOrigImage.data(), &NewIPH))
		return false;

	WriteInjectionPackHeader(xfNewImage, &NewIPH);

	if(false == WriteSections(xfNewImage, xfOrigImage.data(), &NewIPH, xNewIPD.data(), xNewIPD.size()))
		return false;

	return xfNewImage.save(strOutPeImagePath);
}

bool CPeImageDataInjector::BuildInjectionPack(const void* pOrigImage, bool bInjectNew, OUT PPE_INJECTION_PACK_HEADER pIPH, OUT xstreambuf& xIPD)
{
	PCIMAGE_DOS_HEADER pOrigDosHeader = GetPtrOfImageDosHeader(pOrigImage);
	if(NULL == pOrigDosHeader)	
		return false;
	
	PCIMAGE_NT_HEADERS pOrigNtHeaders = GetPtrOfImageNtHeaders(pOrigImage);
	if(NULL == pOrigNtHeaders)	//. acquire original nt header
		return false;

	DWORD dwSizeOfNonAlignedHeaders = pOrigDosHeader->e_lfanew /* DOS Headers */
		+ sizeof(IMAGE_NT_HEADERS) /* NT Headers */
		+ (sizeof(IMAGE_SECTION_HEADER)*pOrigNtHeaders->FileHeader.NumberOfSections) /* Section Headers */
		+ sizeof(PE_INJECTION_PACK_HEADER);	/* Injection Pack Headers */
	DWORD wFileAlignment = 0x200;
	DWORD wSizeOfHeaders = GetAlignedSize(dwSizeOfNonAlignedHeaders, wFileAlignment);

	PCIMAGE_SECTION_HEADER pOrigFirstSectionHeader = GetPtrOfImageSectionHeader(pOrigImage, 0);
	if(NULL == pOrigFirstSectionHeader)
		return false;

	PE_INJECTION_PACK_HEADER TempIPH;
	memset(&TempIPH, 0, sizeof(PE_INJECTION_PACK_HEADER));
	
	TempIPH.dwSignature = PE_INJECTION_PACK_SIGNATURE;
	TempIPH.wFileAlignment = wFileAlignment;
	TempIPH.wSizeOfHeaders = wSizeOfHeaders;
	TempIPH.dwBaseOfCode = wSizeOfHeaders;
	TempIPH.dwBaseOfIPD = wSizeOfHeaders + pOrigFirstSectionHeader->SizeOfRawData;

	PCPE_INJECTION_PACK_HEADER pOrigIPH = GetPtrOfInjectionPackHeader(pOrigImage);
	if(pOrigIPH)	//. acquire original IPH
		TempIPH.dwNumberOfIPD = pOrigIPH->dwNumberOfIPD + m_listInjectionData.size();
	else
		TempIPH.dwNumberOfIPD = m_listInjectionData.size();

	//. Build IPD
	xstreambuf xTempIPD;
	DWORD dwPointerToWrite = TempIPH.dwBaseOfIPD;
	if(NULL != pOrigIPH && false == bInjectNew) /* Build original data if the IPH exists */
	{
		for(int i = 0; i < (int)pOrigIPH->dwNumberOfIPD; i++)
		{
			PCPE_INJECTION_DATA_HEADER pOrigIDH = GetPtrOfInjectionDataHeader(pOrigImage, i);
			if(NULL == pOrigIDH)
				return false;

			PE_INJECTION_DATA_HEADER NewIDH;
			memset(&NewIDH, 0, sizeof(PE_INJECTION_DATA_HEADER));

			NewIDH.dwSignature = PE_INJECTION_DATA_SIGNATURE;
			NewIDH.dwPointerToDataName = dwPointerToWrite + sizeof(PE_INJECTION_DATA_HEADER);
			NewIDH.wSizeOfDataName = pOrigIDH->wSizeOfDataName;
			NewIDH.dwPointerToRawData = NewIDH.dwPointerToDataName + NewIDH.wSizeOfDataName;
			NewIDH.dwSizeOfData = pOrigIDH->dwSizeOfData;

			//. write header
			xTempIPD.write(&NewIDH, sizeof(PE_INJECTION_DATA_HEADER));
			//. write name
			xTempIPD.write((BYTE*)pOrigImage+pOrigIDH->dwPointerToDataName, pOrigIDH->wSizeOfDataName);
			//. write data
			xTempIPD.write((BYTE*)pOrigImage+pOrigIDH->dwPointerToRawData, pOrigIDH->dwSizeOfData);
			
			dwPointerToWrite = NewIDH.dwPointerToRawData + NewIDH.dwSizeOfData;
		}
	}

	/* Build new data */
	PCIMAGE_SECTION_HEADER pOrigCodeSectionHeader = GetPtrOfImageSectionHeader(pOrigImage, ".text");
	if(NULL == pOrigCodeSectionHeader)
		return false;
	
	CCyclicXorTrans XorTrans;	//. Section Alignment: 0x1000
	if(false == XorTrans.LoadKeyTable((PBYTE)pOrigImage+pOrigCodeSectionHeader->PointerToRawData, 0x200))
		return false;

	std::vector<INJECTION_DATA*>::iterator vi = m_listInjectionData.begin();
	for(int index=0; vi != m_listInjectionData.end(); vi++, index++)
	{
		INJECTION_DATA* pInjectionData = (*vi);

		//. Encoding
		DWORD dwSeed = ((PBYTE)pOrigImage+pOrigCodeSectionHeader->PointerToRawData)[0x200+index];
		xstreambuf xEncodedName, xEncodedData;
		XorTrans.Trans(pInjectionData->strDataName.c_str(), pInjectionData->strDataName.size()+1, xEncodedName, dwSeed);
		XorTrans.Trans(pInjectionData->pvBuffer, pInjectionData->SizeOfBuffer, xEncodedData, dwSeed);

		PE_INJECTION_DATA_HEADER NewIDH;
		memset(&NewIDH, 0, sizeof(PE_INJECTION_DATA_HEADER));

		NewIDH.dwSignature = PE_INJECTION_DATA_SIGNATURE;
		NewIDH.dwPointerToDataName = dwPointerToWrite + sizeof(PE_INJECTION_DATA_HEADER);
		NewIDH.wSizeOfDataName = xEncodedName.size();
		NewIDH.dwPointerToRawData = NewIDH.dwPointerToDataName + NewIDH.wSizeOfDataName;
		NewIDH.dwSizeOfData = xEncodedData.size();

		//. write header
		xTempIPD.write(&NewIDH, sizeof(PE_INJECTION_DATA_HEADER));
		//. write name
		xTempIPD.write(xEncodedName.data(), xEncodedName.size());
		//. write data
		xTempIPD.write(xEncodedData.data(), xEncodedData.size());

		dwPointerToWrite = NewIDH.dwPointerToRawData + NewIDH.dwSizeOfData;
	}

	TempIPH.dwSizeOfIPD = GetAlignedSize(xTempIPD.size(), pOrigNtHeaders->OptionalHeader.SectionAlignment);
	
	*pIPH = TempIPH;
	xIPD = xTempIPD;

	return true;
}

bool CPeImageDataInjector::WriteDosHeader(xstreambuf& xNewImage, const void* pOrigImage, OUT PIMAGE_DOS_HEADER pNewDosHeader)
{
	//. acquire original dos header
	PCIMAGE_DOS_HEADER pOrigDosHeader = GetPtrOfImageDosHeader(pOrigImage);
	if(NULL == pOrigDosHeader)
		return false;

	//. write dos header
	xNewImage.write(pOrigDosHeader, sizeof(IMAGE_DOS_HEADER));

	//. write extra dos header
	DWORD SizeOfExtraDosHeader = pOrigDosHeader->e_lfanew - sizeof(IMAGE_DOS_HEADER);
	xNewImage.write((BYTE*)pOrigImage+sizeof(IMAGE_DOS_HEADER), SizeOfExtraDosHeader);
	
	//. acquire new dos header
	if(pNewDosHeader)
		return GetImageDosHeader(xNewImage.data(), pNewDosHeader);

	return true;
}
bool CPeImageDataInjector::WriteNtHeaders(xstreambuf& xNewImage, const void* pOrigImage, const PPE_INJECTION_PACK_HEADER pIPH, OUT PIMAGE_NT_HEADERS pNewNtHeaders)
{
	//. acquire nt header
	PCIMAGE_NT_HEADERS pOrigNtHeaders = GetPtrOfImageNtHeaders(pOrigImage);
	if(NULL == pOrigNtHeaders)
		return false;
	
	//. write nt header
	IMAGE_NT_HEADERS NewNtHeaders = *pOrigNtHeaders;
	NewNtHeaders.OptionalHeader.FileAlignment = pIPH->wFileAlignment;
	NewNtHeaders.OptionalHeader.SizeOfHeaders = pIPH->wSizeOfHeaders;
	NewNtHeaders.OptionalHeader.BaseOfCode = pIPH->dwBaseOfCode;
	NewNtHeaders.OptionalHeader.BaseOfData = pIPH->dwBaseOfCode + pOrigNtHeaders->OptionalHeader.SizeOfCode + pIPH->dwSizeOfIPD;
	xNewImage.write(&NewNtHeaders, sizeof(IMAGE_NT_HEADERS));

	//. acquire nt header
	if(pNewNtHeaders)
		return GetImageNtHeaders(xNewImage.data(), pNewNtHeaders);

	return true;	
}
bool CPeImageDataInjector::WriteSectionHeader(xstreambuf& xNewImage, const void* pOrigImage, const PPE_INJECTION_PACK_HEADER pIPH)
{
	//. acquire nt header
	PCIMAGE_NT_HEADERS pOrigNtHeaders = GetPtrOfImageNtHeaders(pOrigImage);
	if(NULL == pOrigNtHeaders)
		return false;

	DWORD dwPtrToNextSection = pIPH->dwBaseOfCode;
	for(int i=0; i<pOrigNtHeaders->FileHeader.NumberOfSections; i++)
	{
		PCIMAGE_SECTION_HEADER pOrigSectionHeader = GetPtrOfImageSectionHeader(pOrigImage, i);
		if(NULL == pOrigSectionHeader)
			return false;

		IMAGE_SECTION_HEADER NewSectionHeader = *pOrigSectionHeader;
		NewSectionHeader.PointerToRawData = dwPtrToNextSection;
		
		// pOrigSectionHeader->PointerToRelocations : this field is meaningless in EXEs.
		// pOrigSectionHeader->PointerToLinenumbers : old-style
		assert(0 == pOrigSectionHeader->PointerToLinenumbers && 0 == pOrigSectionHeader->NumberOfLinenumbers);

		xNewImage.write(&NewSectionHeader, sizeof(IMAGE_SECTION_HEADER));

		dwPtrToNextSection += NewSectionHeader.SizeOfRawData;

		if(i==0)	//. Injection Point
			dwPtrToNextSection += pIPH->dwSizeOfIPD;
	}
	return true;
}
void CPeImageDataInjector::WriteInjectionPackHeader(xstreambuf& xNewImage, const PPE_INJECTION_PACK_HEADER pIPH)
{
	xNewImage.write(pIPH, sizeof(PE_INJECTION_PACK_HEADER));
	xNewImage.resize(pIPH->wSizeOfHeaders);
	xNewImage.seek(pIPH->wSizeOfHeaders);
}
bool CPeImageDataInjector::WriteSections(xstreambuf& xNewImage, const void* pOrigImage, const PPE_INJECTION_PACK_HEADER pIPH, 
									const void* pcvIPDBuffer, size_t SizeOfIPDBuffer)
{
	//. acquire nt header
	PCIMAGE_NT_HEADERS pOrigNtHeaders = GetPtrOfImageNtHeaders(pOrigImage);
	if(NULL == pOrigNtHeaders)
		return false;

	for(int i=0; i<pOrigNtHeaders->FileHeader.NumberOfSections; i++)
	{
		PCIMAGE_SECTION_HEADER pOrigSectionHeader = GetPtrOfImageSectionHeader(pOrigImage, i);
		if(NULL == pOrigSectionHeader)
			return false;
		
		if(pOrigSectionHeader->SizeOfRawData > 0)
			xNewImage.write((const BYTE*)pOrigImage+pOrigSectionHeader->PointerToRawData, pOrigSectionHeader->SizeOfRawData);

		if(i == 0)	//. Injection Point
		{
			xNewImage.write(pcvIPDBuffer, SizeOfIPDBuffer);
			xNewImage.resize(xNewImage.size()+(pIPH->dwSizeOfIPD-SizeOfIPDBuffer));
			xNewImage.seek(xNewImage.size());
		}
	}

	return true;
}

DWORD CPeImageDataInjector::GetAlignedSize(DWORD dwNonAlignedSize, DWORD dwAlignment)
{
	return (DWORD)(((double)dwNonAlignedSize/(double)dwAlignment)+0.99f)*dwAlignment;
}
