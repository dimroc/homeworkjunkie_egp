//--------------------------------------------------------------------------------------
// File: JunkieHomework.cpp
//
// Empty starting point for new Direct3D applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "dxstdafx.h"
#include "resource.h"

#include "mp3player.h"
#include "HomeworkJunkie.h"
#include "GlobalDefine.h"


//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
Cmp3player                          g_mp3p; //! MP3 Player used to play music!
CHomeworkJunkie*                    g_pJunkie; //! Pointer to Game!
D3DXMATRIX                          g_mOrtho2D; // orthogonal 2D matrix projection
D3DXMATRIX                          g_mPersProj; //! Perspective Proj
D3DXMATRIX                          g_mIdentity; // view matrix will be identity and other places use WorldView in World

CDXUTDialogResourceManager          g_DialogResourceManager; //! manager for shared resources of dialogs
CDXUTDialog                         g_UI;                   //! dialog for sample specific controls
CDXUTDialog                         g_NextLevelButton;

ID3DXFont*                          g_pFont = NULL;          //! Font for drawing text
ID3DXSprite*                        g_pTextSprite = NULL;    //! Sprite for batching draw text calls

BOOL                                g_bDisplayNextLevelOption = false; //! whether to display next level option

//--------------------------------------------------------------------------------------
//! Forward Declarations
//--------------------------------------------------------------------------------------*/
void            InitApp();   //! Initializes 
void            CleanupApp();   //! Cleans up memory allocated in InitApp()
VOID            RenderText();
VOID            HandleGraphEvent(); //! Used to handle graph mp3 player

UCHAR           ConvertCharToHex(char c); //! Converts char to hex.
UINT WCharStringToCharString(LPCWSTR fn, LPSTR pszFilename, int MAXLENGTH);

bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK MouseProc(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext);
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );


//! Initializes 
void InitApp()
{
    D3DXMatrixIdentity(&g_mIdentity);
    g_mp3p.PlayFile(g_cstrThemeSong, DXUTGetHWND(), WM_GRAPHNOTIFY);

    g_UI.Init( &g_DialogResourceManager );
    g_NextLevelButton.Init ( &g_DialogResourceManager );

    g_UI.SetCallback( OnGUIEvent ); 
    g_NextLevelButton.SetCallback( OnGUIEvent );

    int iY = 0;
    CDXUTEditBox *pEditBox;
    g_UI.AddEditBox( IDC_PASSWORDENTRY, L"Enter Level Password", 5, iY, 210, 20, false, &pEditBox );
    if( pEditBox )
    {
        pEditBox->SetBorderWidth( 1 );
        pEditBox->SetSpacing( 2 );
    }
    g_UI.AddButton( IDC_LOADPASSWORD, L"Load Level Password", 0, iY += 24, 150, 22 );

    g_NextLevelButton.AddButton( IDC_LOADNEXTLEVEL, L"LOAD NEXT LEVEL!", 0, iY += 24, 150, 22 );
}

void CleanupApp()
{
    g_mp3p.CleanUp();
}

//! Converts char to hex.
UCHAR ConvertCharToHex(char c)
{ 
    if(isdigit(c))
        return c - 48;
    else if(islower(c))
        return c - 87;
    else if(isupper(c))
        return c - 55;
    else 
        return 0;
}

VOID RenderText()
{
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( pd3dsdBackBuffer->Width/5, pd3dsdBackBuffer->Height  - 15*2 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 0.2f, 0.2f, 0.85f, 1.0f ) );    
    txtHelper.DrawFormattedTextLine( L"CURRENT LEVEL:   %d  ||  "
                                     L"CURRENT SCORE:   %.2f ||  "
                                     L"LEVEL PASSWORD:  %x",
                                     g_pJunkie->GetLevel(),
                                     g_pJunkie->GetScore(), 
                                     g_pJunkie->GetPassword()
                                     );
    if(g_bDisplayNextLevelOption) {
        txtHelper.SetInsertionPos( 0, 90 );
        txtHelper.DrawTextLine(L"YOU ARE THE BIGGEST STICK ON THE BOARD!\n"
                               L"DESTROY ALL PENCILS OR LOAD NEXT LEVEL!");
    }

    txtHelper.End();
}

