void OpenTextData();

void DeleteNpcs();
void OpenNpc(int Type);
void DeleteMonsters();
void OpenMonsterModel(int Type);

void OpenModel(int Type,char *Dir,char *ModelFileName,...);
void OpenInterfaceData();
void OpenBasicData(HDC hDC);
void OpenMainData();
void ReleaseMainData();
//void OpenWorld(int World);
//void SaveWorld(int Wordl);
void OpenFont();
void OpenLogoSceneData();
void ReleaseLogoSceneData();
void OpenCharacterSceneData();
void ReleaseCharacterSceneData();

void OpenPlayerTextures();
void OpenItemTextures();

void OpenSkillHotKey(char *File);
void SaveOptions();
void SaveMacro(char *FileName);
void OpenMacro(char *FileName);

extern char AbuseFilter[][20];
extern char AbuseNameFilter[][20];
extern int  AbuseFilterNumber;
extern int  AbuseNameFilterNumber;
extern std::string g_strSelectedML;