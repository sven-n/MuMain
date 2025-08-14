///////////////////////////////////////////////////////////////////////////////
// DirectX Sound
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// File: PlaySound.cpp
//
// Desc: DirectSound support for how to load a wave file and play it using a
//       static DirectSound buffer.
//
// Copyright (c) 1999 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <objbase.h>
#include <dsound.h>
#include "DSwaveio.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"
#include "DSPlaySound.h"



bool                    g_EnableSound = false;
bool                    g_Enable3DSound = false;
int                  	SoundLoadCount = 0;

LPDIRECTSOUND           g_lpDS = NULL;
LPDIRECTSOUNDNOTIFY     g_lpDSNotify = NULL;
LPDIRECTSOUND3DLISTENER g_lpDS3DListener;

LPDIRECTSOUNDBUFFER     g_lpDSBuffer[MAX_BUFFER][MAX_CHANNEL];
LPDIRECTSOUND3DBUFFER   g_lpDS3DBuffer[MAX_BUFFER][MAX_CHANNEL];

DWORD                   g_dwBufferBytes;

OBJECT* Object3DSound[MAX_BUFFER][MAX_CHANNEL];
wchar_t                 BufferName[MAX_BUFFER][64];
int                     BufferChannel[MAX_BUFFER];
int                     MaxBufferChannel[MAX_BUFFER];
bool                    Enable3DSound[MAX_BUFFER];

//LPEAXMANAGER			lpEAXManager = NULL;
LPKSPROPERTYSET		    g_lpPropertySet[MAX_BUFFER];			// Property Set Interfaces

waveIO* wavefile = NULL;					// WAVE IO class instances

//. master volume
long g_MasterVolume = 0L;

//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT InitDirectSound(HWND hDlg)
{
    HRESULT             hr;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER lpDSBPrimary = NULL;
    WAVEFORMATEX        wfx;

    if (g_Enable3DSound)
    {
    }
    else
    {
        // Initialize COM
        CoInitialize(NULL);

        // Create IDirectSound using the primary sound device
        if (FAILED(hr = DirectSoundCreate(NULL, &g_lpDS, NULL)))
        {
            g_ErrorReport.Write(L"Init - DirectSoundCreate Error\r\n");
            return hr;
        }

        // Set coop level to DSSCL_PRIORITY
        if (FAILED(hr = g_lpDS->SetCooperativeLevel(hDlg, DSSCL_PRIORITY)))
        {
            g_ErrorReport.Write(L"Init - DS SetCooperativeLevel Error\r\n");
            return hr;
        }
        //if( FAILED( hr = g_lpDS->SetCooperativeLevel( hDlg, DSSCL_NORMAL ) ) )
        //    return hr;

        // Obtain primary buffer, asking it for 3D control
        ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
        dsbdesc.dwSize = sizeof(DSBUFFERDESC);
        dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
        if (FAILED(hr = g_lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBPrimary, NULL)))
        {
            g_ErrorReport.Write(L"Init - DS CreateSoundBuffer Error\r\n");
            return hr;
        }

        if (FAILED(hr = lpDSBPrimary->QueryInterface(IID_IDirectSound3DListener,
            (VOID**)&g_lpDS3DListener)))
        {
            g_ErrorReport.Write(L"Init DS QueryInterface Error\r\n");
            return hr;
        }

        // Get listener parameters
        //g_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
        //m_pDirectSound3DListener->GetAllParameters( &g_dsListenerParams );

        // Set primary buffer format to 22kHz and 16-bit output.
        ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 22050;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        if (FAILED(hr = lpDSBPrimary->SetFormat(&wfx)))
        {
            g_ErrorReport.Write(L"Init DS SetFormat Error\r\n");
            return hr;
        }

        // Release the primary buffer, since it is not need anymore

        //  LPDIRECTSOUNDBUFFER √ ±‚»≠.
        for (int i = 0; i < MAX_BUFFER; ++i)
        {
            g_lpDSBuffer[i][0] = NULL;
            g_lpDSBuffer[i][1] = NULL;
            g_lpDSBuffer[i][2] = NULL;
            g_lpDSBuffer[i][3] = NULL;
        }
    }
    SAFE_RELEASE(lpDSBPrimary);
    g_EnableSound = true;
    return S_OK;
}

void    SetEnableSound(bool b)
{
    g_EnableSound = b;
}