//--------------------------------------------------------------------------------------
// Rejects any devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Typically want to skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3DPOOL_MANAGED resources here 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
    V_RETURN( g_DialogResourceManager.OnCreateDevice( pd3dDevice ) );

        // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Verdana", &g_pFont ) );
    // make new game
    g_pJunkie = new CHomeworkJunkie(pd3dDevice, (float)pBackBufferSurfaceDesc->Width, (float)pBackBufferSurfaceDesc->Height);

    //set up projection matrix
    D3DXMatrixOrthoLH(&g_mOrtho2D, (float)pBackBufferSurfaceDesc->Width, (float)pBackBufferSurfaceDesc->Height, 0.0f, 1.0f);    

    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    D3DXMatrixPerspectiveFovLH( &g_mPersProj, D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3DPOOL_DEFAULT resources here 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
    V_RETURN( g_DialogResourceManager.OnResetDevice() );    

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    g_UI.SetLocation( pBackBufferSurfaceDesc->Width-225, 5 );
    g_UI.SetSize( 220, 470 );

    g_NextLevelButton.SetLocation( pBackBufferSurfaceDesc->Width-225, 5 );
    g_NextLevelButton.SetSize( 220, 470 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{    
    if( g_pJunkie->OnFrameMove(fElapsedTime)) {
        g_bDisplayNextLevelOption = true;
    }
}


//--------------------------------------------------------------------------------------
// Render the scene 
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        V( g_pJunkie->Render(pd3dDevice, &g_mPersProj, &g_mOrtho2D) );
        V( g_UI.OnRender(fElapsedTime) );

        if(g_bDisplayNextLevelOption)
            V( g_NextLevelButton.OnRender(fElapsedTime) );

        RenderText();
        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
                          bool* pbNoFurtherProcessing, void* pUserContext )
{       
    *pbNoFurtherProcessing = g_UI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    *pbNoFurtherProcessing = g_NextLevelButton.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    switch(uMsg)
    {    
    case WM_GRAPHNOTIFY: // event by mp3player
      HandleGraphEvent();
      break;    
    case WM_MOUSEMOVE:
        //g_pJunkie->MouseEvent(false, LOWORD(lParam), HIWORD(lParam), false, false, 0);
        break;
    }
    return 0;
}


//--------------------------------------------------------------------------------------
// Release resources created in the OnResetDevice callback here 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnLostDevice();

    SAFE_RELEASE(g_pTextSprite);
    
    if( g_pFont )
        g_pFont->OnLostDevice();
}

//--------------------------------------------------------------------------------------
// Release resources created in the OnCreateDevice callback here
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnDestroyDevice();    
    
    SAFE_RELEASE( g_pFont );

    SAFE_DELETE(g_pJunkie);
}

UINT WCharStringToCharString(LPCWSTR fn, LPSTR pszFilename, int MAXLENGTH) {
    UINT nLength;
    StringCchLength(fn, MAXLENGTH, &nLength);        
    WideCharToMultiByte(CP_ACP, NULL, fn, nLength, pszFilename, MAXLENGTH, NULL, NULL);
    pszFilename[nLength] = 0; // null terminate string
    return nLength;
}

//--------------------------------------------------------------------------------------
// Keyboard Event handler
//--------------------------------------------------------------------------------------
void  CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    WCHAR wszInput[9];
    UINT pw;
    switch(nControlID)
    {
    case IDC_LOADPASSWORD:
        g_UI.GetEditBox( IDC_PASSWORDENTRY )->GetTextCopy(wszInput, 9);      
        
        char rawr[9];
        WCharStringToCharString(wszInput, rawr, 9);
        pw = static_cast<int>(strtol(rawr, NULL, 16));
        
        if(g_pJunkie->LoadPassword(pw))
            g_UI.GetEditBox( IDC_PASSWORDENTRY )->SetText(L"LEVEL LOADED");
        else
            g_UI.GetEditBox( IDC_PASSWORDENTRY )->SetText(L"Password Failed!");
        break;
    case IDC_LOADNEXTLEVEL:
        g_bDisplayNextLevelOption = false;
        g_pJunkie->LoadNextLevel();
        break;
    }
    
}

//--------------------------------------------------------------------------------------
// Keyboard Event handler
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    g_pJunkie->KeyboardEvent( nChar, bKeyDown );
}

//--------------------------------------------------------------------------------------
// Mouse Event Handler
//--------------------------------------------------------------------------------------
void CALLBACK MouseProc(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, 
                        bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, 
                        int xPos, int yPos, void* pUserContext)
{    
    g_pJunkie->MouseEvent(bLeftButtonDown, xPos, yPos, bLeftButtonDown, bRightButtonDown, nMouseWheelDelta);
}

//--------------------------------------------------------------------------------------
// Used to handle mp3 player
//--------------------------------------------------------------------------------------
VOID HandleGraphEvent() {  
   if(!g_mp3p.IsEventInit())
      return;
   long evCode;
   LONG_PTR param1, param2;   
   while(SUCCEEDED(g_mp3p.pEvent->GetEvent(&evCode, &param1, &param2, 0))) {      
      g_mp3p.pEvent->FreeEventParams(evCode, param1, param2);
      switch(evCode) {
        case EC_COMPLETE:   // dont loop it gets annoying!
            //g_mp3p.CleanUp();
            //g_mp3p.PlayFile(g_cstrThemeSong, DXUTGetHWND(), WM_GRAPHNOTIFY);
            //return;
        case EC_USERABORT: // Fall through.
        case EC_ERRORABORT:
           g_mp3p.CleanUp();
           return;           
      }      
   }
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackMouse( MouseProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, false, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Homework Junkie: The Ink Oppression" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 1024, 768, IsDeviceAcceptable, ModifyDeviceSettings );

    InitApp();

    // Start the render loop
    DXUTMainLoop();

    CleanupApp();

#if defined(DEBUG) || defined(_DEBUG)
    // Functions to pinpoint memory leaks
    // _CrtDumpMemoryLeaks ();
    _CrtCheckMemory();
#endif

    return DXUTGetExitCode();
}
