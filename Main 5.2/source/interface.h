#ifdef INFO_BUILDER

#pragma once

#include "_struct.h"

//#define INFOFILEDATA_TEMPLATE   template<typename DATA>

namespace concept
{

struct Interface
{
    virtual ~Interface(void) = 0{}
};

};

namespace info
{
class InfoFile : public concept::Interface
{
public:
	InfoFile( const string& filename ) : m_FileName( filename ) {}
	virtual ~InfoFile(void) = 0{}

public:	
	virtual bool isopenfile() = 0;
	virtual void clear() = 0;

public:
	const char*	GetinfoFileName() const { return m_FileName.c_str(); }

private:
	string	m_FileName;
};

};

#endif //INFO_BUILDER