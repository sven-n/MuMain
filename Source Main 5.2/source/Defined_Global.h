


#ifdef _DEBUG
	#define KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK
#endif // _DEBUG

#define NEW_PROTOCOL_SYSTEM

#define PJH_NEW_SERVER_SELECT_MAP

#define ASG_ADD_GENS_SYSTEM
#ifdef ASG_ADD_GENS_SYSTEM
	#define ASG_ADD_INFLUENCE_GROUND_EFFECT
	#define ASG_ADD_GENS_MARK
	#define PBG_MOD_STRIFE_GENSMARKRENDER
	#define PBG_ADD_GENSRANKING
#endif	// ASG_ADD_GENS_SYSTEM

//----------------------------------------------------------------------------------------------
// << Season 5 - Part 1 

#define UPDATE_100527
#ifdef UPDATE_100527

	#define SEASON5_PART2
	#ifdef SEASON5_PART2
		#define KJH_PBG_ADD_INGAMESHOP_SYSTEM
		#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
			#define PBG_ADD_INGAMESHOP_UI_MAINFRAME
			#define PBG_ADD_INGAMESHOP_UI_ITEMSHOP
			#define PBG_ADD_NAMETOPMSGBOX
			#define KJH_ADD_INGAMESHOP_UI_SYSTEM
			#define KJH_ADD_PERIOD_ITEM_SYSTEM
			#define PBG_ADD_INGAMESHOPMSGBOX
			#define PBG_ADD_MSGBACKOPACITY
			#define PBG_ADD_ITEMRESIZE
			#define PBG_FIX_ITEMANGLE
			#define PBG_ADD_MU_LOGO
			#define KJH_MOD_SHOP_SCRIPT_DOWNLOAD
			//^#define PBG_ADD_CHARACTERSLOT
			#define PBG_ADD_CHARACTERCARD
			//^#define LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
		#endif //KJH_PBG_ADD_INGAMESHOP_SYSTEM

	#endif // SEASON5_PART2

	#define ADD_TOTAL_CHARGE_8TH
	#ifdef  ADD_TOTAL_CHARGE_8TH
		#define YDG_ADD_HEALING_SCROLL						// Ä¡À¯ÀÇ ½ºÅ©·Ñ (2009.12.03)

		#define LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12		// Èñ±Í¾ÆÀÌÅÛ Æ¼ÄÏ 7-12 Ãß°¡ (2010.02.12)
		#define LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH	//µµÇÃ°»¾î, ¹¸£Ä«, ¹¸£Ä« Á¦ 7¸Ê ÀÚÀ¯ÀÔÀå±Ç Ãß°¡(2010.02.17)

		#define ADD_SKELETON_PET							// ½ºÄÌ·¹Åæ Æê (2009.12.03)
		#ifdef  ADD_SKELETON_PET
			#define YDG_ADD_SKELETON_PET					// ½ºÄÌ·¹Åæ Æê 
			#ifndef KJH_ADD_PERIOD_ITEM_SYSTEM					// #ifndef
				#define LJH_FIX_IGNORING_EXPIRATION_PERIOD		// ÀÏº»¿¡´Â ¾ÆÁ÷ ±â°£Á¦°¡ Àû¿ëµÇÁö ¾ÊÀ¸¹Ç·Î bExpiredPeriod¸¦ »ç¿ëÇÏÁö ¾Êµµ·Ï ¼öÁ¤(2010.02.11) - ±â°£Á¦ ½Ã½ºÅÛ µµÀÔ ÈÄ »èÁ¦
			#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
			#define LDK_FIX_PC4_GUARDIAN_DEMON_INFO			// µ¥¸ó Ä³¸¯ÅÍÁ¤º¸Ã¢¿¡ °ø°Ý·Â,¸¶·Â,ÀúÁÖ·Â Ãß°¡ µ¥¹ÌÁö Àû¿ë(09.10.15) - Ä³¸¯ÅÍÁ¤º¸Ã¢¿¡ Æê Á¾·¿¡ ÀÇÇÑ ÀúÁÖ·Â Ãß°¡ µ¥¹ÌÁö¸¦ Àû¿ë ½ÃÅ°±â À§ÇØ Ãß°¡
			#define LJH_FIX_NOT_POP_UP_HIGHVALUE_MSGBOX_FOR_BANNED_TO_TRADE	//NPC¿¡°Ô ÆÇ¸Å ±ÝÁöµÈ ¾ÆÀÌÅÛÀ» ÆÇ¸ÅÇÏ·Á°í ÇÒ¶§ °í°¡ÀÇ ¾ÆÀÌÅÛÀÌ¶ó°í ³ª¿À´Â ¸Þ¼¼Áö Ã¢À» ¾È³ª¿À°Ô ¼öÁ¤(10.02.19)
		#endif  //ADD_SKELETON_PET

		#define YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT			// º¯½Å¹ÝÁö ÇÑÁ¾·¸¸ ÀåºñÇÏµµ·Ï ¼öÁ¤ (2009.12.28)
	#endif  //ADD_TOTAL_CHARGE_8TH


	//-----------------------------------------------------------------------------
	// [ Season 5 - Part 1] (09.09.11)
	#define SEASON5										
	#ifdef SEASON5
		#define YDG_ADD_DOPPELGANGER_EVENT
		#ifdef YDG_ADD_DOPPELGANGER_EVENT
			#define YDG_ADD_MAP_DOPPELGANGER1					// µµÇÃ°»¾î ¸Ê1 (¶óÅ¬¸®¿Âº£ÀÌ½º) (2009.03.22)
			#define YDG_ADD_MAP_DOPPELGANGER2					// µµÇÃ°»¾î ¸Ê2 (ºÒÄ«´©½ºº£ÀÌ½º) (2009.03.22)
			#define YDG_ADD_MAP_DOPPELGANGER3					// µµÇÃ°»¾î ¸Ê3 (¾ÆÆ²¶õ½ºº£ÀÌ½º) (2009.03.22)
			#define YDG_ADD_MAP_DOPPELGANGER4					// µµÇÃ°»¾î ¸Ê4 (Ä­Åõ¸£1Â÷º£ÀÌ½º) (2009.03.22)
			#define YDG_ADD_DOPPELGANGER_MONSTER				// µµÇÃ°»¾î ¸ó½ºÅÍ Ãß°¡ (2009.05.20)
			#define YDG_ADD_DOPPELGANGER_NPC					// µµÇÃ°»¾î NPC ·ç°¡µå Ãß°¡ (2009.05.20)
			#define YDG_ADD_DOPPELGANGER_ITEM					// µµÇÃ°»¾î ¾ÆÀÌÅÛ Ãß°¡ (2009.05.20)
			#define YDG_ADD_DOPPELGANGER_UI						// µµÇÃ°»¾î UI Ãß°¡ (2009.05.20)
			#define YDG_ADD_DOPPELGANGER_PROTOCOLS				// µµÇÃ°»¾î ÇÁ·ÎÅäÄÝ Ãß°¡ (2009.06.23)
			#define YDG_ADD_DOPPELGANGER_PORTAL					// µµÇÃ°»¾î ¸Ê Æ÷Å» Ãß°¡ (2009.07.30)
			#define YDG_ADD_DOPPELGANGER_SOUND					// µµÇÃ°»¾î »ç¿îµå Ãß°¡ (2009.08.04)
		#endif	// YDG_ADD_DOPPELGANGER_EVENT

		#define LDS_ADD_EMPIRE_GUARDIAN
		#ifdef LDS_ADD_EMPIRE_GUARDIAN
			#define LDS_ADD_MAP_EMPIREGUARDIAN2					// Á¦±¹ ¼öÈ£±º ¸Ê 2   (È­,  ±Ý)
			#define LDS_ADD_MAP_EMPIREGUARDIAN4					// Á¦±¹ ¼öÈ£±º ¸Ê 2   (ÀÏ	  )
			#define LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN	// Á¦±¹ ¼öÈ£±º ¸Ê 4   (ÀÏ	  )¸ó½ºÅÍ ¿µå º¸½º °¡ÀÌ¿Â Ä«·¹ÀÎ	(504/164)
			#define LDS_ADD_EG_4_MONSTER_JELINT					// Á¦±¹ ¼öÈ£±º ¸Ê 4   (ÀÏ	  )¸ó½ºÅÍ °¡ÀÌ¿Â º¸ÁÂ°ü Á¦¸°Æ®		(505/165)
			#define LDS_ADD_EG_3_4_MONSTER_RAYMOND				// Á¦±¹ ¼öÈ£±º ¸Ê 3,4 (¼öÅä,ÀÏ)¸ó½ºÅÍ ºÎ»ç·É°ü ·¹ÀÌ¸óµå			(506/166)
			#define LDS_ADD_EG_2_4_MONSTER_ERCANNE				// Á¦±¹ ¼öÈ£±º ¸Ê 2,4 (È­±Ý,ÀÏ)¸ó½ºÅÍ ÁöÈÖ°ü ¿¡¸£Ä­´À			(507/167)
			#define LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// Á¦±¹ ¼öÈ£±º ¸Ê 2   (È­,  ±Ý)¸ó½ºÅÍ 2±º´ÜÀå ¹ö¸óÆ®			(509/169)
			#define LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// Á¦±¹ ¼öÈ£±º ¸Ê 2   (È­,  ±Ý)¸ó½ºÅÍ ±â»ç´ÜÀå					(514/174)
			#define LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// Á¦±¹ ¼öÈ£±º ¸Ê 2   (È­,  ±Ý)¸ó½ºÅÍ ´ë¸¶¹ý»ç					(515/176)

			#define LDK_ADD_EG_MONSTER_DEASULER					// Á¦±¹ ¼öÈ£±º 1±º´ÜÀå µ¥½½·¯									(508/168)
			#define LDK_ADD_EG_MONSTER_DRILLMASTER				// Á¦±¹ ¼öÈ£±º Á¦±¹¼öÈ£±ºÀüÅõ±³°ü
			#define LDK_ADD_EG_MONSTER_QUARTERMASTER			// Á¦±¹ ¼öÈ£±º º´ÂüÀå±³
			#define LDK_ADD_EG_MONSTER_CATO						// Á¦±¹ ¼öÈ£±º 2±º´ÜÀå Ä«Åä
			#define LDK_ADD_EG_MONSTER_ASSASSINMASTER			// Á¦±¹ ¼öÈ£±º ¾Ï»ìÀå±³
			#define LDK_ADD_EG_MONSTER_RIDERMASTER				// Á¦±¹ ¼öÈ£±º ±â¸¶´ÜÀå
			#define LDK_ADD_EG_MONSTER_GALLIA					// Á¦±¹ ¼öÈ£±º 4±º´ÜÀå °¥¸®¾Æ

			#define LDK_FIX_EG_DOOR_ROTATION_FIXED				// Á¦±¹ ¼öÈ£±º ¼º¹® ¾Ç·É¿¡ ÀÇÇÑ È¸Àü¼öÁ¤(2009.07.21)
			#define LDK_ADD_EG_DOOR_EFFECT						// Á¦±¹ ¼öÈ£±º ¸Ê ¼º¹® ÆÄ±« ÀÌÆåÆ®
			#define LDK_ADD_EG_STATUE_EFFECT					// Á¦±¹ ¼öÈ£±º ¸Ê ¼®»ó ÆÄ±« ÀÌÆåÆ®

			#define	LDK_ADD_EG_MONSTER_RAYMOND					// Á¦±¹ ¼öÈ£±º ºÎ»ç·É°ü ·¹ÀÌ¸óµå (¼ö, ÀÏ)
			#define LDK_ADD_EG_MONSTER_KNIGHTS					// Á¦±¹ ¼öÈ£±º ±â»ç´Ü	Imperial Guardian Knights	520 - 181	
			#define LDK_ADD_EG_MONSTER_GUARD					// Á¦±¹ ¼öÈ£±º È£À§º´	Imperial Guardian guard		521 - 182
			#define ASG_ADD_EG_MONSTER_GUARD_EFFECT				// Á¦±¹ ¼öÈ£±º È£À§º´ (°Ë±â ÀÌÆåÆ®)
			#define LDS_ADD_EG_MONSTER_GUARDIANDEFENDER			// Á¦±¹ ¼öÈ£±º ¸Ê 1234(¸ðµç¿äÀÏ)¸ó½ºÅÍ ¼öÈ£±º ¹æÆÐº´			(518/178)
			#define LDS_ADD_EG_MONSTER_GUARDIANPRIEST			// Á¦±¹ ¼öÈ£±º ¸Ê 1234(¸ðµç¿äÀÏ)¸ó½ºÅÍ ¼öÈ£±º Ä¡À¯º´			(519/179)

			#define LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC		// MapProcess¿¡ RenderBaseSmoke Ã³¸® ÇÔ¼ö Ãß°¡(09.07.31)
			#define	LDK_ADD_MAP_EMPIREGUARDIAN1					// Á¦±¹ ¼öÈ£±º ¸Ê 1 (¿, ¸ñ)
			#define	LDK_ADD_MAP_EMPIREGUARDIAN3					// Á¦±¹ ¼öÈ£±º ¸Ê 3 (¼ö, Åä)

			#define LDK_ADD_EMPIRE_GUARDIAN_DOOR_ATTACK			// Á¦±¹ ¼öÈ£±º ¼º¹®Àº ¸Ê¼Ó¼º¹«±âÇÏ°í °ø°Ý°¡´ÉÇÏµµ·ÏÇÔ (09.08.06)

			#define LDK_ADD_EMPIREGUARDIAN_UI					// Á¦±¹ ¼öÈ£±º ui
			#define LDK_ADD_EMPIREGUARDIAN_PROTOCOLS			// Á¦±¹ ¼öÈ£±º ÇÁ·ÎÅäÄÝ
			#define LDK_ADD_EMPIREGUARDIAN_ITEM					// Á¦±¹ ¼öÈ£±º ¾ÆÀÌÅÛ(4Á¾:¸í·É¼­, Á¾ÀÌÂÊÁö, ¼¼Å©·Î¹ÌÄÜ, Á¶°¢)

			#define LDS_ADD_RESOURCE_FLARERED					// FLARE_RED.jpg È¿°ú Ãß°¡

			#define LDS_ADD_MODEL_ATTACH_SPECIFIC_NODE_			// Æ¯Á¤ ¸ðµ¨ÀÇ Æ¯Á¤ ³ëµå¿¡ ´¸¥ Æ¯Á¤ ¸ðµ¨À» ATTACH ÇÏ´Â ÇÔ¼ö¸¦ Ãß°¡ÇÕ´Ï´. (09.06.12)
			#define LDS_ADD_INTERPOLATION_VECTOR3				// vec3_tÀÇ º¸°£ ÇÔ¼ö¸¦ zzzmathlib.h¿¡ Ãß°¡. (09.06.24)
			#define LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER		// ´Å©·ÎµåÀÇ ÆÄÀÌ¾î½ºÅ©¸² ½ºÅ³¿¡ Æ¯Á¤ ¸ó½ºÅÍ(¿¡¸£Ä­´À)ÀÇ ÆÄÀÌ¾î½ºÅ©¸²À» Ãß°¡ÇÕ´Ï´. (09.07.07)
			#define LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD		// EffectBlurSpark ÀÇ °Ë±âµéÀÇ °¹¼ö Á¦ÇÑ¿¡ ¹®Á¦·Î °Ë±â°¡ °£È¤ ±úÁö´Â Çö»óÀ¸·Î ºÎºÐ ¼öÁ¤. (09.07.28)
			#define LDS_ADD_MAP_EMPIREGUARDIAN4_MAPEFFECT		// ÁÖ¸»¸ÊÀÇ ¸Ê Effect¿¡ Ãß°¡µÇ´Â ÆÄÆ¼Å¬, EFFECTµé.. 
			#define LDS_FIX_MEMORYLEAK_DESTRUCTORFORBMD			// BMDÀÇ ¼Ò¸êÀÚ·Î ÀÎÇÑ ±âÁ¸ BMD¿¡¼­ ¹ß»ýÇÑ memoryleak FIXED.. (2009.08.13)

			#define PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT	// (·¹ÀÌ¸óµå)(Ä¡À¯º´)¸ó½ºÅÍ ÀÌÆåÆ® ÀÛ¾÷(09.07.02)

			#define KJH_ADD_EG_MONSTER_KATO_EFFECT					// 3±º´ÜÀå Ä«Åä ÀÌÆÑÆ® (09.07.20)
			#define KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT		// ¼öÈ£±º ¹æÆÐº´ ÀÌÆÑÆ® (09.07.31)
		#endif //LDS_ADD_EMPIRE_GUARDIAN

		#define ASG_ADD_NEW_QUEST_SYSTEM						// ´º Ä½ºÆ® ½Ã½ºÅÛ(2009.05.20)
		#ifdef ASG_ADD_NEW_QUEST_SYSTEM
			#define ASG_ADD_UI_QUEST_PROGRESS					// Ä½ºÆ® ÁøÇà Ã¢(NPC¿ë)(2009.05.27)
			#define ASG_ADD_UI_QUEST_PROGRESS_ETC				// Ä½ºÆ® ÁøÇà Ã¢(±âÅ¸¿ë)(2009.06.24)
			#define ASG_MOD_UI_QUEST_INFO						// Ä½ºÆ® Á¤º¸ Ã¢(2009.07.06)
			#define ASG_MOD_3D_CHAR_EXCLUSION_UI				// 3D Ä³¸¯ÅÍ Ç¥ÇöÀÌ ¾ø´Â UI·Î ¹²Þ.(2009.08.03)
		#endif	// ASG_ADD_NEW_QUEST_SYSTEM

		#define LJH_MOD_POSITION_OF_REGISTERED_LUCKY_COIN		// Çà¿îÀÇ µ¿Àü µî·Ï Á¦ÇÑ ¼ö°¡ 2^31ÀÌ µÊ¿¡ µû¶ó µ¿ÀüÀÌ¹ÌÁö¿Í °ãÄ¡´Â Çö»ó ¸·±âÀ§ÇØ ¼öÁ¤(2010.02.16)							
		#define YDG_MOD_SEPARATE_EFFECT_SKILLS					// ½ºÅ³ °Ë»çÇÏ´Â ÀÌÆåÆ® ºÐ¸® (ºí·¯µå ¾îÅÃ µî) (2009.08.10)
		#define ASG_ADD_NEW_DIVIDE_STRING						// »õ·Î¿î ¹®Àå ³ª´©±â ±â´É.(2009.06.08)
		#define PBG_ADD_DISABLERENDER_BUFF						// ·£´õ¾ÈÇÒ ¹öÇÁ ÀÌ¹ÌÁö Ã³¸®(09.08.10)

	#endif	// SEASON5


	#define ADD_TOTAL_CHARGE_7TH
	#ifdef ADD_TOTAL_CHARGE_7TH
		// - ¾ç´ë±
		#define YDG_ADD_CS7_CRITICAL_MAGIC_RING		// Ä¡¸í¸¶¹ý¹ÝÁö
		#define YDG_ADD_CS7_MAX_AG_AURA				// AGÁõ°¡ ¿À¶ó
		#define YDG_ADD_CS7_MAX_SD_AURA				// SDÁõ°¡ ¿À¶ó
		#define YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM	// ÆÄÆ¼ °æÇèÄ¡ Áõ°¡ ¾ÆÀÌÅÛ
		#define YDG_ADD_CS7_ELITE_SD_POTION			// ¿¤¸®Æ® SDÈ¸º¹ ¹°¾à
		#define YDG_MOD_AURA_ITEM_EXCLUSIVELY_BUY	// ¿À¶ó Áßº¹ ±¸ÀÔ ¸·±â (09.11.02)

		// - ÀÌµ¿±
		#define LDK_MOD_PETPROCESS_SYSTEM			// ÆêÇÁ·Î¼¼½º ¾÷µ¥ÀÌÆ® ¼ø¼­ º¯°æ(¾Ö´Ï¸ÞÀÌ¼Ç ¼Óµµ ÀÓÀÇ º¯°æ °¡´É)(2009.09.11)
		#define LDK_ADD_CS7_UNICORN_PET				// À¯´ÏÄÜ Æê 
		#define LDK_FIX_CS7_UNICORN_PET_INFO		// À¯´ÏÄÜ Æê Ä³¸¯ÅÍ Á¤º¸Ã¢ ¹æ¾î·Â Ãß°¡

		// - ¹Úº¸±
		#define PBG_ADD_AURA_EFFECT					// AG SD Áõ°¡ ¿À¶ó ÀÌÆåÆ®
	#endif //ADD_TOTAL_CHARGE_7TH

	#define KJH_FIX_GET_ZEN_SYSTEM_TEXT						// °×ºí»óÁ¡¿¡¼­ ¾ÆÀÌÅÛ±¸ÀÔ½Ã Á¨Áõ°¡ ¸Þ¼¼Áö°¡ ³ª¿À´Â ¹®Á¦. (09.03.18) [09.03.19 Å×¼·]
	#define KJH_FIX_WOPS_K33695_EQUIPABLE_DARKLOAD_PET_ITEM	// ´Å©·Îµå°¡ ÀÎº¥¿¡ ÆêÀ» ¼ÒÀ¯ÇÏ°í ÀÖÀ»¶§, ÅøÆÁ¿¡ Âø¿ëºÒ°¡·Î ³ª¿À´Â ¹ö±×¼öÁ¤ (09.04.27)
	#define KJH_FIX_WOPS_K33479_SELECT_CHARACTER_TO_HEAL	// Ä³¸¯ÅÍ¿¡°Ô µÑ·¯½×¿´À»¶§, ¿äÁ¤ÀÇ Ä¡·á½ºÅ³°­È­ÀÌÈÄ Ä³¸¯ÅÍ ¼±ÅÃÀÌ Àß ¾ÈµÇ¾îÁö´Â ¹ö±×¼öÁ¤ (09.04.27)
	#define KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET			// ÆêÀ»Å¸°í ´Å©·Îµå Ä«¿ÀÆ½µð¼¼ÀÌ¾î ½ºÅ³½Ã ¾Ö´Ï¸ÞÀÌ¼Ç¼öÁ¤ (08.12.18)

	#define CSK_FIX_UI_FUNCTIONNAME							// UI°ü·Ã ÇÔ¼ö¸í º¯°æ(2009.01.22) [2009.03.26 Å×¼·]
	#define CSK_FIX_ANIMATION_BLENDING						// ¾Ö´Ï¸ÞÀÌ¼Ç ºí·»µÂÊ ºí·»µå ¾ÈµÇ°Ô ¿É¼Ç ÁÖ°Ô ¼öÁ¤(2009.01.29) [2009.03.26 Å×¼·]
	#define CSK_FIX_ADD_EXEPTION							// dmpÆÄÀÏ ºÐ¼®ÇØº¸´Ï getTargetCharacterKey ÇÔ¼ö¿¡¼­ Æ¨±â´Â°É·Î ¿¹ÃøµÇ¼­ ¿¹¿ÜÃ³¸® Ãß°¡(2009.01.29) [2009.03.26 Å×¼·]

	#define PBG_FIX_PKFIELD_ATTACK							// PKFIELD¿¡¼­ ±æµå¿ø³¢¸® °ø°ÝµÇ´Â Çö»ó(09.04.02) [2009.04.06 Å×¼·]
	#define PBG_FIX_PKFIELD_CAOTIC							// Ä«¿ÀÆ½ µð¼¼ÀÌ¾î Å» °Í¿¡¼­ ¹ö±× ¼öÁ¤(09.04.07) [2009.04.13 Å×¼·]
	#define PBG_FIX_GUILDWAR_PK								// PKFIELD¿¡¼­ ±æµå¿ö Áß¿¡ ÀÏ¹ÝÀÎ °ø°Ý¾È¹Þ´Â ¹ö±× ¼öÁ¤(09.04.07) [2009.04.13 Å×¼·]
	#define PBG_FIX_CHAOTIC_ANIMATION						// Ä«¿ÀÆ½ µð¼¼ÀÌ¾î ¾Ö´Ï¸ÞÀÌ¼Ç ¼öÁ¤(09.06.11)
	#define PBG_FIX_DARK_FIRESCREAM_HACKCHECK				// ´Å© ÆÄÀÌ¾î½ºÅ©¸² (¼­¹ö)ÇÃ¼Å©°ü·Ã ¹ö±× ¼öÁ¤(09.06.22)
	#define PBG_MOD_INVENTORY_REPAIR_COST					// ÀÚµ¿¼ö¸®±Ý¾× ÇÏÇâÁ¶Á¤(09.06.08)
	#define PBG_MOD_LUCKYCOINEVENT							// Çà¿îÀÇµ¿Àü 255°³ ÀÌ»ó µî·ÏµÇÁö ¾Êµµ·Ï ÀÌº¥Æ® ¼öÁ¤(09.07.15)(´¸¥UI¿ÍÀÇ¹ö±×,¿­°íÀÌµ¿¹ö±×)

	#define LDK_FIX_INVENTORY_SPEAR_SCALE					// ÀÎº¥Åä¸® ¹«±â-spear Å©±â Ã³¸® if¹® ¼öÁ¤ [09.03.19 Å×¼·]

	#define YDG_FIX_INVALID_TERRAIN_LIGHT					// ¸Þ¸ð¸® Ä§¹ü ¹ö±× ¼öÁ¤(¸ÊÅø °ü·Ã) (2009.03.30) [2009.04.13 Å×¼·]
	#define YDG_FIX_MEMORY_LEAK_0905						// ¸Þ¸ð¸® ´©¼ö Á¦°Å (2009.05.11)
	#define YDG_FIX_INVALID_SET_DEFENCE_RATE_BONUS			// ¹æ¾î±¸¼¼Æ® Ãß°¡ ¹æ¾î·ÂÇ¥½Ã ¹æ¾î·ÂÀÇ 10%°¡ ¾Æ´Ï¶ó ¹æ¾îÀ²ÀÇ 10%ÀÓ (2009.04.15) [ÆÐÄ¡¾ÈµÊ] wops_32937
	#define YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE			// Ä«¿À½ºÄ³½½¿¡¼­ ¸¶°Ë»ç ÁöÆÎÀÌ Âø¿ë½Ã ÇÃ·¹ÀÓ½ºÆ®¶óÀÌÅ©¿Í ºí·¯µå¾îÅÃ ¾ÆÀÌÄÜ»ö ¹®Á¦ (2009.04.15) [ÆÐÄ¡¾ÈµÊ] wops_34747
	#define YDG_FIX_LEFTHAND_MAGICDAMAGE					// ¸¶°Ë»ç°¡ ÁöÆÎÀÌ³ª ·é¹½ºÅ¸µå¸¦ ¿Þ¼Õ¿¡ Ã¡À»¶§ ¸¶·ÂÀÌ ¿Ã¶ó°¡´Â ¹®Á¦ (2009.04.15) [ÆÐÄ¡¾ÈµÊ] wops_32641
	#define YDG_FIX_BLOCK_STAFF_WHEEL						// ¸¶°Ë»ç ÁöÆÎÀÌ, ¸Ç¼Õ È¸¿À¸®º£±â ¸·±â (2009.05.18)
	#define YDG_FIX_MEMORY_LEAK_0905_2ND					// ¸Þ¸ð¸® ´©¼ö Á¦°Å 2Â÷ (2009.05.19)
	#define YDG_FIX_CLIENT_SKILL_EFFECT_SIZE				// ÀÌÆåÆ® ¸¹À»¶§ ºí·¯µå¾îÅÃµî Å¬¶ó½ºÅ³ ¾È¸Ô´Â ¹ö±× (2009.07.13)

	#define ASG_FIX_PICK_ITEM_FROM_INVEN					// ÀÎº¥Ã¢ Æ¯Á¤ Ä­¿¡¼­ ¾ÆÀÌÅÛÀ» Áý¾îµé¾úÀ» ¶§ Ä³¸¯Á¤º¸Ã¢¿¡¼­ ´É·ÂÄ¡°¡ º¯µ¿µÇ´Â ¹ö±×(2009.04.15) wops_33567
	#define ASG_FIX_MOVE_WIN_MURDERER1_BUG					// ¹«¹ýÀÚ(Ä«¿À) ·¹º§1ÀÏ ¶§ ¸Ê ÀÌµ¿Ã¢ Ç¥½Ã ¹ö±×.(2009.04.20) wops_34498

	#define PJH_FIX_4_BUGFIX_33								// wops_34066

	#define LDS_FIX_MASTERSKILLKEY_DISABLE_OVER_LIMIT_MAGICIAN		// ¸¶½ºÅÍ ½ºÅ³ °­È­ µÈ ½ºÅ³µé¿¡ ´ëÇØ ¿ä±¸Ä¡ ¹Ì´Þ½Ã¿¡ ÀÌ¿ë ºÒ°¡. (09.04.22) no_wops
	#define LDS_FIX_SETITEM_WRONG_CALCULATION_IMPROVEATTACKVALUE	// ¼¼Æ®¾ÆÀÌÅÛÂø¿ë ÀÌÈÄ »ç¶ûÀÇ ¹¦¾àÀ» ¸ÔÀ¸¸é, ÄÉ¸¯ÅÍ Á¤º¸Ã¢¿¡ ÈûÁõ°¡ ½ºÅÈ¼öÄ¡°¡ ´¸¥Çö»ó. (09.04.23) wops_35131
	#define LDS_FIX_MEMORYLEAK_PHYSICSMANAGER_RELEASE				// ¸Þ¸ð¸® ´©¼ö Á¦°Å, Physicsmanager Èü¿µ¿ª ¸Þ¸ð¸® ¹ÝÈ¯ Ã³¸®. (´Å©·Îµå ÄÉ¸¯ ·ÎµÀÌÈÄ Á¾·á ½Ã ¸¯ ¹ß»ý)(09.05.25)
