
#ifndef _CHECKINTEGRITY_H_
#define _CHECKINTEGRITY_H_

#include <windows.h>

#pragma warning(disable : 4786)
#include <string>
#include <vector>

#include "stdleaf.h"

namespace leaf {

	/* CheckSum */

	typedef struct _CHECKSUMTABLE {
		DWORD TABLE_SIZE;
		DWORD	pdwTable[256];
		_CHECKSUMTABLE() : TABLE_SIZE(sizeof(DWORD)*256) {}
	} CHECKSUMTABLE, * LPCHECKSUMTABLE;

	bool GenerateCheckSum(IN const void* pBuffer, size_t size, OUT LPCHECKSUMTABLE lpCheckSumTable);
	bool GenerateCheckSum(const std::string& in_filename, OUT LPCHECKSUMTABLE lpCheckSumTable);
	bool GenerateCheckSum(IN const void* pBuffer, size_t size, const std::string& out_filename);
	bool GenerateCheckSum(const std::string& in_filename, const std::string& out_filename);

	class CCheckSumGenerator
	{
		struct SEGMENTINFO {
			std::string name;	//. file name or data name
			CHECKSUMTABLE cs_table;
		};
		std::vector<SEGMENTINFO*>	m_listSegment;
		
	public:
		CCheckSumGenerator();
		~CCheckSumGenerator();

		bool AddSegmentFromBuffer(const std::string& dataname, const void* pBuffer, size_t size);
		bool AddSegmentFromFile(const std::string& filename);
		bool RemoveSegment(const std::string& name);
		bool RemoveSegment(int index);
		void RemoveAllSegment();

		int GetSegmentCount() const;
		const SEGMENTINFO* GetSegmentInfo(const std::string& name);
		const SEGMENTINFO* GetSegmentInfo(int index);

		/* Generate 1KB CheckSum data */
		bool Generate(OUT LPCHECKSUMTABLE lpCheckSumTable);
		bool Generate(const std::string& out_filename);

		static bool GenerateCheckSumTable(IN const void* pBuffer, size_t size, OUT LPCHECKSUMTABLE lpCheckSumTable);
		static bool GenerateCheckSumTable(const std::string& in_filename, OUT LPCHECKSUMTABLE lpCheckSumTable);
		static bool GenerateCheckSumFile(IN const void* pBuffer, size_t size, const std::string& out_filename);
		static bool GenerateCheckSumFile(const std::string& in_filename, const std::string& out_filename);
	};

	/* Cyclic Redundancy Check (CRC) */

	bool GenerateCrc32Code(const void* pBuffer, size_t size, DWORD& dwCrc32);	/* Generate 32bit CRC code from buffer */
	bool GenerateCrc32Code(const std::string& filename, DWORD& dwCrc32);		/* Generate 32bit CRC code from file */

	
	class CCyclicRedundancyCheck32 {
		DWORD*	m_pdwCrc32Table;

	public:
		~CCyclicRedundancyCheck32();

		bool GenerateCrc32Code(const void* pBuffer, size_t size, DWORD& dwCrc32);
		bool GenerateCrc32Code(const std::string& filename, DWORD& dwCrc32);

		static CCyclicRedundancyCheck32* GetObjPtr();
		
	private:
		CCyclicRedundancyCheck32();		//. Ban create instance

		void CreateCrc32Table();
		void DestroyCrc32Table();

		void GenerateCrc32SeedAssembly(BYTE* pbyBuffer, size_t size, DWORD& dwCrc32);
	};
}

#endif // _CHECKINTEGRITY_H_