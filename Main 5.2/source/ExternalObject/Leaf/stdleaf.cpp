
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "stdleaf.h"

// GetTimeString
#include <time.h>
#include <sys/timeb.h>

bool leaf::CreateDirectoryIncSub(const std::string& path) {
	
	for(int i=1; i<(int)path.size(); i++) {
		if(path[i] == '/' || path[i] == '\\') {
			std::string subpath = path.substr(0, i+1/* length */);
			
			if(GetFileAttributes(subpath.c_str()) == 0xFFFFFFFF) {
				if(!CreateDirectory(subpath.c_str(), NULL))
					return false;
			}
		}
		else if(i == (int)(path.size())-1) {
			if(GetFileAttributes(path.c_str()) == 0xFFFFFFFF) {
				if(!CreateDirectory(path.c_str(), NULL))
					return false;
			}
		}
	}
	return true;
}
bool leaf::DeleteDirectoryIncSub(const std::string& path) {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	std::string	finddir = path;
	finddir += "/*";
	hFind = FindFirstFile(finddir.data(), &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
		return false;
	
	HRESULT hr = true;
	do{
		std::string filename = path;
		filename += '/';
		filename += FindFileData.cFileName;
		
		if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
			hr = DeleteFile(filename.data());
		
		else if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			if(stricmp(FindFileData.cFileName, ".") && stricmp(FindFileData.cFileName, ".."))
				hr = DeleteDirectoryIncSub(filename.data());
			
			if(!hr){
				FindClose(hFind);
				return false;
			}
	}
	while(FindNextFile(hFind, &FindFileData));
	
	FindClose(hFind);
	
	if(!RemoveDirectory(path.c_str()))
		return false;
	
	return true;
}

bool leaf::GetFileSizeQW(const HANDLE hFile, QWORD &qwSize) 
{
	if(hFile == NULL)
		return false;
	
	DWORD dwLo = 0, dwHi = 0;
	dwLo = GetFileSize(hFile, &dwHi);
	
	if(dwLo == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
		qwSize = 0;
		return false;
	}
	qwSize = MAKEQWORD(dwLo, dwHi);
	return true;
}

void leaf::GetAbsolutePath(IN const std::string& path, OUT std::string& abspath) {
	char* pszFileName = NULL;
	char szFullPath[_MAX_PATH] = {0, };
	GetFullPathName(path.c_str(), _MAX_PATH, szFullPath, &pszFileName);
	abspath = szFullPath;
}
void leaf::GetAbsoluteFilePath(IN const std::string& path, OUT std::string& abspath, OUT std::string& filename) {
	char* pszFileName = NULL;
	char szFullPath[_MAX_PATH] = {0, };
	GetFullPathName(path.c_str(), _MAX_PATH, szFullPath, &pszFileName);
	abspath = szFullPath;
	filename = pszFileName;
}

void leaf::SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt) {
	char __fname[_MAX_FNAME] = {0, };
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, __fname, __ext);
	filename = __fname;
	if(bIncludeExt)
		filename += __ext;
}
void leaf::SplitDirectoryPath(IN const std::string& filepath, OUT std::string& dir) {
	char __drive[_MAX_DRIVE] = {0, };
	char __dir[_MAX_DIR] = {0, };
	_splitpath(filepath.c_str(), __drive, __dir, NULL, NULL);
	dir = __drive;
	dir += __dir;
}
void leaf::SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot) {
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, NULL, __ext);
	if(bIncludeDot) {
		ext = __ext;
	}
	else {
		if((__ext[0] == '.') && __ext[1])
			ext = __ext+1;
	}
}

void leaf::ExtractDirectoryName(IN const std::string& path, OUT std::string& dirname) {
	int start = 0;
	int end = path.size()-1;
	for(int i=end; i>=start; i--) {
		if(path[i] == '\\' || path[i] == '/') {
			if(i == end) {
				end = i-1;
			}
			else {
				start = i+1;
				dirname = path.substr(start, end-start+1);
				break;
			}
		}
		else if(i == start) {
			dirname = path.substr(start, end-start+1);
		}
	}
}
void leaf::ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath) {
	char __drive[_MAX_DRIVE] = {0, };
	char __dir[_MAX_DIR] = {0, };
	char __fname[_MAX_FNAME] = {0, };
	_splitpath(in_filepath.c_str(), __drive, __dir, __fname, NULL);
	
	out_filepath = __drive;
	out_filepath += __dir;
	out_filepath += __fname;
	out_filepath += '.';
	out_filepath += ext;
}

bool leaf::CompareFilePath(IN const std::string& path1, IN const std::string& path2, size_t size)
{
	bool bIdentity = true;
	for(int i=0; i<(int)size; i++) {
		if((path1[i] == '\\' || path1[i] == '/') && 
			(path2[i] == '\\' || path2[i] == '/')) continue;
		if(_tolower(path1[i]) != _tolower(path2[i])) {
			bIdentity = false;
			break;
		}
	}
	return bIdentity;
}
void leaf::AppendFilePath(IN const std::string& dir, IN const std::string& to_append, OUT std::string& out_path)
{
	if(dir.empty()) {	// 'dir' is empty.
		if(!to_append.empty()) {	//. 'to_append' is NOT empty.
			char ch = to_append[0];
			if(ch == '\\' || ch == '/')
				out_path = to_append.substr(1,to_append.size()-1);
			else
				out_path = to_append;
		}
	}
	else {	// 'dir' is NOT empty.
		char ch = dir[dir.size()-1];
		if(ch == '\\' || ch == '/') {
			out_path = dir.substr(0,dir.size()-1);
		}
		else {
			out_path = dir;
		}

		if(!to_append.empty()) {	// 'to_append' is NOT empty.
			ch = to_append[0];
			if(ch != '\\' && ch != '/')
				out_path += '\\';
			out_path += to_append;
		}
	}
}

bool leaf::GetFileCreationTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime) {
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr == 0xFFFFFFFF)
		return false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	FILETIME ftime = FindFileData.ftCreationTime;
	if(bLocalTime)
		FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &ftime);

	FileTimeToSystemTime(&ftime, &systime);

	return true;
}
bool leaf::GetFileLastModifiedTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime) {
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr == 0xFFFFFFFF || dwAttr == FILE_ATTRIBUTE_DIRECTORY)
		return false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	FILETIME ftime = FindFileData.ftLastWriteTime;
	if(bLocalTime)
		FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &ftime);

	FileTimeToSystemTime(&ftime, &systime);

	return true;
}
bool leaf::GetFileLastAccessedTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime) {
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr == 0xFFFFFFFF || dwAttr == FILE_ATTRIBUTE_DIRECTORY)
		return false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	FILETIME ftime = FindFileData.ftLastAccessTime;
	if(bLocalTime)
		FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &ftime);

	FileTimeToSystemTime(&ftime, &systime);

	return true;
}

const char* leaf::FormatString(const char* pFormat, ...)
{
	static char buf[8][2048];
	static DWORD index = 0;

	va_list marker;
	va_start(marker, pFormat);
	vsprintf(buf[index&7], pFormat, marker);
	va_end(marker);

	return ((const char*)buf[index++ & 7]);
}
void leaf::GetTimeString(OUT std::string& str) {
	_timeb ftime;
	_ftime(&ftime);
	tm* plocaltime = localtime(&ftime.time);
	
	char szTime[256] = {0, };
	sprintf(szTime, "%d/%d/%d %d:%02d:%02d:%003d", 1900+plocaltime->tm_year,plocaltime->tm_mon+1,plocaltime->tm_mday, 
		plocaltime->tm_hour,plocaltime->tm_min,plocaltime->tm_sec,ftime.millitm);
	str = szTime;
}