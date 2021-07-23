#pragma once
#define PSW_ONLY_EVENT			// 이벤트 전용 디파인 이걸로 디파인 걸지 말것.( 2008.03.30 )
#ifdef	PSW_ONLY_EVENT 

/*----------------------------------- 이벤트 --------------------------------------*/


// 07년 추석이벤트 (2007.08.24)
#define PRUARIN_EVENT07_3COLORHARVEST			

// 벚꽃놀이 이벤트(2008.03.12)
#define CSK_EVENT_CHERRYBLOSSOM							    
#ifdef CSK_EVENT_CHERRYBLOSSOM
	#define ADD_GLOBAL_MIX_MAR08						    // 해외 카오스카드, 벚꽃 조합 관련 스크립트 확장-국내에도 같이 확장 (2008.03.21) [mix.txt 패치해야됨]
	#define PSW_EVENT_CHERRYBLOSSOMTREE					    // 벚꽃나무 추가 국내 미적용 ( 2008.04.03 )
	#define PBG_FIX_GOLD_CHERRYBLOSSOMTREE					// 벚꽃나무 서버측과 젠 맞추는 버그 수정(2009.03.26)
#endif CSK_EVENT_CHERRYBLOSSOM

// 일본 전용 황금 궁수 이벤트
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
	#define PSW_GOLDBOWMAN									// 일본 전용 황금 궁수 복권 이벤트
	#define PSW_EVENT_LENA									// 일본 전용 황금 궁수 레나 이벤트
	#define YDG_ADD_GOLDBOWMAN_INVENTORY_SPACE_CHECK		// 일본 전용 황금 궁수 인벤토리 검사 (2*4) (2008.12.02)
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANES

/*--------------------------------- 이벤트 맵 --------------------------------------*/

#endif //PSW_ONLY_EVENT