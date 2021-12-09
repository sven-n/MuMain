#ifndef _STL_DEFINITION_H_
#define _STL_DEFINITION_H_

#pragma warning( disable : 4786 )
#pragma warning( disable : 4800 ) 
#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <functional>

typedef std::vector<int> IntVector;
typedef std::vector<unsigned int> UIntVector;
typedef std::vector<unsigned short> UShortVector;
typedef std::vector<short> ShortVector;
typedef std::vector<std::string> StrVector;

typedef std::pair<unsigned int, unsigned int> UIntPair;
typedef std::vector<UIntPair> UIntPairVector;

typedef std::map<unsigned int, unsigned int> UIntUIntMap;
typedef std::map<unsigned int, std::string> UIntStringMap;
typedef std::map<int, std::string> IntStrMap;


#endif //_STL_DEFINITION_H_