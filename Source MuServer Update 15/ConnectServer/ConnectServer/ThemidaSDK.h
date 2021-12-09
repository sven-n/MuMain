/****************************************************************************** 
/* Header: ThemidaSDK.h
/* Description: SDK header definition for the C/C++ language
/*
/* Author/s: Rafael Ahucha  
/* (c) 2004 Oreans Technologies
/*****************************************************************************/ 


#ifndef __THEMIDASDK__

#define __THEMIDASDK__


/* Definition for Borland C/C++  */

#ifdef __BORLANDC__
 
 #define CODEREPLACE_START __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define CODEREPLACE_END  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x01, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
  
 #define ENCODE_START     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x04, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define ENCODE_END       __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x05, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define CLEAR_START      __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x06, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define CLEAR_END        __emit__ (0xEB, 0x15, 0x57, 0x4C, 0x20, 0x20, 0x07, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, \
                                    0x00, 0x00, 0x00);

 #define VM_START     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define VM_END       __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0D, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define VM_START_WITHLEVEL(x)     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, x, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define UNPROTECTED_START     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define UNPROTECTED_END       __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x21, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define CHECK_PROTECTION(var, val) \
 asm {  \
   dw    0x10EB; \
   dd    0x091ab3167; \
   dd    0x08a8b717a; \
   dd    0x0bc117abd; \
   dd    0x0; \
   push  val; \
   pop   var; \
   dw    0x0CEB; \
   dd    0x0bc117abd; \
   dd    0x08a8b717a; \
   dd    0x091ab3167; \
}

 #define CHECK_CODE_INTEGRITY(var, val) \
 asm {  \
   dw    0x10EB; \
   dd    0x091ab3167; \
   dd    0x08a8b717a; \
   dd    0x0bc117abd; \
   dd    0x1; \
   push  val; \
   pop   var; \
   dw    0x0CEB; \
   dd    0x0bc117abd; \
   dd    0x08a8b717a; \
   dd    0x091ab3167; \
}

 #define CHECK_VIRTUAL_PC(var, val) \
 asm {  \
   dw    0x10EB; \
   dd    0x091ab3167; \
   dd    0x08a8b717a; \
   dd    0x0bc117abd; \
   dd    0x3; \
   push  val; \
   pop   var; \
   dw    0x0CEB; \
   dd    0x0bc117abd; \
   dd    0x08a8b717a; \
   dd    0x091ab3167; \
}

 #define __WL_MACROS__

 #endif
 
#endif

/******* GNUC (MinGW) Compatible compiler ********/

#ifdef __GNUC__
#define NO_OPTIMIZATION __attribute__((optimize("O0")))

 #define CODEREPLACE_START \
 asm ( ".byte 0xEB\n"\
       ".byte 0x10\n"\
       ".byte 0x57\n"\
       ".byte 0x4C\n"\
       ".byte 0x20\n"\
       ".byte 0x20\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x00\n"\
       ".byte 0x57\n"\
       ".byte 0x4C\n"\
       ".byte 0x20\n"\
       ".byte 0x20\n"\
     );

#define CODEREPLACE_END \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x01\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define ENCODE_START \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x04\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define ENCODE_END \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x05\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define CLEAR_START \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x06\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define CLEAR_END \
asm ( ".byte 0xEB\n"\
      ".byte 0x15\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x07\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
    );

#define VM_START \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x0C\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define VM_END \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x0D\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define VM_START_WITHLEVEL(x) \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x0C\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte $" #x "\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define UNPROTECTED_START \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );


#define UNPROTECTED_END \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
      ".byte 0x21\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x57\n"\
      ".byte 0x4C\n"\
      ".byte 0x20\n"\
      ".byte 0x20\n"\
    );

