#pragma once 

#include "Uploader.h"

/**
 * @class CUploaderSftp 
 *
 * SFTP 전송을 지원하기 위한 클래스이다. 
 */
class CUploaderSftp : public CUploader
{
public:
	CUploaderSftp(LPCTSTR szUrl, int nPort, LPCTSTR szId, LPCTSTR szPassword, int nUploadCount);
	~CUploaderSftp();

	/**
	 * 로컬 파일을 서버 파일로 전송한다.
	 * 
	 * 파일명은 루트 폴더부터 지정되어야 한다. 로컬/리모트 모두에 대해 그렇다.
	 * 예) UploadFTP( "c:\\Mu\\dump.dmp", "/Mu/crash_ABFFAABCDEDD.dmp" );
	 *
	 * 전체 경로를 사용하면 환경을 타지 않기 때문에 보다 안정적이다. 
	 *
	 * 같은 파일이 있으면 지워지도록 구현됨. 따라서, 중복되지 않도록 파일명 생성을 해야 함 
	 * crash_MAC_TIME.dmp 형식으로 만드는 게 좋을 듯 (IP는 로컬 아이피면 중복 가능. 따라서 랜 카드 주소가 나음)
	 *
	 * TODO: 위와 같은 내용을 반영하여 CrashHandler.cpp 수정할 것. 
	 */
	int	UploadFTP(LPCTSTR szLocalFile, LPCTSTR szServerFile);
};


	