//-----------------------------------------------------------------------------
// Name: CreateStaticBuffer()
// Desc: Creates a wave file, sound buffer and notification events
//-----------------------------------------------------------------------------
HRESULT CreateStaticBuffer(int Buffer, TCHAR* strFileName, int MaxChannel, bool Enable)
{
    HRESULT             hr;
    WAVEFORMATEX		wfx;						// WAVE structure
    DSBUFFERDESC		dsbdSecondary;				// Secondary buffer description
    DWORD	         	datasize;

    //Create a new instance of our WaveIO Object to load in a wavefile later
    wavefile = new waveIO(INPUT);

    // Load in our test wave file
    if (wavefile->LoadWaveHeader(strFileName) == false) {
        delete wavefile;
        return 0;
    }

    wfx = wavefile->GetWaveFormatEx();
    datasize = wavefile->GetDataSize();
    g_dwBufferBytes = datasize;

    memset(&dsbdSecondary, 0, sizeof(DSBUFFERDESC));

    // Fill in our DS Secondary buffer description
    dsbdSecondary.dwSize = sizeof(DSBUFFERDESC);
    dsbdSecondary.dwBufferBytes = datasize;

    // Don't set DSBCAPS_CTRLPOSITIONNOTIFY or DSBCAPS_CTRLALL - you won't get a hardware buffer
    if (Enable)
        dsbdSecondary.dwFlags = DSBCAPS_LOCHARDWARE | DSBCAPS_CTRL3D | DSBCAPS_STATIC;
    //dsbdSecondary.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRL3D | DSBCAPS_STATIC;
    else
#if DIRECTSOUND_VERSION < 0x0800
        dsbdSecondary.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC;
#else
        dsbdSecondary.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
#endif
    dsbdSecondary.lpwfxFormat = &wfx;

    if (FAILED(hr = g_lpDS->CreateSoundBuffer(&dsbdSecondary, &g_lpDSBuffer[Buffer][0], NULL)))
    {
        g_ErrorReport.Write(L"Failed to create Direct Sound buffer [%s]\r\n", strFileName);
        delete  wavefile;
        return 0;
    }

    if (Enable)
    {
        // Query for a 3D Interface for the buffer
        if (FAILED(hr = g_lpDSBuffer[Buffer][0]->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&g_lpDS3DBuffer[Buffer][0])))
        {
            g_ErrorReport.Write(L"Failed to query for 3D Interface on Direct Sound buffer [%s]\r\n", strFileName);
            MessageBox(g_hWnd, L"Failed to query for 3D Interface on Direct Sound buffer", NULL, MB_OK);
            g_ErrorReport.Write(L"Failed to query for 3D Interface on Direct Sound buffer");
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
            delete  wavefile;
            return 0;
        }

        // Set position to be to the front left of listener
        if (FAILED(hr = g_lpDS3DBuffer[Buffer][0]->SetPosition(-2, 0, 2, DS3D_IMMEDIATE)))
        {
            g_ErrorReport.Write(L"Failed to set position of 3D buffer [%s]\r\n", strFileName);
            MessageBox(g_hWnd, L"Failed to set position of 3D buffer", NULL, MB_OK);
            g_ErrorReport.Write(L"Failed to set position of 3D buffer");
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
            delete  wavefile;
            return 0;
        }

        // Query for a Property Set Interface for the buffer
        if (FAILED(hr = g_lpDS3DBuffer[Buffer][0]->QueryInterface(IID_IKsPropertySet, (void**)&g_lpPropertySet[Buffer])))
        {
            g_ErrorReport.Write(L"Failed to get Property Set Interface [%s]\r\n", strFileName);
            MessageBox(g_hWnd, L"Failed to get Property Set Interface", NULL, MB_OK);
            g_ErrorReport.Write(L"Failed to get Property Set Interface");
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
            delete  wavefile;
            return 0;
        }
    }

    //HRESULT hr;
    DWORD* lpPart1, * lpPart2;
    DWORD	dwPart1Size, dwPart2Size;

    // Next we need to load our audio data into our Secondary Buffer
    if (FAILED(hr = g_lpDSBuffer[Buffer][0]->Lock(0, g_dwBufferBytes, (void**)&lpPart1, &dwPart1Size, (void**)&lpPart2, &dwPart2Size, 0)))
    {
        g_ErrorReport.Write(L"Failed to Lock Secondary buffer [%s]\r\n", strFileName);
        wprintf(L"Failed to Lock Secondary buffer\n");
        SAFE_RELEASE(g_lpDSBuffer[Buffer][0]);
        delete  wavefile;
        return 0;
    }

    if (dwPart1Size != g_dwBufferBytes)
    {
        g_ErrorReport.Write(L"Couldn't Lock the whole buffer for some reason ... [%s]\r\n", strFileName);
        wprintf(L"Couldn't Lock the whole buffer for some reason ...\n");
        SAFE_RELEASE(g_lpDSBuffer[Buffer][0]);
        delete  wavefile;
        return 0;
    }

    // Use the waveIO Class ReadWaveData method to load in the data to where lpPart1 points
    if (FAILED(hr = wavefile->ReadWaveData((char*)lpPart1, g_dwBufferBytes)))
    {
        g_ErrorReport.Write(L"Failed to read audio data [%s]\r\n", strFileName);
        wprintf(L"Failed to read audio data\n");
        SAFE_RELEASE(g_lpDSBuffer[Buffer][0]);
        delete  wavefile;
        return 0;
    }

    // Unlock the buffer
    if (FAILED(hr = g_lpDSBuffer[Buffer][0]->Unlock(lpPart1, g_dwBufferBytes, lpPart2, 0)))
    {
        g_ErrorReport.Write(L"Failed to UnLock Secondary buffer [%s]\r\n", strFileName);
        wprintf(L"Failed to UnLock Secondary buffer\n");
        SAFE_RELEASE(g_lpDSBuffer[Buffer][0]);
        delete  wavefile;
        return 0;
    }
    delete wavefile;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FillBuffer()
