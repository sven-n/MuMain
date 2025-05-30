#pragma once

#include "zzzinfomation.h"
#include "SpinLock.h"

extern EGameScene SceneFlag;

extern int  ErrorMessage;
extern bool InitServerList;
extern bool InitLogIn;
extern bool InitLoading;
extern bool InitCharacterScene;
extern bool InitMainScene;
extern bool EnableMainRender;
extern wchar_t* szServerIpAddress;
extern unsigned short g_ServerPort;
extern int g_iLengthAuthorityCode;

inline SpinLock* g_render_lock = new SpinLock();

extern bool CheckRenderNextFrame();
extern void WaitForNextActivity(bool usePreciseSleep);
extern void UpdateSceneState();
extern void LoadingScene(HDC hDC);
extern void RenderScene(HDC Hdc);
extern bool CheckName();
void    StartGame();
void SetTargetFps(double targetFps);

BOOL	ShowCheckBox(int num, int index, int message = MESSAGE_TRADE_CHECK);

int	SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize);

bool	GetTimeCheck(int DelayTime);
void	SetEffectVolumeLevel(int level);
void    SetViewPortLevel(int level);

bool IsEnterPressed();
void SetEnterPressed(bool enterpressed);