#endif // UPDATE_100527

#define KJH_ADD_SERVER_LIST_SYSTEM						// ¼­¹ö¸®½ºÆ® ½Ã½ºÅÛ º¯°æ (09.10.19) - ±¹³»ÀÇ ³»¿ëÀ» ¹Ì¸® ´ç°Ü »ç¿ë.
#define ASG_ADD_SERVER_LIST_SCRIPTS						// ServerList.txt ½ºÅ©¸³Æ® Ãß°¡.(2009.10.29) - ±¹³»ÀÇ ³»¿ëÀ» ¹Ì¸® ´ç°Ü »ç¿ë.
#ifdef ASG_ADD_SERVER_LIST_SCRIPTS
	#define ASG_ADD_SERVER_LIST_SCRIPTS_GLOBAL_TEMP		// ¹Ì¸® ASG_ADD_SERVER_LIST_SCRIPTS Ãß°¡·Î ÀÎÇÑ ¹ö±× ¼öÁ¤.(10.03.02) 
															//Season5 ÀÌÈÄ »èÁ¦ÇØµµ µÊ.(BuxConvert() ÇÔ¼ö À§Ä¡ ¹®Á¦) // Season5 ¾÷µ¥ÀÌÆ®½Ã¿¡ »èÁ¦ÇØµµ µÇ´Â°ÍÀÎÁö È®ÀÎ ´½Ã ÇÒ°Í.
	#define ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL		// ¼­¹ö¸®½ºÆ®¿¡ À¯·á Ã¤³Î Ãß°¡.(10.02.26) - ÇöÀç ±Û·Î¹ú¸¸ Àû¿ë.