// Desc: Fill the DirectSound buffer with data from the wav file
//-----------------------------------------------------------------------------
HRESULT FillBuffer(int Buffer, int MaxChannel, bool Enable)
{
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: LoadWaveFile()
// Desc: Loads the wave file into a secondary static DirectSound buffer
//-----------------------------------------------------------------------------
VOID LoadWaveFile(ESound Buffer, wchar_t* strFileName, int MaxChannel, bool Enable)
{
    if (!g_EnableSound)
        return;
    if (Buffer < 0)
        return;

    if (MaxBufferChannel[Buffer] > 0)
    {
        return;
    }

    if (!g_Enable3DSound)
    {
        Enable = false;
    }

    // Create the sound buffer object from the wave file data
    if (FAILED(CreateStaticBuffer(Buffer, strFileName, MaxChannel, Enable)))
    {
        return;
    }

    if (FAILED(FillBuffer(Buffer, MaxChannel, Enable)))  // The sound buffer was successfully created
    {
        return;
    }

    BufferChannel[Buffer] = 0;
    MaxBufferChannel[Buffer] = MaxChannel;
    Enable3DSound[Buffer] = Enable;
    wcscpy(BufferName[Buffer], strFileName);

    SoundLoadCount++;

    SetVolume(Buffer, g_MasterVolume);
}

//-----------------------------------------------------------------------------
// Name: FreeDirectSound()
// Desc: Releases DirectSound
//-----------------------------------------------------------------------------
HRESULT ReleaseBuffer(int Buffer)
{
    if (!g_EnableSound) return false;
    if (Buffer < 0) return false;
    //SAFE_DELETE( g_pWaveSoundRead );

    // Release DirectSound interfaces
    for (int i = 0; i < MaxBufferChannel[Buffer]; i++)
    {
        SAFE_RELEASE(g_lpDSBuffer[Buffer][i]);
        if (Enable3DSound[i])
        {
            SAFE_RELEASE(g_lpDS3DBuffer[Buffer][i]);
            //SAFE_RELEASE( g_lpPropertySet[Buffer][i] );
        }
    }

    for (int i = 0; i < MAX_CHANNEL; i++)
        Object3DSound[Buffer][i] = 0;

    MaxBufferChannel[Buffer] = 0;

    SoundLoadCount--;

    // Release COM
    //CoUninitialize();

    return S_OK;
}

void FreeDirectSound()
{
    if (!g_EnableSound) return;
    SAFE_RELEASE(g_lpDS);
    g_EnableSound = false;
}

//-----------------------------------------------------------------------------
// Name: RestoreBuffers()
// Desc: Restore lost buffers and fill them up with sound if possible
//-----------------------------------------------------------------------------
HRESULT RestoreBuffers(int Buffer, int Channel)
{
    if (!g_EnableSound) return false;
    if (Buffer < 0) return false;

    HRESULT hr;

    if (NULL == g_lpDSBuffer[Buffer][Channel])
        return S_OK;

    DWORD dwStatus;
    if (FAILED(hr = g_lpDSBuffer[Buffer][Channel]->GetStatus(&dwStatus)))
        return hr;

    if (dwStatus & DSBSTATUS_BUFFERLOST)
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so
        // the restoring the buffer may fail.
        // If it does, sleep until DirectSound gives us control.
        do
        {
            hr = g_lpDSBuffer[Buffer][Channel]->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
        } while (hr = g_lpDSBuffer[Buffer][Channel]->Restore());

        if (FAILED(hr = FillBuffer(Buffer, Channel, Enable3DSound[Buffer])))
            return hr;
    }

    return S_OK;
}

