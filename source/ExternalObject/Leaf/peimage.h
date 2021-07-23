

#ifndef _PEIMAGE_H_
#define _PEIMAGE_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>
#include <vector>

#include <windows.h>

#ifndef __IMAGE_COR20_HEADER_DEFINED__		//. for .NET
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines
{
// COM+ Header entry point flags.
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

// Version flags for image.
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

// Maximum size of a NativeType descriptor.
    NATIVE_TYPE_MAX_CB                  =1,   
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

// #defines for the MIH FLAGS
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,    
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

// V-table constants
    COR_VTABLE_32BIT                    =0x01,          // V-table slots are 32-bits in size.   
    COR_VTABLE_64BIT                    =0x02,          // V-table slots are 64-bits in size.   
    COR_VTABLE_FROM_UNMANAGED           =0x04,          // If set, transition from unmanaged.
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,          // Call most derived method described by

// EATJ constants
    IMAGE_COR_EATJ_THUNK_SIZE           =32,            // Size of a jump thunk reserved range.

// Max name lengths    
    //@todo: Change to unlimited name lengths.
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} ReplacesCorHdrNumericDefines;

// COM+ 2.0 header structure.
typedef struct IMAGE_COR20_HEADER
{
    // Header versioning
    DWORD                   cb;              
    WORD                    MajorRuntimeVersion;
    WORD                    MinorRuntimeVersion;
    
    // Symbol table and startup information
    IMAGE_DATA_DIRECTORY    MetaData;        
    DWORD                   Flags;           
    DWORD                   EntryPointToken;
    
    // Binding information
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

    // Regular fixup and binding information
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

    // Precompiled image info (internal use only - set to zero)
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
    
} IMAGE_COR20_HEADER, *PIMAGE_COR20_HEADER;

#endif // __IMAGE_COR20_HEADER_DEFINED__

typedef const IMAGE_DOS_HEADER* PCIMAGE_DOS_HEADER;
typedef const IMAGE_NT_HEADERS* PCIMAGE_NT_HEADERS;
typedef const IMAGE_SECTION_HEADER* PCIMAGE_SECTION_HEADER;
typedef const IMAGE_DATA_DIRECTORY* PCIMAGE_DATA_DIRECTORY;

namespace leaf {
	
	/* Image Format */

/*
	IMAGE_DIRECTORY_ENTRY_EXPORT: PIMAGE_EXPORT_DIRECTORY				// Export Directory
	IMAGE_DIRECTORY_ENTRY_IMPORT: PIMAGE_IMPORT_DESCRIPTOR				// Import Directory
	IMAGE_DIRECTORY_ENTRY_RESOURCE:	PIMAGE_RESOURCE_DIRECTORY			// Resource Directory
	IMAGE_DIRECTORY_ENTRY_EXCEPTION: PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY	// Exception Directory
	IMAGE_DIRECTORY_ENTRY_SECURITY:	LPWIN_CERTIFICATE					// Security Directory
	IMAGE_DIRECTORY_ENTRY_BASERELOC: PIMAGE_BASE_RELOCATION				// Base Relocation Table
	IMAGE_DIRECTORY_ENTRY_DEBUG: PIMAGE_DEBUG_DIRECTORY					// Debug Directory
	IMAGE_DIRECTORY_ENTRY_ARCHITECTURE: PIMAGE_ARCHITECTURE_HEADER		// Architecture Specific Data
	IMAGE_DIRECTORY_ENTRY_GLOBALPTR:									// RVA of GP
	IMAGE_DIRECTORY_ENTRY_TLS: PIMAGE_TLS_DIRECTORY						// TLS Directory
	IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG: PIMAGE_LOAD_CONFIG_DIRECTORY		// Load Configuration Directory
	IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT: PIMAGE_BOUND_IMPORT_DESCRIPTOR	// Bound Import Directory in headers
	IMAGE_DIRECTORY_ENTRY_IAT:											// Import Address Table
	IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT: PCImgDelayDescr					// Delay Load Import Descriptors
	IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR: PIMAGE_COR20_HEADER			// COM Runtime descriptor
*/