#define CHECK_PROTECTION(var, val) \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x67\n"\
      ".byte 0x31\n"\
      ".byte 0xAB\n"\
      ".byte 0x91\n"\
      ".byte 0x7A\n"\
      ".byte 0x71\n"\
      ".byte 0x8B\n"\
      ".byte 0x8A\n"\
      ".byte 0xBD\n"\
      ".byte 0x7A\n"\
      ".byte 0x11\n"\
      ".byte 0xBC\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      "push  $" #val "\n"); \
      __asm__  ("pop %0" : "=m" (var):); \
      asm (".byte 0xEB\n"\
      ".byte 0x0C\n"\
      ".byte 0xBD\n"\
      ".byte 0x7A\n"\
      ".byte 0x11\n"\
      ".byte 0xBC\n"\
      ".byte 0x7A\n"\
      ".byte 0x71\n"\
      ".byte 0x8B\n"\
      ".byte 0x8A\n"\
      ".byte 0x67\n"\
      ".byte 0x31\n"\
      ".byte 0xAB\n"\
      ".byte 0x91\n"\
      );

#define CHECK_CODE_INTEGRITY(var, val) \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x67\n"\
      ".byte 0x31\n"\
      ".byte 0xAB\n"\
      ".byte 0x91\n"\
      ".byte 0x7A\n"\
      ".byte 0x71\n"\
      ".byte 0x8B\n"\
      ".byte 0x8A\n"\
      ".byte 0xBD\n"\
      ".byte 0x7A\n"\
      ".byte 0x11\n"\
      ".byte 0xBC\n"\
      ".byte 0x01\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      "push  $" #val "\n"); \
      __asm__  ("pop %0" : "=m" (var):); \
      asm (".byte 0xEB\n"\
      ".byte 0x0C\n"\
      ".byte 0xBD\n"\
      ".byte 0x7A\n"\
      ".byte 0x11\n"\
      ".byte 0xBC\n"\
      ".byte 0x7A\n"\
      ".byte 0x71\n"\
      ".byte 0x8B\n"\
      ".byte 0x8A\n"\
      ".byte 0x67\n"\
      ".byte 0x31\n"\
      ".byte 0xAB\n"\
      ".byte 0x91\n"\
      );

