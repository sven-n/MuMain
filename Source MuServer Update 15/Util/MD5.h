#pragma once

#include <windows.h> 
#include <string> 
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h> 


using namespace std;

// first, some types:
  typedef unsigned       int uint4; // assumes integer is 4 words long
  typedef unsigned short int uint2; // assumes short integer is 2 words long
  typedef unsigned      char uint1; // assumes char is 1 word long

class MD5 
{
public:

//---------------------------------------------
//	MD5 추가된 메서드들
bool MD5_EncodeKeyVal	(					// 입력된 문자열과 키인덱스 (0~255)를 사용하여 128 bits (16 bytes) 의 MD5 키값 생성
	  char * lpszInputStr,					// 입력 문자열
	  char * lpszOutputKeyVal,				// 출력 버퍼
	  int iKeyIndex							// 키인덱스 (0~255)
	  );

bool MD5_EncodeString	(					// 입력된 문자열과 키인덱스 (0~255)를 사용하여 128 x 2 bits (32 bytes) 의 MD5 문자열 키값 생성
	  char * lpszInputStr,					// 입력 문자열
	  char * lpszOutputStr,					// 출력 버퍼
	  int iKeyIndex							// 키인덱스 (0~255)
	  );

bool MD5_CheckValue	(						// 문자열과 MD5 키값과 키인덱스 (0~255) 를 입력받아 키값을 인증함 (true : 맞음 / false : 틀림)
											// P.S.> 키값은 문자열(256 bits)이 아닌 128 bits MD5 키여야 한다.
	  char * lpszInputStr,					// 입력 문자열
	  char * szKeyVal,						// 입력 MD5 키값
	  int iKeyIndex							// 키인덱스 (0~255)
	  );


//---------------------------------------------
//	MD5 기존 메서드들

// methods for controlled operation:
  MD5              ();  // simple initializer
  void  update     (uint1 *input, uint4 input_length);
  void  update     (std::istream& stream);
  void  update     (FILE *file);
  void  update     (std::ifstream& stream);
  void  finalize   ();

// constructors for special circumstances.  All these constructors finalize
// the MD5 context.
  MD5              (unsigned char *string); // digest string, finalize
  MD5              (std::istream& stream);       // digest stream, finalize
  MD5              (FILE *file);            // digest file, close, finalize
  MD5              (std::ifstream& stream);      // digest stream, close, finalize

// methods to acquire finalized result
  unsigned char    *raw_digest ();  // digest as a 16-byte binary array
  char *            hex_digest ();  // digest as a 33-byte ascii-hex string
  friend std::ostream&   operator<< (std::ostream&, MD5 context);



private:

//---------------------------------------------
//	MD5 추가된 메서드들
  void setmagicnum (int keyindex);


//---------------------------------------------
//	MD5 추가된 멤버 변수들
	unsigned char	m_cRaw_digest[16];  
	char			m_cHex_digest[33];  



//---------------------------------------------
//	MD5 기존 멤버 변수들



// next, the private data:
  uint4 state[4];
  uint4 count[2];     // number of *bits*, mod 2^64
  uint1 buffer[64];   // input buffer
  uint1 digest[16];
  uint1 finalized;

// last, the private methods, mostly static:
  void init             ();               // called by all constructors
  void transform        (uint1 *buffer);  // does the real update work.  Note 
                                          // that length is implied to be 64.

  static void encode    (uint1 *dest, uint4 *src, uint4 length);
  static void decode    (uint4 *dest, uint1 *src, uint4 length);
  static void memcpy    (uint1 *dest, uint1 *src, uint4 length);
  static void memset    (uint1 *start, uint1 val, uint4 length);

  static inline uint4  rotate_left (uint4 x, uint4 n);
  static inline uint4  F           (uint4 x, uint4 y, uint4 z);
  static inline uint4  G           (uint4 x, uint4 y, uint4 z);
  static inline uint4  H           (uint4 x, uint4 y, uint4 z);
  static inline uint4  I           (uint4 x, uint4 y, uint4 z);
  static inline void   FF  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
			    uint4 s, uint4 ac);
  static inline void   GG  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
			    uint4 s, uint4 ac);
  static inline void   HH  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
			    uint4 s, uint4 ac);
  static inline void   II  (uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, 
			    uint4 s, uint4 ac);

};


// Constants for MD5Transform routine.
// Although we could use C++ style constants, defines are actually better,
// since they let us easily evade scope clashes.

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

