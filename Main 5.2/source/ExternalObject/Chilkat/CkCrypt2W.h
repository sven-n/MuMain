#ifndef _CkCrypt2W_H
#define _CkCrypt2W_H
#pragma once

#include "CkString.h"
#include "CkCrypt2.h"


#pragma pack (push, 8) 


class CkCrypt2W : public CkCrypt2
{
    private:
	unsigned long nextIdxW(void)
	    {
	    m_resultIdxW++;
	    if (m_resultIdxW >= 10)
		{
		m_resultIdxW = 0;
		}
	    return m_resultIdxW;
	    }
	unsigned long m_resultIdxW;
	CkString m_resultStringW[10];

    public:
	CkCrypt2W() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCrypt2W() { }
	void put_CharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Charset(s1.getUtf8());
	    }
	void put_CipherModeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_CipherMode(s1.getUtf8());
	    }
	void put_CompressionAlgorithmW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_CompressionAlgorithm(s1.getUtf8());
	    }
	void put_CryptAlgorithmW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_CryptAlgorithm(s1.getUtf8());
	    }
	void put_EncodingModeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_EncodingMode(s1.getUtf8());
	    }
	void put_HashAlgorithmW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_HashAlgorithm(s1.getUtf8());
	    }
	void put_PbesAlgorithmW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_PbesAlgorithm(s1.getUtf8());
	    }
	void put_PbesPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_PbesPassword(s1.getUtf8());
	    }
	bool bytesToStringW(CkByteData &inData,const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BytesToString(inData,sTemp1.getUtf8(),outStr);
		}
	bool ckDecryptFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CkDecryptFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool ckEncryptFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CkEncryptFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool compressStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressString(sTemp1.getUtf8(),out);
		}
	bool compressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressStringENC(sTemp1.getUtf8(),out);
		}
	bool createDetachedSignatureW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CreateDetachedSignature(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool createP7MW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CreateP7M(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool createP7SW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CreateP7S(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void decodeW(const wchar_t *w1,const wchar_t *w2,CkByteData &bData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->Decode(sTemp1.getUtf8(),sTemp2.getUtf8(),bData);
		}
	bool decryptBytesENCW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecryptBytesENC(sTemp1.getUtf8(),out);
		}
	bool decryptEncodedW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecryptEncoded(sTemp1.getUtf8(),outStr);
		}
	bool decryptStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecryptStringENC(sTemp1.getUtf8(),out);
		}
	void encodeW(const CkByteData &bData,const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->Encode(bData,sTemp1.getUtf8(),str);
		}
	bool encryptEncodedW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->EncryptEncoded(sTemp1.getUtf8(),outStr);
		}
	bool encryptStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->EncryptString(sTemp1.getUtf8(),out);
		}
	bool encryptStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->EncryptStringENC(sTemp1.getUtf8(),out);
		}
	void genEncodedSecretKeyW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GenEncodedSecretKey(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	void generateSecretKeyW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GenerateSecretKey(sTemp1.getUtf8(),out);
		}
	void getEncodedIVW(const wchar_t *w1,CkString &strIV)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedIV(sTemp1.getUtf8(),strIV);
		}
	void getEncodedKeyW(const wchar_t *w1,CkString &strKey)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedKey(sTemp1.getUtf8(),strKey);
		}
	void getEncodedSaltW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedSalt(sTemp1.getUtf8(),outStr);
		}
	bool hashBeginStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HashBeginString(sTemp1.getUtf8());
		}
	bool hashFileW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HashFile(sTemp1.getUtf8(),out);
		}
	bool hashFileENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HashFileENC(sTemp1.getUtf8(),out);
		}
	bool hashMoreStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HashMoreString(sTemp1.getUtf8());
		}
	bool hashStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HashString(sTemp1.getUtf8(),out);
		}
	bool hashStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HashStringENC(sTemp1.getUtf8(),out);
		}
	void hmacStringW(const wchar_t *w1,CkByteData &hmacOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->HmacString(sTemp1.getUtf8(),hmacOut);
		}
	void hmacStringENCW(const wchar_t *w1,CkString &encodedHmacOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->HmacStringENC(sTemp1.getUtf8(),encodedHmacOut);
		}
	bool inflateBytesENCW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->InflateBytesENC(sTemp1.getUtf8(),out);
		}
	bool inflateStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->InflateStringENC(sTemp1.getUtf8(),out);
		}
	bool mySqlAesDecryptW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->MySqlAesDecrypt(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool mySqlAesEncryptW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->MySqlAesEncrypt(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool opaqueSignStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpaqueSignString(sTemp1.getUtf8(),out);
		}
	bool opaqueSignStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpaqueSignStringENC(sTemp1.getUtf8(),out);
		}
	bool opaqueVerifyBytesENCW(const wchar_t *w1,CkByteData &original)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpaqueVerifyBytesENC(sTemp1.getUtf8(),original);
		}
	bool opaqueVerifyStringENCW(const wchar_t *w1,CkString &original)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpaqueVerifyStringENC(sTemp1.getUtf8(),original);
		}
	bool pbkdf1W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4,int iterationCount,int outputKeyBitLen,const wchar_t *w5,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		CkString sTemp5; sTemp5.appendU(w5);
		return this->Pbkdf1(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8(),iterationCount,outputKeyBitLen,sTemp5.getUtf8(),outStr);
		}
	bool pbkdf2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4,int iterationCount,int outputKeyBitLen,const wchar_t *w5,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		CkString sTemp5; sTemp5.appendU(w5);
		return this->Pbkdf2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8(),iterationCount,outputKeyBitLen,sTemp5.getUtf8(),outStr);
		}
	bool reEncodeW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->ReEncode(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),outStr);
		}
	bool readFileW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadFile(sTemp1.getUtf8(),outBytes);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	void setEncodedIVW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetEncodedIV(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void setEncodedKeyW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetEncodedKey(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void setEncodedSaltW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetEncodedSalt(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void setHmacKeyEncodedW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetHmacKeyEncoded(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void setHmacKeyStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetHmacKeyString(sTemp1.getUtf8());
		}
	void setSecretKeyViaPasswordW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetSecretKeyViaPassword(sTemp1.getUtf8());
		}
	bool signStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SignString(sTemp1.getUtf8(),out);
		}
	bool signStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SignStringENC(sTemp1.getUtf8(),out);
		}
	bool stringToBytesW(const wchar_t *w1,const wchar_t *w2,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->StringToBytes(sTemp1.getUtf8(),sTemp2.getUtf8(),outBytes);
		}
	void trimEndingWithW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->TrimEndingWith(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool verifyBytesENCW(const CkByteData &bData,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->VerifyBytesENC(bData,sTemp1.getUtf8());
		}
	bool verifyDetachedSignatureW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyDetachedSignature(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool verifyP7MW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyP7M(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool verifyP7SW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyP7S(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool verifyStringW(const wchar_t *w1,const CkByteData &sigData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->VerifyString(sTemp1.getUtf8(),sigData);
		}
	bool verifyStringENCW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyStringENC(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool writeFileW(const wchar_t *w1,CkByteData &fileData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFile(sTemp1.getUtf8(),fileData);
		}
	const wchar_t *bytesToStringW(CkByteData &inData,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->BytesToString(inData,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *charsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Charset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *cipherModeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_CipherMode(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *compressBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->CompressBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *compressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->CompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *compressionAlgorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_CompressionAlgorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *cryptAlgorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_CryptAlgorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decryptEncodedW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->DecryptEncoded(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decryptStringW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->DecryptString(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decryptStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->DecryptStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodeW(const CkByteData &bData,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->Encode(bData,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodingModeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncodingMode(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->EncryptBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptEncodedW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->EncryptEncoded(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->EncryptStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *genEncodedSecretKeyW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GenEncodedSecretKey(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *genRandomBytesENCW(int numBytes)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GenRandomBytesENC(numBytes,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEncodedIVW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedIV(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEncodedKeyW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedKey(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEncodedSaltW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedSalt(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hashAlgorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HashAlgorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hashBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->HashBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hashFileENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->HashFileENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hashFinalENCW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->HashFinalENC(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hashStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->HashStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hmacBytesENCW(const CkByteData &inBytes)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->HmacBytesENC(inBytes,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hmacStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->HmacStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *inflateStringW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->InflateString(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *inflateStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->InflateStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorHtmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorHtml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *mySqlAesDecryptW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->MySqlAesDecrypt(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *mySqlAesEncryptW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->MySqlAesEncrypt(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *opaqueSignBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->OpaqueSignBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *opaqueSignStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->OpaqueSignStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *opaqueVerifyStringW(const CkByteData &p7s)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->OpaqueVerifyString(p7s,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *opaqueVerifyStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->OpaqueVerifyStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *pbesAlgorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_PbesAlgorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *pbesPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_PbesPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *pbkdf1W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4,int iterationCount,int outputKeyBitLen,const wchar_t *w5)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		CkString sTemp5; sTemp5.appendU(w5);
		this->Pbkdf1(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8(),iterationCount,outputKeyBitLen,sTemp5.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *pbkdf2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4,int iterationCount,int outputKeyBitLen,const wchar_t *w5)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		CkString sTemp5; sTemp5.appendU(w5);
		this->Pbkdf2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8(),iterationCount,outputKeyBitLen,sTemp5.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *reEncodeW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->ReEncode(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *signBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->SignBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *signStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->SignStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *trimEndingWithW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->TrimEndingWith(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