#define CHECK_VIRTUAL_PC(var, val) \
asm ( ".byte 0xEB\n"\
      ".byte 0x10\n"\
      ".byte 0x67\n"\
      ".byte 0x31\n"\
      ".byte 0xAB\n"\
      ".byte 0x91\n"\
      ".byte 0x7A\n"\
      ".byte 0x71\n"\
      ".byte 0x8B\n"\
      ".byte 0x8A\n"\
      ".byte 0xBD\n"\
      ".byte 0x7A\n"\
      ".byte 0x11\n"\
      ".byte 0xBC\n"\
      ".byte 0x03\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      ".byte 0x00\n"\
      "push  $" #val "\n"); \
      __asm__  ("pop %0" : "=m" (var):); \
      asm (".byte 0xEB\n"\
      ".byte 0x0C\n"\
      ".byte 0xBD\n"\
      ".byte 0x7A\n"\
      ".byte 0x11\n"\
      ".byte 0xBC\n"\
      ".byte 0x7A\n"\
      ".byte 0x71\n"\
      ".byte 0x8B\n"\
      ".byte 0x8A\n"\
      ".byte 0x67\n"\
      ".byte 0x31\n"\
      ".byte 0xAB\n"\
      ".byte 0x91\n"\
      );

#define __WL_MACROS__

#endif

/* Definition for Intel cpp compiler */
 
#ifndef __WL_MACROS__
 
 #ifdef __ICL
 
 #define CODEREPLACE_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
 
 #define CODEREPLACE_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x01 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
 
 #define ENCODE_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x04 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define ENCODE_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x05 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define CLEAR_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x06 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
 
 #define CLEAR_END \
  __asm __emit 0xEB \
  __asm __emit 0x15 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x07 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 

  #define VM_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x0C \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define VM_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x0D \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \

  #define VM_START_WITHLEVEL(x) \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x0C \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit x \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \

  #define UNPROTECTED_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define UNPROTECTED_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x21 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \

 #define __WL_MACROS__
 
 #endif

#endif
 
 
/* Definition for LCC by Jacob Navia */
 
#ifndef __WL_MACROS__
 
 #ifdef __LCC__
 
 #define CODEREPLACE_START __asm__ (" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 #define CODEREPLACE_END  __asm__ (" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x01, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
  
 #define ENCODE_START     __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x04, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 #define ENCODE_END       __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x05, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 
 #define CLEAR_START      __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x06, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 
 #define CLEAR_END        __asm__(" .byte\t0xEB, 0x15, 0x57, 0x4C, 0x20, 0x20, 0x07, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, \
                                    0x00, 0x00, 0x00");
 #define VM_START         __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define VM_END           __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0D, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define VM_START_WITHLEVEL(x) __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, x, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define UNPROTECTED_START         __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define UNPROTECTED_END           __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x21, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define __WL_MACROS__
 
 #endif
 
#endif
 

/* Definition for Visual C++ and others */

#ifndef __WL_MACROS__
 
 #define CODEREPLACE_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
 #define CODEREPLACE_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x01 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
 #define ENCODE_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x04 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
 #define ENCODE_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x05 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
 #define CLEAR_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x06 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
 #define CLEAR_END \
  __asm _emit 0xEB \
  __asm _emit 0x15 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x07 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \

 #define VM_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x0C \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
 #define VM_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x0D \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \

 #define VM_START_WITHLEVEL(x) \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x0C \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit x \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \


 #define UNPROTECTED_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \

 #define UNPROTECTED_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x21 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \

  #define CHECK_PROTECTION(var, val) \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x67 \
  __asm _emit 0x31 \
  __asm _emit 0xAB \
  __asm _emit 0x91 \
  __asm _emit 0x7A \
  __asm _emit 0x71 \
  __asm _emit 0x8B \
  __asm _emit 0x8A \
  __asm _emit 0xBD \
  __asm _emit 0x7A \
  __asm _emit 0x11 \
  __asm _emit 0xBC \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm push  val \
  __asm pop   var \
  __asm _emit 0xEB \
  __asm _emit 0x0C \
  __asm _emit 0xBD \
  __asm _emit 0x7A \
  __asm _emit 0x11 \
  __asm _emit 0xBC \
  __asm _emit 0x7A \
  __asm _emit 0x71 \
  __asm _emit 0x8B \
  __asm _emit 0x8A \
  __asm _emit 0x67 \
  __asm _emit 0x31 \
  __asm _emit 0xAB \
  __asm _emit 0x91 \

  #define CHECK_CODE_INTEGRITY(var, val) \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x67 \
  __asm _emit 0x31 \
  __asm _emit 0xAB \
  __asm _emit 0x91 \
  __asm _emit 0x7A \
  __asm _emit 0x71 \
  __asm _emit 0x8B \
  __asm _emit 0x8A \
  __asm _emit 0xBD \
  __asm _emit 0x7A \
  __asm _emit 0x11 \
  __asm _emit 0xBC \
  __asm _emit 0x01 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm push  val \
  __asm pop   var \
  __asm _emit 0xEB \
  __asm _emit 0x0C \
  __asm _emit 0xBD \
  __asm _emit 0x7A \
  __asm _emit 0x11 \
  __asm _emit 0xBC \
  __asm _emit 0x7A \
  __asm _emit 0x71 \
  __asm _emit 0x8B \
  __asm _emit 0x8A \
  __asm _emit 0x67 \
  __asm _emit 0x31 \
  __asm _emit 0xAB \
  __asm _emit 0x91 \

  #define CHECK_VIRTUAL_PC(var, val) \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x67 \
  __asm _emit 0x31 \
  __asm _emit 0xAB \
  __asm _emit 0x91 \
  __asm _emit 0x7A \
  __asm _emit 0x71 \
  __asm _emit 0x8B \
  __asm _emit 0x8A \
  __asm _emit 0xBD \
  __asm _emit 0x7A \
  __asm _emit 0x11 \
  __asm _emit 0xBC \
  __asm _emit 0x03 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm push  val \
  __asm pop   var \
  __asm _emit 0xEB \
  __asm _emit 0x0C \
  __asm _emit 0xBD \
  __asm _emit 0x7A \
  __asm _emit 0x11 \
  __asm _emit 0xBC \
  __asm _emit 0x7A \
  __asm _emit 0x71 \
  __asm _emit 0x8B \
  __asm _emit 0x8A \
  __asm _emit 0x67 \
  __asm _emit 0x31 \
  __asm _emit 0xAB \
  __asm _emit 0x91 \

#endif