#endif	//ASG_ADD_SERVER_LIST_SCRIPTS


//-----------------------------------------------------------------------------
// [ Season 4.5 ] 
#define SEASON4_5										
#ifdef SEASON4_5

	#define KJH_ADD_09SUMMER_EVENT					// 09³â ¿©¸§ÀÌº¥Æ®		(09.06.23)

	#define YDG_ADD_NEW_DUEL_SYSTEM					// »õ·Î¿î °áÅõ ½Ã½ºÅÛ (2009.01.05) [09.03.19 Å×¼·]
	#ifdef YDG_ADD_NEW_DUEL_SYSTEM
		#define YDG_ADD_NEW_DUEL_UI					// »õ·Î¿î °áÅõ °ü·Ã UI Ãß°¡ (2009.01.05) [09.03.19 Å×¼·]
		#define YDG_ADD_NEW_DUEL_PROTOCOL			// »õ·Î¿î °áÅõ °ü·Ã ÇÁ·ÎÅäÄÝ Ãß°¡ (2009.01.08) [09.03.19 Å×¼·]
		#define YDG_ADD_NEW_DUEL_NPC				// »õ·Î¿î °áÅõ °ü·Ã NPC Å¸ÀÌÅõ½º Ãß°¡ (2009.01.19) [09.03.19 Å×¼·]
		#define YDG_ADD_NEW_DUEL_WATCH_BUFF			// »õ·Î¿î °áÅõ °üÀü ¹öÇÁ Ãß°¡ (2009.01.20) [09.03.19 Å×¼·]
		#define YDG_ADD_MAP_DUEL_ARENA				// »õ·Î¿î °áÅõÀå ¸Ê Ãß°¡ (2009.02.04) [09.03.19 Å×¼·]
	#endif	// YDG_ADD_PVP_SYSTEM

	#define PBG_ADD_PKFIELD							// PKÇÊµå Ãß°¡(2008.12.29) [09.03.19 Å×¼·]
	#define PBG_ADD_HONOROFGLADIATORBUFFTIME		// °ËÅõ»çÀÇ ¸í¿¹ ¹öÇÁ ½Ã°£ ÅøÆÁ Ãß°¡(2009.03.19) [09.03.19 Å×¼·]

	#define LDK_ADD_GAMBLE_SYSTEM					// Á¨¼Ò¸ð ¹«±â »Ì±â »óÁ¡ (09.01.05) [09.03.19 Å×¼·]
	#ifdef LDK_ADD_GAMBLE_SYSTEM
		#define LDK_ADD_GAMBLE_NPC_MOSS				// °×ºí »óÀÎ ¸ð½º [09.03.19 Å×¼·]
		#define LDK_ADD_GAMBLE_RANDOM_ICON			// °×ºí¿ë ·£´ý¹«±â ¾ÆÀÌÄÜ [09.03.19 Å×¼·]
		#define LDK_ADD_GAMBLERS_WEAPONS			// °×ºíÀü¿ë ·¹¾î ¹«±âµé [09.03.19 Å×¼·]
	#endif //LDK_ADD_GAMBLE_SYSTEM

	#define PJH_ADD_PANDA_PET
	#define PJH_ADD_PANDA_CHANGERING

