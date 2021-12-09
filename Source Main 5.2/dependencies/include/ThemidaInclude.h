#ifndef _THEMIDA_INCLUDE_H_
#define _THEMIDA_INCLUDE_H_

#ifdef PKD_ADD_BINARY_PROTECTION 
#include "ThemidaSDK.h"
#else
#define VM_START
#define VM_END
// 여기에 사용하는 매크로 추가할 것  
#endif 

#endif // _THEMIDA_INCLUDE_H_