HRESULT SetHall(int Buffer)
{
    return true;
}

DWORD Temp;

//-----------------------------------------------------------------------------
// Name: PlayBuffer()
// Desc: User hit the "Play" button, so play the DirectSound buffer
//-----------------------------------------------------------------------------
HRESULT PlayBuffer(ESound Buffer, OBJECT* Object, BOOL bLooped)
{
    if (!g_EnableSound) return false;
    if (Buffer < 0) return false;

    HRESULT hr;

    if (NULL == g_lpDSBuffer[Buffer][BufferChannel[Buffer]])
    {
        return E_FAIL;
    }

    // Play buffer
    DWORD dwLooped = bLooped ? DSBPLAY_LOOPING : 0L;

    if (FAILED(hr = g_lpDSBuffer[Buffer][BufferChannel[Buffer]]->Play(0, 0, dwLooped)))
    {
        g_ConsoleDebug->Write(MCD_ERROR, L"Play Sound: %d, %d", Buffer, dwLooped);
        return hr;
    }

    if (Enable3DSound[Buffer])
    {
        Object3DSound[Buffer][BufferChannel[Buffer]] = Object;
        SetHall(Buffer);
    }

    if (BufferChannel[Buffer] >= MaxBufferChannel[Buffer])
    {
        BufferChannel[Buffer] = 0;
    }
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: IsSoundPlaying()
// Desc: Checks to see if a sound is playing and returns TRUE if it is.
//-----------------------------------------------------------------------------
BOOL IsSoundPlaying(int Buffer, int Channel)
{
    if (!g_EnableSound)
        return false;
    if (Buffer < 0)
        return false;

    if (g_lpDSBuffer[Buffer][Channel])
    {
        DWORD dwStatus = 0;
        g_lpDSBuffer[Buffer][Channel]->GetStatus(&dwStatus);
        return((dwStatus & DSBSTATUS_PLAYING) != 0);
    }
    else
    {
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
// Name: StopBuffer()
// Desc: Stop the DirectSound buffer from playing
//-----------------------------------------------------------------------------
VOID StopBuffer(ESound Buffer, BOOL bResetPosition)
{
    //return;
    if (!g_EnableSound)
        return;
    if (Buffer < 0)
        return;

    if (NULL == g_lpDSBuffer[Buffer][0])
        return;

    g_lpDSBuffer[Buffer][0]->Stop();

    if (bResetPosition)
        g_lpDSBuffer[Buffer][0]->SetCurrentPosition(0L);
}

void AllStopSound(void)
{
    if (!g_EnableSound)
        return;

    for (int i = 0; i < MAX_BUFFER; ++i)
    {
        StopBuffer(static_cast<ESound>(i), true);
    }
}

void SetVolume(int Buffer, long vol)
{
    if (!g_EnableSound)
        return;
    if (Buffer < 0)
        return;

    if (g_lpDSBuffer[Buffer][0])
    {
        if (vol > DSBVOLUME_MAX)
            vol = DSBVOLUME_MAX;
        if (vol < DSBVOLUME_MIN)
            vol = DSBVOLUME_MIN;

        g_lpDSBuffer[Buffer][0]->SetVolume(vol);
    }
}

void SetMasterVolume(long vol)
{
    if (!g_EnableSound)
        return;

    for (int i = 0; i < MAX_BUFFER; ++i)
    {
        SetVolume(i, vol);
    }

    g_MasterVolume = vol;
}

extern vec3_t CameraAngle;

void Set3DSoundPosition()
{
    if (!g_EnableSound) return;

    if (g_Enable3DSound)
    {
        vec3_t Angle;
        float Matrix[3][4];
        Vector(0.f, 0.f, CameraAngle[2], Angle);
        AngleMatrix(Angle, Matrix);
        for (int i = 0; i < MAX_BUFFER; i++)
        {
            if (Enable3DSound[i])
            {
                for (int j = 0; j < MaxBufferChannel[i]; j++)
                {
                    if (Object3DSound[i][j])
                    {
                        vec3_t SoundPosition;
                        VectorCopy(Object3DSound[i][j]->Position, SoundPosition);
                        VectorSubtract(Hero->Object.Position, SoundPosition, SoundPosition);
                        VectorRotate(SoundPosition, Matrix, SoundPosition);
                        VectorScale(SoundPosition, 0.004f, SoundPosition);
                        g_lpDS3DBuffer[i][j]->SetPosition(-SoundPosition[0], 0.f, -SoundPosition[1], DS3D_IMMEDIATE);
                    }
                }
            }
        }
    }
}