#endif // SEASON4_5


#define ASG_FIX_LENA_REGISTRATION

#define ASG_ADD_CHARGED_CHANNEL_TICKET					// À¯·á Ã¤³Î ÀÔÀå±Ç.(2010.02.05) - ±Û·Î¹úÀü¿ë

#define ADD_TOTAL_CHARGE_6TH
#ifdef ADD_TOTAL_CHARGE_6TH

	#define ADD_CASH_SHOP_6TH
		#ifdef ADD_CASH_SHOP_6TH
		#define ASG_ADD_CS6_GUARD_CHARM				// ¼öÈ£ÀÇºÎÀû(2009.02.09)
		#define ASG_ADD_CS6_ITEM_GUARD_CHARM		// ¾ÆÀÌÅÛº¸È£ºÎÀû(2009.02.09)
		#define ASG_ADD_CS6_ASCENSION_SEAL_MASTER 	// »ó½ÂÀÇÀÎÀå¸¶½ºÅÍ(2009.02.24)
		#define ASG_ADD_CS6_WEALTH_SEAL_MASTER 		// Ç³¿äÀÇÀÎÀå¸¶½ºÅÍ(2009.02.24)
		#define LDS_ADD_CS6_CHARM_MIX_ITEM_WING
	#endif	// ADD_CASH_SHOP_6TH

	#define CSK_ADD_GOLDCORPS_EVENT					// È²±Ý±º´ÜÀÌº¥Æ®(2009.03.13)
	#define LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON // È²±Ý ±×·¹ÀÌÆ® µå·¹°ï ºñÁÖ¾ó ÀçÀÛ¾÷. ºñÁÖ¾ó ÀÌÆåÆ® Ãß°¡. ±âÁ¸ µ¿ÀÏÇÑ ¸÷ Á¸Àç. (2009.04.06) (BITMAP_FIRE_HIK3_MONO È¿°ú ÂüÁ¶.)
