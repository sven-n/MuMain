#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCRC32{

	public:
		CCRC32(void);
		~CCRC32(void);

		void Initialize(void);

		unsigned long CCRC32::FileCRC(const wchar_t *sFileName);
		bool FileCRC(const char *sFileName, unsigned long *ulOutCRC);
		bool FileCRC(const char *sFileName, unsigned long *ulOutCRC, unsigned long ulBufferSize);

		unsigned long FullCRC(const unsigned char *sData, unsigned long ulDataLength);
		void FullCRC(const unsigned char *sData, unsigned long ulLength, unsigned long *ulOutCRC);

		void PartialCRC(unsigned long *ulCRC, const unsigned char *sData, unsigned long ulDataLength);

	private:
		unsigned long Reflect(unsigned long ulReflect, const char cChar);
		unsigned long ulTable[256]; // CRC lookup table array.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