	PIMAGE_DOS_HEADER GetPtrOfImageDosHeader(void* pImage);
	PCIMAGE_DOS_HEADER GetPtrOfImageDosHeader(const void* pImage);

	PIMAGE_NT_HEADERS GetPtrOfImageNtHeaders(void* pImage);
	PCIMAGE_NT_HEADERS GetPtrOfImageNtHeaders(const void* pImage);

	PIMAGE_SECTION_HEADER GetPtrOfImageSectionHeader(void* pImage, int index);
	PCIMAGE_SECTION_HEADER GetPtrOfImageSectionHeader(const void* pImage, int index);

	PIMAGE_SECTION_HEADER GetPtrOfImageSectionHeader(void* pImage, const std::string& name);
	PCIMAGE_SECTION_HEADER GetPtrOfImageSectionHeader(const void* pImage, const std::string& name);

	PIMAGE_DATA_DIRECTORY GetPtrOfDataDirectory(void* pImage, int entry);
	PCIMAGE_DATA_DIRECTORY GetPtrOfDataDirectory(const void* pImage, int entry);

	PIMAGE_SECTION_HEADER FindEnclosingSectionHeaderFromRVA(void* pImage, DWORD dwRVA);
	PCIMAGE_SECTION_HEADER FindEnclosingSectionHeaderFromRVA(const void* pImage, DWORD dwRVA);

	LPVOID GetPtrDirectoryPtrFromRVA(void* pImage, DWORD dwRVA);
	
	bool GetImageDosHeader(const void* pImage, OUT PIMAGE_DOS_HEADER pImageDosHeader);
	bool GetImageNtHeaders(const void* pImage, OUT PIMAGE_NT_HEADERS pImageNtHeaders);
	bool GetImageSectionHeader(const void* pImage, int index, OUT PIMAGE_SECTION_HEADER pImageSectionHeader);
	bool GetImageSectionHeader(const void* pImage, const std::string& name, OUT PIMAGE_SECTION_HEADER pImageSectionHeader);
	bool GetImageDataDirectory(const void* pImage, int entry, OUT PIMAGE_DATA_DIRECTORY pImageDataDirectory);
	

	/* Data Injection into Portable Executable File */

	const DWORD PE_INJECTION_PACK_SIGNATURE = 0x48504950;
	const DWORD PE_INJECTION_DATA_SIGNATURE = 0x48444950;
	
#pragma pack(push, 1)
	typedef struct _PE_INJECTION_PACK_HEADER {
		DWORD	dwSignature;	//. 0x48504950: PIPH
		DWORD	dwNumberOfIPD;
		DWORD	dwBaseOfCode;
		WORD	wFileAlignment;
		WORD	wSizeOfHeaders;
		DWORD	dwBaseOfIPD;
		DWORD	dwSizeOfIPD;
	} PE_INJECTION_PACK_HEADER, * PPE_INJECTION_PACK_HEADER;
	
	typedef struct _PE_INJECTION_DATA_HEADER {
		DWORD	dwSignature;	//. 0x48444950: PIDH
		DWORD	dwPointerToDataName;
		WORD	wSizeOfDataName;
		DWORD	dwPointerToRawData;
		DWORD	dwSizeOfData;
	} PE_INJECTION_DATA_HEADER, * PPE_INJECTION_DATA_HEADER;
#pragma pack(pop)

	typedef const PE_INJECTION_PACK_HEADER* PCPE_INJECTION_PACK_HEADER;
	typedef const PE_INJECTION_DATA_HEADER* PCPE_INJECTION_DATA_HEADER;

	class xstreambuf;	/* Pre-declaration */


	PPE_INJECTION_PACK_HEADER GetPtrOfInjectionPackHeader(void* pImage);
	PCPE_INJECTION_PACK_HEADER GetPtrOfInjectionPackHeader(const void* pImage);

	PPE_INJECTION_DATA_HEADER GetPtrOfInjectionDataHeader(void* pImage, int index);
	PCPE_INJECTION_DATA_HEADER GetPtrOfInjectionDataHeader(const void* pImage, int index);