#endif // ADD_TOTAL_CHARGE_6TH


#define PBG_MOD_NONPVPSERVER
#define LDS_FIX_PETDESTRUCTOR_TERMINATE_EFFECTOWNER_ADD_TYPE
#define LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER

// (2009.04.23)
#define PJH_CHARACTER_RENAME
#define LDK_MOD_GLOBAL_PORTAL_LOGO

//#define LDK_MOD_GLOBAL_PORTAL_CASHSHOP_BUTTON_DENY	// (09.04.07)
#define LDK_FIX_MOVEWINDOW_SCROLL_BUG				// (2009.04.22)
#define LDK_MOD_PASSWORD_LENGTH_20					// (2009.04.24)
#define LDK_MOD_GLOBAL_STORAGELOCK_CHANGE			// (2009.04.27)

#define ADD_CASH_SHOP_5TH

#ifdef ADD_CASH_SHOP_5TH
	#define LDK_FIX_CS5_HIDE_SEEDLEVEL
	#define YDG_ADD_CS5_REVIVAL_CHARM
	#define YDG_ADD_CS5_PORTAL_CHARM
#endif	// YDG_ADD_CASH_SHOP_5TH

#define PBG_WOPS_CURSEDTEMPLEBASKET_MOVING			//(2008.12.18)

