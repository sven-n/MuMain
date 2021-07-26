#include "stdafx.h"
#include "BannerInfo.h"
#include "StringToken.h"
#include <map>

CBannerInfo::CBannerInfo()
{
	CBannerInfo* BannerInfo = new CBannerInfo;
	this->BannerSeq = BannerInfo->BannerSeq;
	memcpy(this->BannerName, BannerInfo->BannerName, sizeof(this->BannerName));
	memcpy(this->BannerImageURL, BannerInfo->BannerImageURL, sizeof(this->BannerImageURL));
	this->BannerOrder = BannerInfo->BannerOrder;
	this->BannerDirection = BannerInfo->BannerDirection;
	memcpy(&this->BannerStartDate, &BannerInfo->BannerStartDate, sizeof(this->BannerStartDate));
	memcpy(&this->BannerEndDate, &BannerInfo->BannerEndDate, sizeof(this->BannerEndDate));
	memcpy(this->BannerLinkURL, BannerInfo->BannerLinkURL, sizeof(this->BannerLinkURL));
	memcpy(this->BannerImagePath, BannerInfo->BannerImagePath, sizeof(this->BannerImagePath));
}

CBannerInfo::~CBannerInfo()
{
	delete(this);
}

bool CBannerInfo::SetBanner(std::string strdata, std::string strDirPath, bool bDonwLoad)
{
	//std::string strImageFile;
	//std::string strImageFileName;

	//strImageFileName.push_back;

	//strImageFileName._Myres = 15;
	//strImageFileName._Mysize = 0;
	//strImageFileName._Bx._Buf[0] = 0;

	//strImageFileName.assign("@", 1);

	//CStringToken* lpScript;

	//if (strImageFileName.size() >= 0x10)
	//	operator delete(strImageFileName._Bx._Ptr);

	//strImageFileName._Myres = 15;
	//strImageFileName._Mysize = 0;
	//strImageFileName._Bx._Buf[0] = 0;

	//const char *v6;

	//std::string v5 = lpScript->nextToken();

	//if (v5._Myres < 0x10)
	//	v6 = v5._Bx._Ptr;
	//else
	//	v6 = v5._Bx._Buff;

	//this->BannerSeq = atoi(v6);

//	if (v45 >= 0x10)
//		operator delete(v44);
//
//	v7 = CStringToken::nextToken(&st, v43);
//
//	if (*(_DWORD *)(v7 + 24) < 0x10u)
//		v8 = (char *)(v7 + 4);
//	else
//		v8 = *(char **)(v7 + 4);
//	v9 = this->BannerName;
//	v10 = 2147483646;
//	v11 = 50;
//	v12 = 0;
//	while (v11)
//	{
//		if (!v10 || !*v8)
//			goto LABEL_17;
//		*v9++ = *v8++;
//		--v11;
//		--v10;
//		++v12;
//	}
//	--v9;
//LABEL_17:
//	*v9 = 0;
//	if (v45 >= 0x10)
//		operator delete(v44);
//	v13 = CStringToken::nextToken(&st, v43);
//	if (*(_DWORD *)(v13 + 24) < 0x10u)
//		v14 = (char *)(v13 + 4);
//	else
//		v14 = *(char **)(v13 + 4);
//	v15 = v46->BannerImageURL;
//	v16 = 2147483646;
//	v17 = 2084;
//	v18 = 0;
//	while (v17)
//	{
//		if (!v16 || !*v14)
//			goto LABEL_28;
//		*v15++ = *v14++;
//		--v17;
//		--v16;
//		++v18;
//	}
//	--v15;
//LABEL_28:
//	*v15 = 0;
//	if (v45 >= 0x10)
//		operator delete(v44);
//	v19 = CStringToken::nextToken(&st, v43);
//	if (*(_DWORD *)(v19 + 24) < 0x10u)
//		v20 = (const char *)(v19 + 4);
//	else
//		v20 = *(const char **)(v19 + 4);
//	v21 = atoi(v20);
//	v22 = v46;
//	v46->BannerOrder = v21;
//	if (v45 >= 0x10)
//		operator delete(v44);
//	v23 = CStringToken::nextToken(&st, v43);
//	if (*(_DWORD *)(v23 + 24) < 0x10u)
//		v24 = (const char *)(v23 + 4);
//	else
//		v24 = *(const char **)(v23 + 4);
//	v22->BannerDirection = atoi(v24);
//	if (v45 >= 0x10)
//		operator delete(v44);
//	v42 = (int *)v40;
//	CStringToken::nextToken(&st, v40);
//	CStringMethod::ConvertStringToDateTime(&v22->BannerStartDate);
//	v42 = &v39;
//	CStringToken::nextToken(&st, &v39);
//	CStringMethod::ConvertStringToDateTime(&v22->BannerEndDate);
//	v25 = CStringToken::nextToken(&st, v43);
//	if (*(_DWORD *)(v25 + 24) < 0x10u)
//		v26 = (char *)(v25 + 4);
//	else
//		v26 = *(char **)(v25 + 4);
//	v27 = v22->BannerLinkURL;
//	v28 = 2147483646;
//	v29 = 2084;
//	v30 = 0;
//	while (v29)
//	{
//		if (!v28 || !*v26)
//			goto LABEL_49;
//		*v27++ = *v26++;
//		--v29;
//		--v28;
//		++v30;
//	}
//	--v27;
//LABEL_49:
//	v31 = v45 < 0x10;
//	*v27 = 0;
//	if (!v31)
//		operator delete(v44);
//	v32 = v46;
//	v33 = v46->BannerImageURL;
//	strImageFile._Myres = 15;
//	strImageFile._Mysize = 0;
//	strImageFile._Bx._Buf[0] = 0;
//	std::string::assign(&strImageFile, v46->BannerImageURL, strlen(v46->BannerImageURL));
//	LOBYTE(v51) = 6;
//	v34 = std::string::rfind(&strImageFile, "/", 0xFFFFFFFF, 1u);
//	if (v34 != -1)
//	{
//		std::string::substr(&strImageFile, &strImageFileName, v34 + 1, strImageFile._Mysize - v34 - 1);
//		v35 = (std::string::_Bxty *)strImageFileName._Bx._Ptr;
//		if (strImageFileName._Myres < 0x10)
//			v35 = &strImageFileName._Bx;
//		v36 = (std::string::_Bxty *)strDirPath._Bx._Ptr;
//		if (strDirPath._Myres < 0x10)
//			v36 = &strDirPath._Bx;
//		v37 = v32->BannerImagePath;
//		StringCchPrintfA(v37, 0x104u, "%s%s", v36->_Buf, v35->_Buf);
//		if (bDonwLoad || GetFileAttributesA(v37) == -1)
//			URLDownloadToFileA(0, v33, v37, 0, 0);
//		if (strImageFileName._Myres >= 0x10)
//			operator delete(strImageFileName._Bx._Ptr);
//	}
//	if (strImageFile._Myres >= 0x10)
//		operator delete(strImageFile._Bx._Ptr);
//	strImageFile._Myres = 15;
//	strImageFile._Mysize = 0;
//	strImageFile._Bx._Buf[0] = 0;
//
//	CStringToken::~CStringToken(&st); 
//	if (strdata._Myres >= 0x10)
//		operator delete(strdata._Bx._Ptr);
//	strdata._Myres = 15;
//	strdata._Mysize = 0;
//	strdata._Bx._Buf[0] = 0;
//	if (strDirPath._Myres >= 0x10)
//		operator delete(strDirPath._Bx._Ptr);
	return 1;
}