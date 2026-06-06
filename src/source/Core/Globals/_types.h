#pragma once

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec34_t[3][4];

template< typename T > inline
void InitVector(T* vect, int size) {
    for (int i = 0; i < size; ++i) {
        vect[i] = 0.0f;
    }
}

template< typename T > inline
void IdentityMatrix(T(*mat)[4]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            mat[i][j] = 0;
        }
    }
}

template< typename T > inline
void IdentityVector2D(T* vect) {
    InitVector(vect, 2);
}

template< typename T > inline
void IdentityVector3D(T* vect) {
    InitVector(vect, 3);
}

typedef char* PCHAR;
typedef char			CHAR;
typedef std::string		STRING;

typedef wchar_t* PWCHAR;
typedef wchar_t			WCHAR;
typedef std::wstring	WSTRING;
typedef std::map<eBuffState, DWORD>    BuffStateMap;