#define PSW_PARTCHARGE_ITEM4

#ifdef PSW_PARTCHARGE_ITEM4
	#define PSW_ADD_PC4_SEALITEM
	#define PSW_ADD_PC4_SCROLLITEM
	#define PSW_ADD_PC4_CHAOSCHARMITEM
	#define LDK_ADD_PC4_GUARDIAN
	#define	LDK_FIX_GUARDIAN_CHANGE_LIFEWORD			//(2009.01.14)
	#define LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE
#endif //PSW_PARTCHARGE_ITEM4


#define PSW_ADD_FOREIGN_HELPWINDOW

#define PSW_PARTCHARGE_ITEM3

#ifdef PSW_PARTCHARGE_ITEM3
	#define PSW_NEW_CHAOS_CARD
	#define PSW_NEW_ELITE_ITEM
#endif //PSW_PARTCHARGE_ITEM3


#define PSW_PARTCHARGE_ITEM2

#ifdef PSW_PARTCHARGE_ITEM2
	#define PSW_ELITE_ITEM
	#define PSW_SCROLL_ITEM
	#define PSW_SEAL_ITEM
	#define PSW_FRUIT_ITEM
	#define PSW_SECRET_ITEM
	#define PSW_INDULGENCE_ITEM
	#define PSW_RARE_ITEM
	#define PSW_CURSEDTEMPLE_FREE_TICKET
#endif //PSW_PARTCHARGE_ITEM2


#define PSW_PARTCHARGE_ITEM1
				
#ifdef PSW_PARTCHARGE_ITEM1
	#define CSK_FREE_TICKET						// (2007.02.06)
	#define CSK_CHAOS_CARD						// (2007.02.07)
	#define CSK_RARE_ITEM						// (2006.02.15)
	#define CSK_LUCKY_CHARM						// ( 2007.02.22 )
	#define CSK_LUCKY_SEAL						// ( 2007.02.22 )	
#endif //PSW_PARTCHARGE_ITEM1

#define LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX


