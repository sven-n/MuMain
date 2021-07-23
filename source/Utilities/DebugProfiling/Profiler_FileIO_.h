// Profiler_FileIO_.h: interface for the CProfiler_FileIO_ class.
//
//////////////////////////////////////////////////////////////////////

#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING

#if !defined(AFX_PROFILER_FILEIO__H__8710B6B4_758B_4BBF_8BE4_BB944ED58DF6__INCLUDED_)
#define AFX_PROFILER_FILEIO__H__8710B6B4_758B_4BBF_8BE4_BB944ED58DF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <io.h>

class CProfiler_FileIO_
{
private:
	BOOL ExtractSpotLeftBuffer_Descending( const char	*szFullFileName,
		char			**szFindValues,
		const int		iValueCnt = 1,
		const int		iSkipDegree = 0,
		BOOL			bExceptionSpot = TRUE,
		char			*szReturnBuffer = 0 )
	{
		int		iSpot, 
				iLen, 
				iStrLen = 0, 
				iStrCurLen = 0, 
				iFindValueIdx = 0, 
				iSkipDegree_ = iSkipDegree;

		BOOL	bFind = FALSE;
		BOOL	bFindValue_OneTimeMore = FALSE;

		iLen = strlen( szFullFileName );

		for( int i = iLen; i >= 0; --i )
		{
			bFind = FALSE;
			if( FALSE == bFindValue_OneTimeMore )
			{
				for( int j = 0; j < iValueCnt; ++j )
				{
					iStrCurLen = strlen( szFindValues[j] );
					if( (strncmp( &szFullFileName[i], szFindValues[j], iStrCurLen ) == 0) && iStrCurLen > iStrLen )
					{
						iFindValueIdx = j;
						iStrLen = iStrCurLen;
						bFind = TRUE;
						bFindValue_OneTimeMore = TRUE;
					}
				}
			}
			else
			{
				if( (strncmp( &szFullFileName[i], szFindValues[iFindValueIdx], iStrLen ) == 0) )
				{
					bFind = TRUE;
				}
			}

			if( TRUE == bFind)
			{
				if( 1 > iSkipDegree_-- )	
				{
					break;
				}
			}
		}

		if( i < 0 ) return FALSE;

		if( 0 != szReturnBuffer )
		{
			iSpot = (TRUE==bExceptionSpot)? i : i + iStrLen;

			for( int i = 0; i < iSpot; ++i )
			{
				szReturnBuffer[i] = szFullFileName[i];
			}

			szReturnBuffer[iSpot] = '\0';
		}

		return TRUE;
	}

	BOOL ExtractFileExceptEx_( const char* szFullFileName,
		char* szReturnBuffer )
	{
		char		**szFindValues;
		BOOL		bReturn = FALSE;

		szFindValues = new char*[1];
		szFindValues[0] = new char[2];
		memset( szFindValues[0], 0, sizeof( char ) * 2 );
		sprintf( szFindValues[0], "%s", TEXT(".") );

		bReturn = ExtractSpotLeftBuffer_Descending( szFullFileName, 
			szFindValues, 		
			1,
			0, TRUE, szReturnBuffer );

		SAFE_DEL_ARR( szFindValues[0] );
		SAFE_DEL_ARR( szFindValues );

		return bReturn;
	}

	bool _hasfilename_dot(const char *szFileName)
	{
		char		**szFindValues;
		BOOL		bReturn = FALSE;

		szFindValues = new char*[1];
		szFindValues[0] = new char[2];
		memset( szFindValues[0], 0, sizeof( char ) * 2 );
		sprintf( szFindValues[0], "%s", TEXT(".") );

		bReturn = ExtractSpotLeftBuffer_Descending( szFileName, 
			szFindValues, 
			1,
			0, TRUE,
			0 );

		SAFE_DEL_ARR( szFindValues[0] );
		SAFE_DEL_ARR( szFindValues );

		return static_cast<bool>(bReturn);
	}

public:

	bool	_file_exist( const char* szFileName )
	{
		int iResult = 0;		

		iResult = _access( szFileName, 00);

		return ( iResult == 0 );	// 존재 하면 true
	}

	bool _file_save(	const char* szFileName, 
		const char* szBuffer,
		char* szCurrFileName = 0,
		ESAVEFILE_TYPE eSaveFileType = ESAVEFILE_NEW )
	{
		if( !szFileName ) return false;

		FILE			*pStream;

		char	szFullFileName[MAX_PATH];
		char	szFileName_[MAX_PATH];

		memset( szFullFileName, 0, sizeof(char) * MAX_PATH );
		memset( szFileName_, 0, sizeof(char) * MAX_PATH );

		int		iUnit = 1;

		if( !_hasfilename_dot( szFileName ) )
		{
			sprintf( szFileName_, "%s.txt", szFileName );
		}
		else
		{
			strcpy( szFileName_, szFileName );
		}

		switch( eSaveFileType )
		{
		case ESAVEFILE_NEW:
			{
				pStream = fopen( szFileName_, "w+t" );
			}
			break;
		case ESAVEFILE_NEW_NEXT:
			{
				char	szFileNameExceptEx_[MAX_PATH];
				memset( szFileNameExceptEx_, 0, sizeof(char) * MAX_PATH );
				while( !_file_exist( szFullFileName ) )
				{
					ExtractFileExceptEx_( szFileName_, szFileNameExceptEx_ );
					sprintf( szFullFileName, "%s_%d.txt", szFileNameExceptEx_, iUnit );
					++iUnit;
				}

				pStream = fopen( szFullFileName, "w+t" );
			}
			break;
		case ESAVEFILE_ACCORD:
			{
				pStream = fopen( szFileName_, "a+" );
			}
			break;
		default:
			{
				pStream = fopen( szFileName_, "w+t" );
			}
			break;
		}

		if( pStream == NULL )
		{
			return false;
		}

		if( szCurrFileName != 0 )
		{
			strcpy( szCurrFileName, szFullFileName );
		}

		fprintf( pStream, szBuffer );
		fclose( pStream );

		return true;
	}
};

#endif // !defined(AFX_PROFILER_FILEIO__H__8710B6B4_758B_4BBF_8BE4_BB944ED58DF6__INCLUDED_)

#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING