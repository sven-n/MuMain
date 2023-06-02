/*******************************************************************************
*	작 성 자 : 진혜진
*	작 성 일 : 2009.06.10
*	내    용 : 기타 메소드
*******************************************************************************/

#pragma once

class Path
{
public:

    //					모듈 전체 경로 가져오기
    static TCHAR* GetCurrentFullPath(TCHAR* szPath);
    //					모듈 디렉토리 가져오기
    static TCHAR* GetCurrentDirectory(TCHAR* szPath);
    //					모듈 파일 이름 가져오기
    static TCHAR* GetCurrentFileName(TCHAR* szPath);

    //					폴더 문자열 만들기 : 맨 뒤에 "\\" 붙여준다.
    static TCHAR* SetDirString(TCHAR* szPath);
    //					폴더 문자열 만들기 : 맨 뒤에 "\\" 제거
    static TCHAR* ClearDirString(TCHAR* szPath);

    //					폴더 문자열 만들기 : 파일명 제거한 경로
    static TCHAR* GetDirectory(TCHAR* szPath);
    //					파일 문자열 만들기 : 패스 제거한 파일 명
    static TCHAR* GetFileName(TCHAR* szPath);

    //					/ => \\ 로 변경
    static TCHAR* ChangeSlashToBackSlash(TCHAR* szPath);
    //					\\ => / 로 변경
    static TCHAR* ChangeBackSlashToSlash(TCHAR* szPath);

    //					파일에서 마지막 줄 읽어오기
    static BOOL			ReadFileLastLine(TCHAR* szFile, TCHAR* szLastLine);
    //					새 파일에 한줄 쓰기
    static BOOL			WriteNewFile(TCHAR* szFile, TCHAR* szText, INT nTextSize);
    //					파일 경로 디렉토리 생성
    static BOOL			CreateDirectorys(TCHAR* szFilePath, BOOL bIsFile);
};