	PPE_INJECTION_DATA_HEADER GetPtrOfInjectionDataHeader(void* pImage, const std::string& name, OUT int* piIndex = NULL);
	PCPE_INJECTION_DATA_HEADER GetPtrOfInjectionDataHeader(const void* pImage, const std::string& name, OUT int* piIndex = NULL);

	bool GetInjectionPackHeader(const void* pImage, OUT PPE_INJECTION_PACK_HEADER pIPH);
	bool GetInjectionDataHeader(const void* pImage, int index, OUT PPE_INJECTION_DATA_HEADER pIDH);
	bool GetInjectionDataHeader(const void* pImage, const std::string& name, OUT PPE_INJECTION_DATA_HEADER pIDH);

	/* functions for injection of single data to PE file */
	bool InjectDataToPeFile(const std::string& strInPeImagePath, const std::string& strOutPeImagePath, 
		const std::string& strDataName, const std::string& strDataFilePath, bool bInjectNew = false);
	bool InjectDataToPeFile(const std::string& strInPeImagePath, const std::string& strOutPeImagePath, 
		const std::string& strDataName, const void* pcvBuffer, size_t SizeOfBuffer, bool bInjectNew = false);
	
	bool ExtractDataFromPeFile(const void* pImage, int Index, OUT void* pvBuffer, size_t SizeOfBuffer);
	bool ExtractDataFromPeFile(const void* pImage, const std::string& strDataName, OUT void* pvBuffer, size_t SizeOfBuffer);
	bool ExtractDataFromPeFile(const void* pImage, int Index, OUT xstreambuf& xBuffer);
	bool ExtractDataFromPeFile(const void* pImage, const std::string& strDataName, OUT xstreambuf& xBuffer);
	bool ExtractDataFromPeFile(const void* pImage, int Index, OUT const std::string& strFileName);
	bool ExtractDataFromPeFile(const void* pImage, const std::string& strDataName, OUT const std::string& strFileName);

	/* a class for injection of multi-data to PE file */
	class CPeImageDataInjector
	{		
		struct INJECTION_DATA {
			std::string	strDataName;
			void*		pvBuffer;
			size_t		SizeOfBuffer;
		};
		std::vector<INJECTION_DATA*>	m_listInjectionData;

	public:
		CPeImageDataInjector();
		~CPeImageDataInjector();

		bool PushBack(const std::string& strDataName, IN const std::string& strFilePath);
		bool PushBack(const std::string& strDataName, IN const void* pcvBuffer, size_t SizeOfBuffer);
		bool Insert(int Index, const std::string& strDataName, IN const std::string& strFilePath);
		bool Insert(int Index, const std::string& strDataName, IN const void* pcvBuffer, size_t SizeOfBuffer);
		void Remove(int Index);
		void Remove(const std::string& strDataName);
		void RemoveAll();

		bool Inject(const std::string& strInPeImagePath, const std::string& strOutPeImagePath, bool bInjectNew = false);

	protected:
		bool BuildInjectionPack(const void* pOrigImage, bool bInjectNew, OUT PPE_INJECTION_PACK_HEADER pIPH, OUT xstreambuf& xIPD);
		
		bool WriteDosHeader(xstreambuf& xNewImage, const void* pOrigImage, OUT PIMAGE_DOS_HEADER pNewDosHeader);
		bool WriteNtHeaders(xstreambuf& xNewImage, const void* pOrigImage, const PPE_INJECTION_PACK_HEADER pIPH, OUT PIMAGE_NT_HEADERS pNewNtHeaders);
		bool WriteSectionHeader(xstreambuf& xNewImage, const void* pOrigImage, const PPE_INJECTION_PACK_HEADER pIPH);
		void WriteInjectionPackHeader(xstreambuf& xNewImage, const PPE_INJECTION_PACK_HEADER pIPH);
		bool WriteSections(xstreambuf& xNewImage, const void* pOrigImage, const PPE_INJECTION_PACK_HEADER pIPH, 
			const void* pcvIPDBuffer, size_t SizeOfIPDBuffer);

		DWORD GetAlignedSize(DWORD dwNonAlignedSize, DWORD dwAlignment);
	};
	
}	// namespace leaf

#endif // _PEIMAGE_H_