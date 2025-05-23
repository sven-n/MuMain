#ifndef __WZAUDIO_H__
#define __WZAUDIO_H__
// wzAudio.h

/***********************************************************************
*
* (c) Copyright Bugs 2000 - 2004  All Rights Reserved.
*
* Filename:	wzAudio.h
* Synopsis:	wzAudio API header
* Author:	  Lee Seungjoo <seungjoo@webzen.co.kr>
* Created:	 2004.04.
*
***********************************************************************/

# ifdef __cplusplus
extern "C" {
# endif


//////////////////////////////////////////////////////////////////////////
// constants
#define MAX_EQ_BANKSLOTS	8

// wzAudio option flags
#define WZAOPT_STOPBEFOREPLAY	0	// stop player and flush directsound buffer before starting new media
#define WZAOPT_REPEATTRACK	   2	// wheather or not repeat current playback media ( do not use current version)
#define WZAOPT_FADEINOUT		4	// fade in, fade out while switching a media (not working in this version)
#define WZAOP_EQUALIZER			8	// equalizer on/off option

// Mixer mode
enum enMixerMode
{
	_mmMasterVolume, // system default master volume control
	_mmWaveVolume,	// system wave mixer
	_mmInternalVolume // wzAudio internal volume control
}; // m_enMixerMode

// Equalizer Bank slots
enum enEq
{
    Eq1 = 0,
    Eq2,
    Eq3,
    Eq4,
    Eq5,
    Eq6,
    Eq7,
    Eq8
};


//////////////////////////////////////////////////////////////////////////
// API

// Function name	: wzAudioCreate
// Description	    : create wzAudio library instance
// Return type		: int 
//				-1	error. No audio devices in this system
//				0	ok
int wzAudioCreate(HWND hParentWnd);


// Function name	: wzAudioDestroy
// Description	    :  destroy wzAudio library instance
// Return type		: void 
void wzAudioDestroy();


// Function name	: wzAudioPlay
// Description	    :  play media for numRepeat times
//				- supported format: mp3, ogg, wav
//                              - ***do not support repeated playback option in this version      				
// Return type		: void 
// Argument         : char* szFilename
// Argument         : int numRepeat = 1 (default : play just 1 time)   
//					 numRepeat = n ( play n times) 
//					 numRepeat = -1 ( play unlimitedly)
//
void wzAudioPlay(char* szFilename, int numRepeat);


// Function name	: wzAudioPause
// Description	    : pause playback
// Return type		: void 
void wzAudioPause();



// Function name	: wzAudioStop
// Description	    : stop playback
// Return type		: void 
void wzAudioStop();



// Function name	: wzAudioSetVolume
// Description	    : reset sound volume level
// Return type		: void 
// Argument         : int numVolume
void wzAudioSetVolume(int numVolume);


// Function name	: wzAudioGetVolume
// Description	    : return current sound volumn level  by range of 0 ~ +100
// Return type		: int - volume level ( 0 ~ 100)
int wzAudioGetVolume();


// Function name	: wzAudioIncreaseVolume
// Description	    :	increase sound volume +5
// Return type		: void 
void wzAudioVolumeUp();


// Function name	: wzAudioDecreaseVolume
// Description	    :  decrease sound volume -5
// Return type		: void 
void wzAudioVolumeDown();


// Function name	: wzAudioOpenFile
// Description	    :  display FileOpen dialog box to select a file to operate
// Return type		: int 
//				1	file selected
//				0	no file selected
// Argument         : char* szFilenameBuf (media file fullpath name)
int wzAudioOpenFile(char* szFilenameBuf);


// Function name	: wzAudioSeek
// Description	    :  offset the playback position by nPosition
// Return type		: void 
// Argument         : int nPosition ( byte offset as divided by 100)
void wzAudioSeek(int nPosition);


// Function name	: wzAudioGetStreamOffsetRange
// Description	    :  query current playback position information by byte offset ( % percent)
// Return type		: int -  stream offset range value ( 0 ~ 100)
int wzAudioGetStreamOffsetRange();


// Function name	: wzAudioGetStreamOffsetSec
// Description	    :  query current playback position information by time seconds offset
// Return type		: int - stream offset range seconds
int wzAudioGetStreamOffsetSec();


// Function name	: wzAudioSetMixerMode
// Description	    :  select a mixer to control volumes
// Return type		: void 
// Argument         : int nMixerMode ( _mmMasterVolume/ _mmWaveVolume / _mmInternalVolume) 
void wzAudioSetMixerMode(int nMixerMode);


// Function name	: wzAudioGetStreamInfo
// Description	    :  query bitrate and frequency informations for the current media
// Return type		: void 
// Argument         : char* lpszBitrate ( bitrate)
// Argument         : char* lpszFrequency ( frequency)
void wzAudioGetStreamInfo(char* lpszBitrate, char* lpszFrequency);


// Function name	: wzAudioOption
// Description	    :  set wzAudio options
// Return type		: void 
// Argument         : int nOption (option flag)
// Argument         : int nVal (option value)
void wzAudioOption(int nOption, int nVal);


// Function name	: wzAudioSetEqualizer
// Description	    :	tune wzAudio equalizer control values
// Return type		: void 
// Argument         : const int Slider[8]   ( Equalizer setting value of each bank slots by range of  -128 ~ + 127 )
void wzAudioSetEqualizer(const int Slider[MAX_EQ_BANKSLOTS]);


//int wzAudioConvertToWave(char* lpszSrcpath, char* lpszDestpath);

# ifdef __cplusplus
}
# endif

#endif //__WZAUDIO_H__