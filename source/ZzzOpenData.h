void OpenTextData();

void DeleteNpcs();
void OpenNpc(int Type);
void DeleteMonsters();
void OpenMonsterModel(int Type);

void OpenTexture(int Model,char *SubFolder,int Wrap=GL_REPEAT, int Type=GL_NEAREST,bool Check=true);
void OpenModel(int Type,char *Dir,char *ModelFileName,...);
void AccessModel(int Type,char *Dir,char *FileName,int i=-1);
void OpenInterfaceData();
void OpenBasicData(HDC hDC);
void OpenMainData();
void ReleaseMainData();
void OpenWorld(int World);
void SaveWorld(int Wordl);
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

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
extern string g_strSelectedML;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE